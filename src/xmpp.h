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

#ifndef BOTCON_XMPP

	#define BOTCON_XMPP 1

	#include <stdlib.h>
	#include <string.h>
	#include <strings.h>
	#include <loudmouth/loudmouth.h>
	#include "protocol.h"
	#include "botcon.h"

	#define RESOURCE "Botcon"

	protocol *new_XMPP();
	void destroy_XMPP(protocol *obj);

#endif /* BOTCON_XMPP */

