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

#ifndef BOTCON_PROTOCOL_DEF

	#define BOTCON_PROTOCOL_DEF 1

	/**
	 * States
	 */
	#define ST_ONLINE    1
	#define ST_AWAY      2
	#define ST_BUSY      3
	#define ST_UNKNOWN  -1


	/**
	 * This implementation was chosen because we just need one object per
	 * protocol. Thus, it's unnecessary pass the object pointer to each
	 * method function. Each function it's associated with just one 
	 * object (pointed by "self" variable).
	 * 
	 * See xmpp.c
	 */

	typedef struct skel_protocol {
		int    state;
		char   name[1024];
		char   passwd[1024];
		char   server[1024];
		int    (*connect)(char *, char *, char *);
		void   (*disconnect)(void);
		char*  (*getError)(void);
		int    (*setState)(int);
		int    (*getState)(void);
		int    (*acceptUser)(char *);
		int    (*acceptRemove)(char *);
		int    (*addUser)(char *);
		int    (*removeUser)(char *);
		int    (*blockUser)(char *);
		int    (*unblockUser)(char *);
		int    (*sendMessage)(char *, char *);
		int    (*loadUserList)(void);
		char*  (*proccessMessage)(const char *user, const char *msg);
	} protocol;


#endif /* BOTCON_PROTOCOL_DEF */

