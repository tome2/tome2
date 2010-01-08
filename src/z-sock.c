/* File: z-term.c */

/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

/* Purpose: a generic, IP connection package */

#include "angband.h"
#include "z-term.h"
#include "z-virt.h"


/*
 * System independant functions
 */

/* Call all the callbacks */
void zsock_handle_callbacks()
{
	timer_callback_list *c = zsock.__timer_callbacks;

	while (c != NULL)
	{
		c->callback();
		c = c->next;
	}
}

/* Add a callback in the list */
void zsock_add_timer_callback(timer_callback callback)
{
	timer_callback_list *c;

	/* Create it */
	MAKE(c, timer_callback_list);
	c->callback = callback;

	/* Add it into the lsit */
	c->next = zsock.__timer_callbacks;
	zsock.__timer_callbacks = c;

	/* Increase the timer count */
	zsock.__timers++;
}

/* Remove a callback in the list */
void zsock_remove_timer_callback(timer_callback callback)
{
	timer_callback_list *c = zsock.__timer_callbacks, *old = NULL;

	/* Find it */
	while ((c != NULL) && (c->callback != callback))
	{
		old = c;
		c = c->next;
	}

	if (c->callback == callback)
	{
		/* Skip it */
		if (old == NULL)
			zsock.__timer_callbacks = c->next;
		else
			old->next = c->next;

		/* Delete it */
		FREE(c, timer_callback_list);

		/* Increase the timer count */
		zsock.__timers--;
	}
	else
	{
		cmsg_print(TERM_VIOLET, "WARNING: tried to remove a non existing timer callback!");
	}
}

/* Creates a connection struct */
ip_connection *zsock_new_connection()
{
	ip_connection *c;

	MAKE(c, ip_connection);
	return c;
}
void zsock_free_connection(ip_connection *c)
{
	FREE(c, ip_connection);
}


/* Set the dying connection callback */
void zsock_set_lose_connection(ip_connection *conn, lose_connection_hook hook)
{
	conn->lost_conn = hook;
}

/* Send data on the connection -- easy to use  */
bool zsock_write_simple(ip_connection *conn, cptr str)
{
	int len = 0;

	return zsock.write(conn, str, &len);
}

/* Read data on the connection -- easy to use */
bool zsock_read_simple(ip_connection *conn, char *str, int len)
{
	int rlen = len;

	return zsock.read(conn, str, &rlen, FALSE);
}

/*
 ******************************* Here comes the Windows socket part ********************************
 */
#ifdef USE_WINSOCK
#include <windows.h>
#include <winsock2.h>

/* Needed for timers -- pfft */
extern HWND get_main_hwnd();

#define ZSOCK_TIMER_ID   1

VOID CALLBACK zsock_timer_callback_win(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime)
{
	zsock_handle_callbacks();
}

bool zsock_add_timer_win(timer_callback callback)
{
	zsock_add_timer_callback(callback);

	/* Is it the first callback ? then we must create the timer */
	if (zsock.__timers == 1)
	{
		SetTimer(get_main_hwnd(), ZSOCK_TIMER_ID, ZSOCK_TIMER_DELAY, zsock_timer_callback_win);
	}
	return TRUE;
}

bool zsock_remove_timer_win(timer_callback callback)
{
	zsock_remove_timer_callback(callback);

	/* No more callbacks ? no need for the timer then */
	if (!zsock.__timers)
	{
		KillTimer(get_main_hwnd(), ZSOCK_TIMER_ID);
	}
	return TRUE;
}


bool zsock_setup_win(ip_connection *conn, cptr conn_ip, int port, byte conn_type, bool server)
{
	/* Already setup! */
	if (conn->setup) return FALSE;

	MAKE(conn->socket, SOCKET);

	if (!server)
	{
		struct hostent *host;

		host = gethostbyname(conn_ip);
		conn->conn_ip = ((struct in_addr *)(host->h_addr))->s_addr;
	}
	else
		conn->conn_ip = 0;

	conn->conn_type = conn_type;
	conn->conn_port = htons(port);

	conn->setup = TRUE;
	conn->server = server;

	conn->lost_conn = NULL;

	return TRUE;
}

