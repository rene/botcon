/**
 * BOTCON - An Instant Messenger Bot
 *
 * Copyright (C)2008 Renê de Souza Pinto  (rene at grad.icmc.usp.br)
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

#include "xmpp.h"

extern char *proccessMessage(const char *user, const char *msg);

/**
 * Methods
 */
static int  connect(char *server, char *jid, char *passwd);
static void disconnect();
static char *getError();
static int  setState(int state);
static int  getState();
static int  removeUser(char *user);
static int  acceptUser(char *user);
static int  acceptRemove(char *user);
static int  blockUser(char *user);
static int  unblockUser(char *user);
static int  sendMessage(char *user, char *msg);

static void register_handlers();
static void connection_open_cb(LmConnection *con, gboolean result, protocol *info);
static void authentication_cb(LmConnection *con, gboolean result, gpointer user_data);
static LmHandlerResult callback_messages(LmMessageHandler *handler, LmConnection *con, LmMessage *msg, gpointer user_data);
static LmHandlerResult callback_presence(LmMessageHandler *handler, LmConnection *con, LmMessage *msg, gpointer user_data);


/**
 * Private attributes
 */
static protocol *self        = NULL;
static LmConnection *conn    = NULL;
static GError *conn_error    = NULL;

/**
 * instance class XMPP
 */
protocol *new_XMPP()
{
	protocol *newxmpp;

	b_print("new_XMPP(): called\n");

	newxmpp = (protocol*)malloc(sizeof(protocol));
	if(newxmpp == NULL) {
		b_print("new_XMPP(): FAIL: memory allocation failed!\n");
		return(NULL);
	} else {
		newxmpp->state           = ST_UNKNOWN;
		newxmpp->connect         = connect;
		newxmpp->disconnect      = disconnect;
		newxmpp->getError        = getError;
		newxmpp->setState        = setState;
		newxmpp->getState        = getState;
		newxmpp->acceptUser      = acceptUser;
		newxmpp->acceptRemove    = acceptRemove;
		newxmpp->addUser         = NULL;		/* TODO: Implement */
		newxmpp->removeUser      = removeUser;
		newxmpp->blockUser       = blockUser;
		newxmpp->unblockUser     = unblockUser;
		newxmpp->sendMessage     = sendMessage;
		newxmpp->loadUserList    = NULL;			/* TODO: Implement */
		newxmpp->proccessMessage = proccessMessage;
		self = newxmpp;
		b_print("new_XMPP(): OK: object created!\n");
		return(newxmpp);
	}
}


/**
 * Destroy class XMPP
 */
void destroy_XMPP(protocol *obj)
{
	b_print("destroy_XMPP(): called\n");
	free(obj);
	obj = self = NULL;
}


/**
 * Connect on server
 */
static int connect(char *server, char *jid, char *passwd)
{
	b_print("connect(): called\n");

	strcpy(self->server, server);
	strcpy(self->name, jid);
	strcpy(self->passwd, passwd);

	/* Create connection */
	conn = lm_connection_new(server);

	/* Register handlers */
	register_handlers();

	/* Connect */
	if( !lm_connection_open(conn, (LmResultFunction) connection_open_cb, self, NULL, &conn_error) )
		return(FALSE);

	return(TRUE);
}


/**
 * Disconnect from server
 */
static void disconnect()
{
	b_print("disconnect(): called\n");
	lm_connection_close(conn, NULL);
	lm_connection_unref(conn);
	return;
}


/**
 * Get error message from server
 */
static char *getError()
{
	if(conn_error != NULL) {
		return(conn_error->message);
	} else {
		return("unknwon error");
	}
}


/**
 * Set user state
 */
static int  setState(int state)
{
	char *sstate;
	LmMessage *mt;

	b_print("setState(): called\n");

	if(self == NULL)
		return(FALSE);

	switch(state) {
		case ST_UNKNOWN: /* We need to select one, so, select online state */
		case ST_ONLINE:
						sstate = "chat";
						break;

		case ST_AWAY:
						sstate = "away";
						break;

		case ST_BUSY:
						sstate = "dnd";
						break;

		default:
			return(FALSE);
	}

	mt = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_AVAILABLE);
	//lm_message_node_add_child(mt->node, "show", NULL);

	/* Send to server */
	if( !lm_connection_send(conn, mt, &conn_error) ) {
		lm_message_unref(mt);
		b_print("setState(): FAIL: error seting state\n");
		return(FALSE);
	}

	self->state = state;
	lm_message_unref(mt);
	return(TRUE);
}


/**
 * Return state
 */
static int  getState()
{
	if(self == NULL)
		return(FALSE);

	return(self->state);
}


/**
 * Remove user
 */
static int  removeUser(char *user)
{
	LmMessage *mt;

	b_print("removeUser(): called\n");

	mt = lm_message_new_with_sub_type(user, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_UNSUBSCRIBE);

	/* Send to server */
	if( !lm_connection_send(conn, mt, &conn_error) ) {
		lm_message_unref(mt);
		b_print("removeUser(): FAIL: error comunicating with server\n");
		return(FALSE);
	}

	lm_message_unref(mt);
	return(TRUE);
}


/**
 * Accpet remove user
 */
