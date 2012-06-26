#include "messages.h"

#include "angband.h"

/*
 * OPTION: Maximum number of messages to remember (see "io.c")
 * Default: assume maximal memorization of 2048 total messages
 */
#define MESSAGE_MAX     2048

/*
 * OPTION: Maximum space for the message text buffer (see "io.c")
 * Default: assume that each of the 2048 messages is repeated an
 * average of three times, and has an average length of 48
 */
#define MESSAGE_BUF     32768




/*
 * The next "free" index to use
 */
static u16b message__next;

/*
 * The index of the oldest message (none yet)
 */
static u16b message__last;

/*
 * The next "free" offset
 */
static u16b message__head;

/*
 * The offset to the oldest used char (none yet)
 */
static u16b message__tail;

/*
 * The array of offsets, by index [MESSAGE_MAX]
 */
static u16b *message__ptr;

/*
 * The array of colors, by index [MESSAGE_MAX]
 */
static byte *message__color;

/*
 * The array of message counts, by index [MESSAGE_MAX]
 */
static u16b *message__count;

/*
 * The array of chars, by offset [MESSAGE_BUF]
 */
static char *message__buf;


/*
* Second try for the "message" handling routines.
*
* Each call to "message_add(s)" will add a new "most recent" message
* to the "message recall list", using the contents of the string "s".
*
* The messages will be stored in such a way as to maximize "efficiency",
* that is, we attempt to maximize the number of sequential messages that
* can be retrieved, given a limited amount of storage space.
*
* We keep a buffer of chars to hold the "text" of the messages, not
* necessarily in "order", and an array of offsets into that buffer,
* representing the actual messages.  This is made more complicated
* by the fact that both the array of indexes, and the buffer itself,
* are both treated as "circular arrays" for efficiency purposes, but
* the strings may not be "broken" across the ends of the array.
*
* The "message_add()" function is rather "complex", because it must be
* extremely efficient, both in space and time, for use with the Borg.
*/

void message_init()
{
	/* Message variables */
	C_MAKE(message__ptr, MESSAGE_MAX, u16b);
	C_MAKE(message__color, MESSAGE_MAX, byte);
	C_MAKE(message__count, MESSAGE_MAX, u16b);
	C_MAKE(message__buf, MESSAGE_BUF, char);

	/* Hack -- No messages yet */
	message__tail = MESSAGE_BUF;
}

/*
* How many messages are "available"?
*/
s16b message_num(void)
{
	int last, next, n;

	/* Extract the indexes */
	last = message__last;
	next = message__next;

	/* Handle "wrap" */
	if (next < last) next += MESSAGE_MAX;

	/* Extract the space */
	n = (next - last);

	/* Return the result */
	return (n);
}



/*
* Recall the "text" of a saved message
*/
cptr message_str(int age)
{
	static char buf[1024];
	s16b x;
	s16b o;
	cptr s;

	/* Forgotten messages have no text */
	if ((age < 0) || (age >= message_num())) return ("");

	/* Acquire the "logical" index */
	x = (message__next + MESSAGE_MAX - (age + 1)) % MESSAGE_MAX;

	/* Get the "offset" for the message */
	o = message__ptr[x];

	/* Access the message text */
	s = &message__buf[o];

	/* Hack -- Handle repeated messages */
	if (message__count[x] > 1)
	{
		strnfmt(buf, 1024, "%s <%dx>", s, message__count[x]);
		s = buf;
	}

	/* Return the message text */
	return (s);
}

/*
* Recall the color of a saved message
*/
byte message_color(int age)
{
	s16b x;
	byte color = TERM_WHITE;

	/* Forgotten messages have no text */
	if ((age < 0) || (age >= message_num())) return (TERM_WHITE);

	/* Acquire the "logical" index */
	x = (message__next + MESSAGE_MAX - (age + 1)) % MESSAGE_MAX;

	/* Get the "offset" for the message */
	color = message__color[x];

	/* Return the message text */
	return (color);
}


