/* File: irc.c */

/* Purpose: irc chat */

/*
 * Copyright (c) 2001 DarkGod, Andrew Sidwell
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#define IRC_SERVER "irc.worldirc.org"
#define IRC_PORT "6667"
#define IRC_CHANNEL "#tome"

/*
 * By the way, CTCP's for unique kills and artefact finds would be nice to
 * have, for example:
 *
 * *pelpel finds Long Sword 'Ringil' (4d5) (+22,+25) (+10 to speed) :)
 */

ip_connection tome_irc_forge;
ip_connection *tome_irc = &tome_irc_forge;
bool irc_can_join = FALSE;
char irc_nick[30];
char irc_world[100];

void irc_connect()
{
	char buf[500], *s;
	int rnd_name = randint(999);

	if (tome_irc->connected) return;

	sprintf(irc_world, "%99s", IRC_CHANNEL);
	sprintf(irc_nick, "Dummy_%03d", rnd_name);
	get_string("Enter Nickname: ", irc_nick, 10);

	zsock.setup(tome_irc, IRC_SERVER, atoi(IRC_PORT), ZSOCK_TYPE_TCP, FALSE);
	zsock.open(tome_irc);
	zsock.write_simple(tome_irc, format("NICK %s\r\n", irc_nick));
	zsock.wait(tome_irc, 40);
	zsock.read_simple(tome_irc, buf, 500);
	s = strchr(buf, ':');
	zsock.write_simple(tome_irc, format("PONG %s\r\n", s));
	zsock.write_simple(tome_irc, format("USER tome 0 *BIRC :%s %d.%d.%d User\r\n",
	                                    game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH));
#if 0 /* Pfft spoilsport */
	while (!irc_can_join)
		irc_poll();
#endif

	zsock.write_simple(tome_irc, format("JOIN %s\r\n", irc_world));

	cmsg_print(TERM_L_GREEN, "Connected to IRC");

	zsock.add_timer(irc_poll);
}

void irc_change_nick()
{
	return;
}

void irc_disconnect()
{
	if (!tome_irc->connected) return;
	irc_can_join = FALSE;

	irc_quit(format("%s %d.%d.%d", game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH));

	cmsg_print(TERM_L_RED, "Disconnected from IRC");
}

void irc_disconnect_aux(char *str, bool message)
{
	if (!tome_irc->connected) return;
	irc_can_join = FALSE;

	irc_quit(str);

	if (message) cmsg_print(TERM_L_RED, "Disconnected from IRC");
}

void irc_emote(char *buf)
{
	char *b;
	char *base = "PRIVMSG %s :%cACTION %s%c\r\n";

	if (!tome_irc->connected) return;

	C_MAKE(b, strlen(buf) + strlen(base) + 1, char);
	sprintf(b, base, irc_world, 1, buf, 1);
	zsock.write_simple(tome_irc, b);
	sprintf(b, "* %s %s", irc_nick, buf);
	message_add(MESSAGE_IRC, b, TERM_YELLOW);
	C_FREE(b, strlen(buf) + strlen(base) + 1, char);
	fix_irc_message();
}

void irc_chat()
{
	char buf[80] = "";

	if (!tome_irc->connected) return;
	if (get_string("Say: ", buf, 80))
	{
		if (prefix(buf, "/me "))
		{
			irc_emote(buf + 4);
		}
		else if ((prefix(buf, "/join ")) && (buf[6] != '\0'))
		{
			zsock.write_simple(tome_irc, format("PART %s\r\n", irc_world));
			sprintf(irc_world, "%99s", buf + 6);
			zsock.write_simple(tome_irc, format("JOIN %s\r\n", irc_world));
		}
		else
		{
			zsock.write_simple(tome_irc, format("PRIVMSG %s :%s\r\n", irc_world, buf /*, 3, irc_world */));
			message_add(MESSAGE_IRC, format("<%s> #w%s", irc_nick, buf), TERM_L_BLUE);
			fix_irc_message();
		}
	}
}

#define TERM_CTCP	TERM_L_GREEN
#define TERM_SERVER	TERM_L_BLUE
#define TERM_CHAT1	TERM_YELLOW
#define TERM_CHAT2	TERM_WHITE

