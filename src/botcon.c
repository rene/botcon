/**
 * BOTCON - An Instant Messenger Bot for Remote Control
 *
 * Copyright (C)2010 Renê de Souza Pinto  (rene at renesp.com.br)
 *
 *  This file is part of BOTCON, BOTCON is an simple IM bot.
 *
 *  BOTCON is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2.
 *
 *  BOTCON is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Bash; see the file COPYING.  If not, write to the Free
 *  Software Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA.
 */

#include <string.h>
#include <strings.h>
#include "botcon.h"
#include <glib/gthread.h>

void do_quit(int sig);
void show_help(const char *progname);


/* Protocol used for the bot */
protocol *proto;
/* Show debug messages */
static char debug;
/* bot */
botcon_t bot;

/* Main */
int main(int argc, char **argv)
{
	GMainLoop *main_loop;
	struct sigaction newact;
	static struct option longOpts[] = {
		{"help",     no_argument,       NULL, 'h'},
		{"key",      required_argument, NULL, 'k'},
		{"username", required_argument, NULL, 'u'},
		{"server",   required_argument, NULL, 's'},
		{"password", required_argument, NULL, 'p'},
		{"debug",    no_argument,		NULL, 'd'},
		{NULL, no_argument, NULL, 0}
	};
	const char *optstring = "hk:u:s:p:d";
	int opt, optli;
	char optc, *name, *key, *server, *user, *pass;

	/* Initializa vars */
	name = key = server = user = pass = NULL;

	/* Quit callback */
	memset(&newact, 0, sizeof(newact));
	newact.sa_handler = do_quit;
	sigaction(SIGINT, &newact, NULL);

	/* init gthreads */
	g_thread_init(NULL);

	/* Treat command line */
	optc  = 0x00; 
	debug = 0x00;
	opt   = getopt_long(argc, argv, optstring, longOpts, &optli);
	while(opt != -1) {
		switch(opt) {
			case 'k':
				optc |= 0x02;
				key = strdup(optarg);
				break;

			case 'u':
				optc |= 0x04;
				user = strdup(optarg);
				break;
	
			case 's':
				optc |= 0x08;
				server = strdup(optarg);
				break;

			case 'p':
				optc |= 0x10;
				pass = strdup(optarg);
				break;

			case 'd':
				debug = 1;
				break;

			case 'h':
			default:
				optc |= 0x01;
		}
		opt = getopt_long(argc, argv, optstring, longOpts, &optli);
	}

	/* Check arguments */
	if ((optc & 0x1E) == 0x1E)  {
		/* Create the bot */
		
		/* Start XMPP protocol */
		if( (proto = new_XMPP()) == NULL ) {
			exit(EXIT_FAILURE);
		}

		bot.im       = proto;
		bot.name     = name;
		bot.user     = user;
		bot.server   = server;
		bot.password = pass;
		bot.key      = key;
		//bot.info = get_info();
		bot.users   = g_slist_alloc();
		bot.running = g_slist_alloc();

		/* Connect to the server */
		if(proto->connect(server, user, pass) == FALSE) {
			b_print("Connection failed: %s\n", proto->getError());
			return(EXIT_FAILURE);
		}
	} else if (optc != 0x01) {
		show_help(argv[0]);
		return(EXIT_FAILURE);
	} else if (optc == 0x01) {
		show_help(argv[0]);
		return(EXIT_SUCCESS);
	}

	/* Main loop */
	main_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(main_loop);

	free(bot.name);
	free(bot.user);
	free(bot.server);
	free(bot.password);
	free(bot.key);

	g_slist_free(bot.users);
	g_slist_free(bot.running);

	return(EXIT_FAILURE);
}


/**
 * Disconnect from server and exit program
 */
void do_quit(int sig)
{
	proto->disconnect();
	destroy_XMPP(proto);
	exit(EXIT_SUCCESS);
}


/**
 * Print info/debug messages
 */
void b_print(const char *format, ...)
{
	va_list ap;
	if (debug) {
		va_start(ap, format);
		vfprintf(stdout, format, ap);
		va_end(ap);
	}
}


/**
 * Show help
 */
void show_help(const char *progname)
{
	printf("BOTCON - An Instant Messenger Bot for Remote Control\n\n");
	printf("Use: %s -k <key> -u <username> -s <server> -p <password> [-d] [-h]\n\n", progname);
	printf("    -k, --key        Set the access key\n");
	printf("    -u, --username   XMPP server username\n");
	printf("    -s, --server     XMPP server\n");
	printf("    -p, --password   XMPP server password\n");
	printf("    -d, --debug      Show debug messages\n");
	printf("    -h, --help       Show this help\n\n");
}