static int  acceptRemove(char *user)
{
	LmMessage *mt;

	b_print("acceptRemove(): called\n");

	mt = lm_message_new_with_sub_type(user, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_UNSUBSCRIBED);

	/* Send to server */
	if( !lm_connection_send(conn, mt, &conn_error) ) {
		lm_message_unref(mt);
		b_print("acceptRemove(): FAIL: error comunicating with server\n");
		return(FALSE);
	}

	lm_message_unref(mt);
	return(TRUE);
}


/**
 * Accpet user invitation
 */
static int  acceptUser(char *user)
{
	LmMessage *mt;

	b_print("acceptUser(): called\n");

	mt = lm_message_new_with_sub_type(user, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_SUBSCRIBED);

	/* Send to server */
	if( !lm_connection_send(conn, mt, &conn_error) ) {
		lm_message_unref(mt);
		b_print("acceptUser(): FAIL: error comunicating with server\n");
		return(FALSE);
	}

	lm_message_unref(mt);
	return(TRUE);
}


/**
 * Block a user
 */
static int  blockUser(char *user)
{
	b_print("blockUser(): called  with %s !!! NOT IMPLEMENTED !!!\n", user);
	return(FALSE);
}


/**
 * Unblock a user
 */
static int  unblockUser(char *user)
{
	b_print("unblockUser(): called  with %s !!! NOT IMPLEMENTED !!!\n", user);
	return(FALSE);
}


/**
 * Send a message to user
 */
static int  sendMessage(char *user, char *msg)
{
	LmMessage *mt;
	GString *str = g_string_new(msg);

	b_print("sendMessage(): called\n");

	mt = lm_message_new_with_sub_type(user, LM_MESSAGE_TYPE_MESSAGE, LM_MESSAGE_SUB_TYPE_CHAT);
	lm_message_node_add_child(mt->node, "body", str->str);

	/* Send to server */
	if( !lm_connection_send(conn, mt, &conn_error) ) {
		lm_message_unref(mt);
		b_print("sendMessage(): FAIL: error comunicating with server\n");
		return(FALSE);
	}

	lm_message_unref(mt);
	g_string_free(str, TRUE);
	return(TRUE);
}


/**
 * Register handlers
 */
static void register_handlers()
{
	LmMessageHandler *handler;

	b_print("register_handlers(): called\n");

	handler = lm_message_handler_new(callback_messages, NULL, NULL);
	lm_connection_register_message_handler(conn, handler, LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);
	lm_message_handler_unref(handler);

	handler = lm_message_handler_new(callback_presence, NULL, NULL);
	lm_connection_register_message_handler(conn, handler, LM_MESSAGE_TYPE_PRESENCE, LM_HANDLER_PRIORITY_NORMAL);
	lm_message_handler_unref(handler);
}


/**
 * Callbacks for authentication
 */
static void connection_open_cb(LmConnection *con, gboolean result, protocol *info)
{
	b_print("connection_open_cb(): called\n");
	lm_connection_authenticate(con, info->name, info->passwd, RESOURCE, (LmResultFunction) authentication_cb, info, FALSE, &conn_error);
}
static void authentication_cb(LmConnection *con, gboolean result, gpointer user_data)
{
	b_print("authentication_cb(): called\n");
	if(result == TRUE) {
		self->setState(ST_ONLINE);
	}
}


/**
 * Callback for incoming messages
 */
static LmHandlerResult callback_messages(LmMessageHandler *handler, LmConnection *con, LmMessage *msg, gpointer user_data)
{
	LmMessageNode *no, *body;
	char *user, *strmsg, *ans;
	GString *str = NULL;

	if(lm_message_get_sub_type(msg) == LM_MESSAGE_SUB_TYPE_CHAT) {
		no   = lm_message_get_node(msg);
		body = lm_message_node_get_child(no, "body");
		if(body != NULL) {
			user   = (char*)lm_message_node_get_attribute(no, "from");
			strmsg = (char*)lm_message_node_get_value(body);

			b_print("received message from: %s\n", user);

			/* Proccess message */
			ans = self->proccessMessage(user, strmsg);
			if(ans != NULL) {
				str = g_string_new(ans);
				self->sendMessage(user, str->str);
			}

			lm_message_node_unref(body);
		}
		lm_message_node_unref(no);

		g_string_free(str, TRUE);
		str = NULL;
	} else {
		b_print("unknwon type of message received\n");
		return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);
	}
	return(LM_HANDLER_RESULT_REMOVE_MESSAGE);
}


/**
 * Callback for presence messages
 */
static LmHandlerResult callback_presence(LmMessageHandler *handler, LmConnection *con, LmMessage *msg, gpointer user_data)
{
	LmMessageNode *no;
	LmMessageType type;
	char *user;

	/* Message info */
	type = lm_message_get_sub_type(msg);
	no   = lm_message_get_node(msg);
	if(no != NULL) {
		user = (char*)lm_message_node_get_attribute(no, "from");
	} else {
		user = NULL;
	}

	/* Actions */
	if(type == LM_MESSAGE_SUB_TYPE_SUBSCRIBE) {

		b_print("received invitation from: %s", user);

		// TODO: Check user in blacklist
		self->acceptUser(user); /* For while, just accept any user */

		lm_message_node_unref(no);
		return(LM_HANDLER_RESULT_REMOVE_MESSAGE);

	} else if(type == LM_MESSAGE_SUB_TYPE_UNSUBSCRIBE) {

		// Somebody does not wanna be our friend, Ok...
		self->acceptRemove(user);
	}

	return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);
}