/*
* Add a new message, with great efficiency
*/
void message_add(cptr str, byte color)
{
	int i, k, x, n;
	cptr s;


	/*** Step 1 -- Analyze the message ***/

	/* Hack -- Ignore "non-messages" */
	if (!str) return;

	/* Message length */
	n = strlen(str);

	/* Important Hack -- Ignore "long" messages */
	if (n >= MESSAGE_BUF / 4) return;


	/*** Step 2 -- Handle repeated messages ***/

	/* Acquire the "logical" last index */
	x = (message__next + MESSAGE_MAX - 1) % MESSAGE_MAX;

	/* Get the last message text */
	s = &message__buf[message__ptr[x]];

	/* Last message repeated? */
	if (streq(str, s))
	{
		/* Increase the message count */
		message__count[x]++;

		/* Success */
		return;
	}


	/*** Step 3 -- Attempt to optimize ***/

	/* Limit number of messages to check */
	k = message_num() / 4;

	/* Limit number of messages to check */
	if (k > MESSAGE_MAX / 32) k = MESSAGE_MAX / 32;

	/* Check the last few messages (if any to count) */
	for (i = message__next; k; k--)
	{
		u16b q;

		cptr old;

		/* Back up and wrap if needed */
		if (i-- == 0) i = MESSAGE_MAX - 1;

		/* Stop before oldest message */
		if (i == message__last) break;

		/* Extract "distance" from "head" */
		q = (message__head + MESSAGE_BUF - message__ptr[i]) % MESSAGE_BUF;

		/* Do not optimize over large distance */
		if (q > MESSAGE_BUF / 2) continue;

		/* Access the old string */
		old = &message__buf[message__ptr[i]];

		/* Compare */
		if (!streq(old, str)) continue;

		/* Get the next message index, advance */
		x = message__next++;

		/* Handle wrap */
		if (message__next == MESSAGE_MAX) message__next = 0;

		/* Kill last message if needed */
		if (message__next == message__last) message__last++;

		/* Handle wrap */
		if (message__last == MESSAGE_MAX) message__last = 0;

		/* Assign the starting address */
		message__ptr[x] = message__ptr[i];
		message__color[x] = color;
		message__count[x] = 1;

		/* Success */
		return;
	}


	/*** Step 4 -- Ensure space before end of buffer ***/

	/* Kill messages and Wrap if needed */
	if (message__head + n + 1 >= MESSAGE_BUF)
	{
		/* Kill all "dead" messages */
		for (i = message__last; TRUE; i++)
		{
			/* Wrap if needed */
			if (i == MESSAGE_MAX) i = 0;

			/* Stop before the new message */
			if (i == message__next) break;

			/* Kill "dead" messages */
			if (message__ptr[i] >= message__head)
			{
				/* Track oldest message */
				message__last = i + 1;
			}
		}

		/* Wrap "tail" if needed */
		if (message__tail >= message__head) message__tail = 0;

		/* Start over */
		message__head = 0;
	}


	/*** Step 5 -- Ensure space before next message ***/

	/* Kill messages if needed */
	if (message__head + n + 1 > message__tail)
	{
		/* Grab new "tail" */
		message__tail = message__head + n + 1;

		/* Advance tail while possible past first "nul" */
		while (message__buf[message__tail - 1]) message__tail++;

		/* Kill all "dead" messages */
		for (i = message__last; TRUE; i++)
		{
			/* Wrap if needed */
			if (i == MESSAGE_MAX) i = 0;

			/* Stop before the new message */
			if (i == message__next) break;

			/* Kill "dead" messages */
			if ((message__ptr[i] >= message__head) &&
			                (message__ptr[i] < message__tail))
			{
				/* Track oldest message */
				message__last = i + 1;
			}
		}
	}


	/*** Step 6 -- Grab a new message index ***/

	/* Get the next message index, advance */
	x = message__next++;

	/* Handle wrap */
	if (message__next == MESSAGE_MAX) message__next = 0;

	/* Kill last message if needed */
	if (message__next == message__last) message__last++;

	/* Handle wrap */
	if (message__last == MESSAGE_MAX) message__last = 0;



	/*** Step 7 -- Insert the message text ***/

	/* Assign the starting address */
	message__ptr[x] = message__head;
	message__color[x] = color;
	message__count[x] = 1;

	/* Append the new part of the message */
	for (i = 0; i < n; i++)
	{
		/* Copy the message */
		message__buf[message__head + i] = str[i];
	}

	/* Terminate */
	message__buf[message__head + i] = '\0';

	/* Advance the "head" pointer */
	message__head += n + 1;
}