bool zsock_unsetup_win(ip_connection *conn)
{
	/* Already unsetup */
	if (!conn->setup) return FALSE;

	FREE(conn->socket, SOCKET);

	return TRUE;
}

bool zsock_open_win(ip_connection *conn)
{
	struct sockaddr_in sin;

	/* Already connected */
	if (conn->connected) return FALSE;

	*((SOCKET*)conn->socket) = socket(AF_INET, SOCK_STREAM, 0);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = conn->conn_ip;
	sin.sin_port = conn->conn_port;

	if (conn->server)
	{
		if (SOCKET_ERROR == bind(*((SOCKET*)conn->socket), &sin, sizeof(sin)))
			return FALSE;

		if (SOCKET_ERROR == listen(*((SOCKET*)conn->socket), 10)) return FALSE;
	}
	else
	{
		if (connect(*((SOCKET*)conn->socket), &sin, sizeof sin) == SOCKET_ERROR)
		{
			/* could not connect to server */
			return (FALSE);
		}
	}

	conn->connected = TRUE;
	return TRUE;
}

bool zsock_can_read_win(ip_connection *conn)
{
	struct timeval t;
	fd_set rd;
	SOCKET *c = conn->socket;

	if (!conn->connected) return FALSE;

	FD_ZERO(&rd);
	FD_SET(*c, &rd);
	t.tv_sec = 0;
	t.tv_usec = 0;
	select(*c + 1, &rd, NULL, NULL, &t);
	if (FD_ISSET(*c, &rd)) return TRUE;
	else return (FALSE);
}

bool zsock_wait_win(ip_connection *conn, int seconds)
{
	struct timeval t;
	fd_set rd;
	SOCKET *c = conn->socket;

	if (!conn->connected) return FALSE;

	t.tv_sec = seconds;
	t.tv_usec = 0;

	FD_ZERO(&rd);
	FD_SET(*c, &rd);
	select(*c + 1, &rd, NULL, NULL, &t);
	if (FD_ISSET(*c, &rd)) return TRUE;
	else return (FALSE);
}

bool zsock_close_win(ip_connection *conn)
{
	SOCKET *c = conn->socket;

	/* Already disconnected */
	if (!conn->connected) return FALSE;

	closesocket(*c);
	conn->connected = FALSE;
	return TRUE;
}

bool zsock_write_win(ip_connection *conn, cptr str, int *size)
{
	SOCKET *c = conn->socket;

	if (!conn->connected) return FALSE;

	if ((*size = send(*c, str, (!*size) ? strlen(str) : *size, 0)) <= 0)
	{
		/* Oups connection died! */
		if (conn->lost_conn) conn->lost_conn(conn);
		zsock.close(conn);
		return FALSE;
	}

	return TRUE;
}

bool zsock_read_win(ip_connection *conn, char *str, int *len, bool raw)
{
	char c;
	int l = 0;
	SOCKET *cc = conn->socket;

	if (!conn->connected) return FALSE;

	if (!raw)
	{
		/* This *IS* fucking slow */
		while ((l < *len) && zsock_can_read_win(conn))
		{
			if (recv(*cc, &c, 1, 0) <= 0)
			{
				/* Oups connection died! */
				if (conn->lost_conn) conn->lost_conn(conn);
				zsock.close(conn);
				return FALSE;
			}
			if (c == '\r') continue;
			if (c == '\n') break;
			str[l++] = c;
		}
		str[l] = '\0';
		*len = l;
		return TRUE;
	}
	else
	{
		if ((*len = recv(*cc, str, *len, 0)) <= 0)
		{
			/* Oups connection died! */
			if (conn->lost_conn) conn->lost_conn(conn);
			zsock.close(conn);
			return FALSE;
		}
		return TRUE;
	}
}

