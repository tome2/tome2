/* File: z-sock.h */

/*
 * Copyright (c) 2002 DarkGod
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifndef INCLUDED_Z_SOCK_H
#define INCLUDED_Z_SOCK_H

#include "h-basic.h"

/*
 * This represents an IP connection
 */
typedef struct ip_connection ip_connection;

/* A callback used when the connection suddently dies */
typedef void (*lose_connection_hook)(ip_connection *conn);

struct ip_connection
{
	bool setup;                     /* Has it been setted up yet? */

	long conn_ip;                  	/* The IP where to connect to */
	int conn_port;                	/* The port where to connect to */
	byte conn_type;                 /* Type of connection */

	bool connected;                 /* The connection status */
	void *socket;                   /* The socket for the connection */

	lose_connection_hook lost_conn; /* Called when the conenction dies */

	bool server;                    /* Is it a server socket ? */
};

/*
 * Possible connection types
 */
#define ZSOCK_TYPE_TCP  	1
/* #define ZSOCK_TYPE_UDP  	2 */


/*
 * The time in milliseconds when to call the sockets callbacks for the timer
 */
#define ZSOCK_TIMER_DELAY       100

/* Timer callbacks */
typedef void (*timer_callback)(void);
typedef struct timer_callback_list timer_callback_list;
struct timer_callback_list
{
	timer_callback callback;
	timer_callback_list *next;
};

/*
 * Hooks needed for a main-foo.c to be sock-able
 */
typedef struct zsock_hooks zsock_hooks;
struct zsock_hooks
{
	/* Creates a struct */
	ip_connection *(*new_connection)(void);

	/* Free it */
	void (*free_connection)(ip_connection *c);

	/* Setup a connection, but do NOT connect */
	bool (*setup)(ip_connection *conn, cptr conn_ip, int port, byte conn_type, bool server);

	/* Unsetup a connection, but and DO close before if needed */
	bool (*unsetup)(ip_connection *conn);

	/* Open(connect) a well setup-ed connection */
	bool (*open)(ip_connection *conn);

	/* Close a connected connection */
	bool (*close)(ip_connection *conn);

	/* Send data on the connection */
	bool (*write)(ip_connection *conn, cptr str, int *size);

	/* Read data on the connection */
	bool (*read)(ip_connection *conn, char *str, int *len, bool raw);

	/* Send data on the connection -- easy to use  */
	bool (*write_simple)(ip_connection *conn, cptr str);

	/* Read data on the connection -- easy to use */
	bool (*read_simple)(ip_connection *conn, char *str, int len);

	/* Set the dying connection callback */
	void (*set_lose_connection)(ip_connection *conn, lose_connection_hook hook);

	/* Accept a connection */
	bool (*accept)(ip_connection *conn, ip_connection *child);

	/* Check if there is any data to be read and return instantly in any case */
	bool (*can_read)(ip_connection *conn);

	/* Wait until there is any data to be read and return after seconds time in any case */
	bool (*wait)(ip_connection *conn, int seconds);


	/*
	 * Timer stuff, I hope I can make that look better
	 */
	int __timers;
	timer_callback_list *__timer_callbacks;

	/* Setup the timer */
	bool (*add_timer)(timer_callback callback);

	/* Remove the timer */
	bool (*remove_timer)(timer_callback callback);
};

extern zsock_hooks zsock;

extern bool zsock_init(void);

#endif