void irc_poll()
{
	char buf[5000], *next, *nick, *space;

	if (tome_irc->connected && zsock.can_read(tome_irc))
	{
		zsock.read_simple(tome_irc, buf, 2500);

		if (prefix(buf, "PING "))
		{
			message_add(MESSAGE_IRC, format("*** Recieved a PING request from server %s.", buf + 6), TERM_SERVER);
			zsock.write_simple(tome_irc, format("PONG %s\r\n", buf + 5));
			return;
		}
		if (*buf != ':') return;
		nick = buf + 1;

		space = strchr(nick, ' ');
		if (space)
		{
			if (prefix(space + 1, "376"))
				irc_can_join = TRUE;
		}

		if (prefix(nick, "_"))
		{
			nick = buf + 6;
		}

		next = strchr(nick, '!');
		if (next == NULL) return;
		*next = '\0';
		next++;
		next = strchr(next, ' ');
		if (next == NULL) return;
		next++;
		if (prefix(next, "PRIVMSG"))
		{
			next = strchr(next, ':');
			if (next == NULL) return;
			*next = '\0';
			next++;
			if (*next == 1)
			{
				next++;
				if (prefix(next, "ACTION"))
				{
					u32b i = 0, j = 0, max = (79 - strlen(nick) - 3);
					bool nicked = FALSE;
					char tmp[90];

					next += 7;
					if (strlen(next)) next[strlen(next) - 1] = '\0';

					while (next[i])
					{
						tmp[j++] = next[i++];
						if (j > max)
						{
							tmp[j] = '\0';
							if (nicked)
								message_add(MESSAGE_IRC, format("%s", tmp), TERM_CHAT1);
							else
								message_add(MESSAGE_IRC, format("* %s %s", nick, tmp), TERM_CHAT1);
							nicked = TRUE;
							j = 0;
						}
					}
					if (j > 0)
					{
						tmp[j] = '\0';
						if (nicked)
							message_add(MESSAGE_IRC, format("%s", tmp), TERM_CHAT1);
						else
							message_add(MESSAGE_IRC, format("* %s %s", nick, tmp), TERM_CHAT1);
					}

					fix_irc_message();
				}
				else if (prefix(next, "PING"))
				{
					message_add(MESSAGE_IRC, format("*** PING request from %s", nick), TERM_CTCP);
					fix_irc_message();

					zsock.write_simple(tome_irc, format("NOTICE %s :%cPING %d%c\r\n", nick, 1, next, 1));
				}
				else if (prefix(next, "NICK"))
				{
					message_add(MESSAGE_IRC, format("*** NICK request from %s", nick), TERM_CTCP);
					fix_irc_message();

					zsock.write_simple(tome_irc, format("NOTICE %s :%cNICK %s%c\r\n", nick, 1, irc_nick, 1));
				}
				else if (prefix(next, "VERSION"))
				{
					message_add(MESSAGE_IRC, format("*** VERSION request from %s", nick), TERM_CTCP);
					fix_irc_message();

					zsock.write_simple(tome_irc, format("NOTICE %s :%cVERSION %s %d.%d.%d%c\r\n", nick, 1, game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, 1));
				}
			}
			else
			{
				u32b i = 0, j = 0, max = (79 - strlen(nick) - 3);
				bool nicked = FALSE;
				char tmp[90];

				while (next[i])
				{
					tmp[j++] = next[i++];
					if (j > max)
					{
						tmp[j] = '\0';
						if (nicked)
							message_add(MESSAGE_IRC, format("#w%s", tmp), TERM_CHAT1);
						else
							message_add(MESSAGE_IRC, format("#y<%s> #w%s", nick, tmp), TERM_CHAT1);
						nicked = TRUE;
						j = 0;
					}
				}
				if (j > 0)
				{
					tmp[j] = '\0';
					if (nicked)
						message_add(MESSAGE_IRC, format("#w%s", tmp), TERM_CHAT1);
					else
						message_add(MESSAGE_IRC, format("#y<%s> #w%s", nick, tmp), TERM_CHAT1);
				}
				fix_irc_message();
			}
		}
		if (prefix(next, "JOIN"))
		{
			message_add(MESSAGE_IRC, format("%s has entered the Chatroom", nick), TERM_YELLOW);
			fix_irc_message();
		}
		if (prefix(next, "QUIT"))
		{
			next = strchr(next, ':');
			if (next == NULL) return;
			*next = '\0';
			next++;
			message_add(MESSAGE_IRC, format("%s has quit the Chatroom (%s)", nick, next), TERM_YELLOW);
			fix_irc_message();
		}
	}
}


void irc_quit(char *str)
{
	char buf[300];

	zsock.remove_timer(irc_poll);

	sprintf(buf, "QUIT :%s\r\n", str);

	zsock.write_simple(tome_irc, buf);
	zsock.close(tome_irc);
	zsock.unsetup(tome_irc);
}