bool zsock_accept_win(ip_connection *conn, ip_connection *child)
{
	SOCKET *s = conn->socket;
	SOCKET sock;
	struct sockaddr_in sin;
	int len;

	if (!conn->server) return FALSE;
	if (!conn->connected) return FALSE;

	len = sizeof(sin);
	sock = accept(*s, (struct sockaddr*) & sin, &len);

	if (sock == SOCKET_ERROR) return FALSE;

	/* Initialize the connection with a fake destination */
	zsock.setup(child, "127.0.0.1", 0, ZSOCK_TYPE_TCP, FALSE);

	/* Set the correct socket */
	*((SOCKET*)child->socket) = sock;
	child->connected = TRUE;

	return TRUE;
}

bool init_socks_win()
{
	WSADATA wsaData;
	WORD version;
	int error;

	version = MAKEWORD( 2, 0 );

	error = WSAStartup( version, &wsaData );

	/* check for error */
	if ( error != 0 )
	{
		/* error occured */
		return FALSE;
	}

	/* check for correct version */
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
	                HIBYTE( wsaData.wVersion ) != 0 )
	{
		/* incorrect WinSock version */
		WSACleanup();
		return FALSE;
	}

	return TRUE;
}
#endif

/*
 ****************************** And there is the unix sockets **************************
 */
#ifdef USE_UNIXSOCK
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>

void handle_timer(int sig)
{
	zsock_handle_callbacks();
}

static struct sigaction handle_old_alarm;
bool zsock_add_timer_unix(timer_callback callback)
{
	zsock_add_timer_callback(callback);

	/* Is it the first callback ? then we must create the timer */
	if (zsock.__timers == 1)
	{
		struct sigaction new_sig;

		/* Register the timer */
		new_sig.sa_handler = handle_timer;
		new_sig.sa_flags = 0;
		sigaction(SIGALRM, &new_sig, &handle_old_alarm);

		ualarm(ZSOCK_TIMER_DELAY * 1000, ZSOCK_TIMER_DELAY * 1000);
	}
	return TRUE;
}

bool zsock_remove_timer_unix(timer_callback callback)
{
	zsock_remove_timer_callback(callback);

	/* No more callbacks ? no need for the timer then */
	if (!zsock.__timers)
	{
		alarm(0);
		sigaction(SIGALRM, &handle_old_alarm, NULL);
	}
	return TRUE;
}

bool zsock_setup_unix(ip_connection *conn, cptr conn_ip, int port, byte conn_type, bool server)
{
	/* Already setup! */
	if (conn->setup) return FALSE;

	MAKE(conn->socket, int);

	if (!server)
	{
		struct hostent *host;

		host = gethostbyname(conn_ip);
		conn->conn_ip = ((struct in_addr *)(host->h_addr))->s_addr;
	}
	else
		conn->conn_ip = 0;

	conn->conn_type = conn_type;
	conn->conn_port = htons(port);

	conn->setup = TRUE;
	conn->server = server;

	conn->lost_conn = NULL;

	return TRUE;
}

bool zsock_unsetup_unix(ip_connection *conn)
{
	/* Already unsetup */
	if (!conn->setup) return FALSE;

	FREE(conn->socket, int);

	return TRUE;
}

bool zsock_open_unix(ip_connection *conn)
{
	struct sockaddr_in sin;

	/* Already connected */
	if (conn->connected) return FALSE;

	*((int*)conn->socket) = socket(AF_INET, SOCK_STREAM, 0);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = conn->conn_ip;
	sin.sin_port = conn->conn_port;

	if (conn->server)
	{
		int option = 1;

		/* Set this so we don't wait forever on startups */
		if ( -1 == setsockopt(*((int*)conn->socket), SOL_SOCKET, SO_REUSEADDR , (void*)&option, sizeof(int))) return FALSE;

		if ( -1 == bind(*((int*)conn->socket), (struct sockaddr*)&sin, sizeof(sin)))
			return FALSE;

		if ( -1 == listen(*((int*)conn->socket), 10)) return FALSE;
	}
	else
	{
		if (connect(*((int*)conn->socket), (struct sockaddr*)&sin, sizeof sin) == -1)
		{
			/* could not connect to server */
			return (FALSE);
		}
	}

	conn->connected = TRUE;
	return TRUE;
}

