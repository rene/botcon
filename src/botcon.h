/**
 * BOTCON - An Instant Messenger Bot
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

#ifndef BOTCON_H

	#define BOTCON_H 1

	#ifdef HAVE_CONFIG_H
		#include <config.h>
	#else
		#error config.h not found. Are you running configure script?
	#endif

	#include <stdio.h>
	#include <strings.h>
	#include <stdarg.h>
	#include <signal.h>
	#include <getopt.h>
	#include <glib.h>
	#include "protocol.h"
	#include "xmpp.h"

	struct _botcon {
		protocol *im;
		char *name;
		char *user;
		char *password;
		char *server;
		char *info;
		char *key;
		GSList *users;    /* Users logged in */
		GSList *running;  /* Users running proccess */
	};

	struct _userproc {
		GString *user;
		GString *filename;
		int in[2];
		int out[2];
		pid_t child;
	};

	typedef struct _botcon botcon_t;
	typedef struct _userproc userproc_t;

	/* Prototypes */
	void b_print(const char *format, ...);

#endif