bool zsock_can_read_unix(ip_connection *conn)
{
	struct timeval t;
	fd_set rd;
	int *c = conn->socket;

	if (!conn->connected) return FALSE;

	FD_ZERO(&rd);
	FD_SET(*c, &rd);
	t.tv_sec = 0;
	t.tv_usec = 0;
	select(*c + 1, &rd, NULL, NULL, &t);
	if (FD_ISSET(*c, &rd)) return TRUE;
	else return (FALSE);
}

bool zsock_wait_unix(ip_connection *conn, int seconds)
{
	struct timeval t;
	fd_set rd;
	int *c = conn->socket;

	if (!conn->connected) return FALSE;

	t.tv_sec = seconds;
	t.tv_usec = 0;

	FD_ZERO(&rd);
	FD_SET(*c, &rd);
	select(*c + 1, &rd, NULL, NULL, &t);
	if (FD_ISSET(*c, &rd)) return TRUE;
	else return (FALSE);
}

bool zsock_close_unix(ip_connection *conn)
{
	int *c = conn->socket;

	/* Already disconnected */
	if (!conn->connected) return FALSE;

	close(*c);
	conn->connected = FALSE;
	return TRUE;
}

bool zsock_write_unix(ip_connection *conn, cptr str, int *size)
{
	int *c = conn->socket;

	if (conn->server) return FALSE;
	if (!conn->connected) return FALSE;

	if ((*size = send(*c, str, (!*size) ? (s32b)strlen(str) : *size, 0)) <= 0)
	{
		/* Oups connection died! */
		if (conn->lost_conn) conn->lost_conn(conn);
		zsock.close(conn);
		return FALSE;
	}
	return TRUE;
}

bool zsock_read_unix(ip_connection *conn, char *str, int *len, bool raw)
{
	char c;
	int l = 0;
	int *cc = conn->socket;

	if (conn->server) return FALSE;
	if (!conn->connected) return FALSE;

	if (!raw)
	{
		/* This *IS* fucking slow */
		while ((l < *len) && zsock_can_read_unix(conn))
		{
			if (recv(*cc, &c, 1, 0) <= 0)
			{
				/* Oups connection died! */
				if (conn->lost_conn) conn->lost_conn(conn);
				zsock.close(conn);
				return FALSE;
			}
			if (c == '\r') continue;
			if (c == '\n') break;
			str[l++] = c;
		}
		str[l] = '\0';
		*len = l;
		return TRUE;
	}
	else
	{
		if ((*len = recv(*cc, str, *len, 0)) <= 0)
		{
			/* Oups connection died! */
			if (conn->lost_conn) conn->lost_conn(conn);
			zsock.close(conn);
			return FALSE;
		}
		return TRUE;
	}
}

bool zsock_accept_unix(ip_connection *conn, ip_connection *child)
{
	int *s = conn->socket;
	int sock;
	struct sockaddr_in sin;
	unsigned int len;

	if (!conn->server) return FALSE;
	if (!conn->connected) return FALSE;

	len = sizeof(sin);
	sock = accept(*s, (struct sockaddr*) & sin, &len);

	if (sock == -1) return FALSE;

	/* Initialize the connection with a fake destination */
	zsock.setup(child, "127.0.0.1", 0, ZSOCK_TYPE_TCP, FALSE);

	/* Set the correct socket */
	*((int*)child->socket) = sock;
	child->connected = TRUE;

	return TRUE;
}

#endif

/* The zsock hook list */
zsock_hooks zsock;

/*
 * Dummy hooks for systems without socks
 */
bool zsock_setup_dummy(ip_connection *conn, cptr conn_ip, int port, byte conn_type, bool server)
{
	return FALSE;
}

bool zsock_unsetup_dummy(ip_connection *conn)
{
	return FALSE;
}

bool zsock_open_dummy(ip_connection *conn)
{
	return FALSE;
}

bool zsock_can_read_dummy(ip_connection *conn)
{
	return (FALSE);
}

bool zsock_wait_dummy(ip_connection *conn, int seconds)
{
	return (FALSE);
}

bool zsock_close_dummy(ip_connection *conn)
{
	return FALSE;
}

bool zsock_write_dummy(ip_connection *conn, cptr str, int *size)
{
	return FALSE;
}

bool zsock_read_dummy(ip_connection *conn, char *str, int *len, bool raw)
{
	return FALSE;
}

bool zsock_accept_dummy(ip_connection *conn, ip_connection *child)
{
	return FALSE;
}

bool zsock_add_timer_dummy(timer_callback callback)
{
	return TRUE;
}

bool zsock_remove_timer_dummy(timer_callback callback)
{
	return TRUE;
}

void dummy_socks()
{
	zsock.setup = zsock_setup_dummy;
	zsock.unsetup = zsock_unsetup_dummy;
	zsock.open = zsock_open_dummy;
	zsock.close = zsock_close_dummy;
	zsock.write = zsock_write_dummy;
	zsock.read = zsock_read_dummy;
	zsock.accept = zsock_accept_dummy;
	zsock.can_read = zsock_can_read_dummy;
	zsock.wait = zsock_wait_dummy;
	zsock.add_timer = zsock_add_timer_dummy;
	zsock.remove_timer = zsock_remove_timer_dummy;
}

/*
 * Initialize the hooks
 */
static bool init_once = TRUE;
bool zsock_init()
{
	/* Sadly on some platforms we will be called 2 times ... */
	if (init_once)
	{
		/* Set the timers */
		zsock.__timers = 0;
		zsock.__timer_callbacks = NULL;
		init_once = FALSE;
	}

	/* Set the few system independants functions */
	zsock.new_connection = zsock_new_connection;
	zsock.free_connection = zsock_free_connection;
	zsock.set_lose_connection = zsock_set_lose_connection;
	zsock.write_simple = zsock_write_simple;
	zsock.read_simple = zsock_read_simple;

#ifdef USE_WINSOCK
	if (init_socks_win())
	{
		zsock.setup = zsock_setup_win;
		zsock.unsetup = zsock_unsetup_win;
		zsock.open = zsock_open_win;
		zsock.close = zsock_close_win;
		zsock.write = zsock_write_win;
		zsock.read = zsock_read_win;
		zsock.accept = zsock_accept_win;
		zsock.can_read = zsock_can_read_win;
		zsock.wait = zsock_wait_win;
		zsock.add_timer = zsock_add_timer_win;
		zsock.remove_timer = zsock_remove_timer_win;
		return TRUE;
	}
	else
	{
		dummy_socks();
		return TRUE;
	}
#elif defined(USE_UNIXSOCK)
zsock.setup = zsock_setup_unix;
	zsock.unsetup = zsock_unsetup_unix;
	zsock.open = zsock_open_unix;
	zsock.close = zsock_close_unix;
	zsock.write = zsock_write_unix;
	zsock.read = zsock_read_unix;
	zsock.accept = zsock_accept_unix;
	zsock.can_read = zsock_can_read_unix;
	zsock.wait = zsock_wait_unix;
	zsock.add_timer = zsock_add_timer_unix;
	zsock.remove_timer = zsock_remove_timer_unix;
	return TRUE;
#else
	dummy_socks();
	return TRUE;
#endif
}
