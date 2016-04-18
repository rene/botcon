/**
 * BOTCON - An Instant Messenger Bot
 *
 * Copyright (C)2008 Renê de Souza Pinto  (rene at renesp.com.br)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <regex.h>
#include <unistd.h>
#include <glib.h>
#include "botcon.h"

#define BUFFER_LEN 4096
#define DEFAULT_PROCCESS g_string_new("/bin/bash")

static char *ctrl_help(void);
static char *ctrl_login(const char *user, const char *msg);
static char *ctrl_logout(const char *user);
static char *create_user_proc(GString *user, GString *filename);
static gint mfind_user(gconstpointer a, gconstpointer b);
static gboolean muser_islogged(const char *user);
static gpointer proccess_monitor(gpointer user_proc);
void sigchld_handler(int sig);

extern botcon_t bot;

GString *m_ans = NULL;

/**
 * Function to receive and response messages
 */
char *proccessMessage(const char *user, const char *msg)
{
	GSList *plist;
	userproc_t *proc;
	GString *puser, *com;
	ssize_t bytes;

	/* Response message */
	if (m_ans == NULL)
		m_ans = g_string_new("");
	else
		m_ans = g_string_erase(m_ans, 0, -1);

	/* Check if the user already running a proccess */
	for (plist = bot.running; plist != NULL; plist = g_slist_next(plist)) {
		proc = plist->data;
		if (proc != NULL) {
			puser = proc->user;
			if (strcmp(puser->str, user) == 0) {
				/* Write in pipe's proccess */
				com = g_string_new(msg);
				com = g_string_append(com, "\n");
				bytes = strlen(com->str);
				if (write(proc->in[1], com->str, bytes) < bytes) {
					return("Error to write on proccess pipe.\n");
				} else {
					return(NULL);
				}
				g_string_free(com, TRUE);
			}
		}
	}

	/* Check for a command */
	if (msg[0] == '#') {
		/* Parse the command */

		/* HELP */
		if (strcasecmp(&msg[1], "help") == 0) {
			return(ctrl_help());

		/* LOGIN */
		} else if (strncasecmp(&msg[1], "login", 5) == 0) {
			return(ctrl_login(user, msg));

		/* LOGOUT */
		} else if (strncasecmp(&msg[1], "logout", 6) == 0) {
			return(ctrl_logout(user));

		/* INVALID COMMAND */
		} else {
			m_ans = g_string_append(m_ans, "Ops, invalid command :(");
			return(m_ans->str);
		}
	} else {
		m_ans = g_string_append(m_ans, "Please, send to me #help to see commands available.");
	}

	return(m_ans->str);
}


/**
 * Show help
 */
static char *ctrl_help(void)
{
	char *msg = "\n\
Hi, I am the BOTCON, an instant messenger BOT for machine remote control. The commands avaliable are: \n\n\
    #login <password> - Login at the BOT\n\
    #logout - Logout from BOT\n\
";
	m_ans = g_string_append(m_ans, msg);
	return(m_ans->str);
}

/**
 * Login at Master BOT
 */
static char *ctrl_login(const char *user, const char *msg)
{
	regex_t rexp;
	regmatch_t match;
	int error, len;
	GString *login, *newuser;
	GSList *element;
	struct sigaction newact;

	regcomp(&rexp, " {1,}(\\w+)", REG_EXTENDED | REG_ICASE);

	error = regexec(&rexp, msg, 1, &match, 0);
	if (error == 0) {
		len   = match.rm_eo - match.rm_so;
		login = g_string_sized_new(len);
		g_string_insert_len(login, 0, (msg+match.rm_so+1), len);

		if (strncasecmp(login->str, bot.key, strlen(bot.key)) == 0) {
			/* Log in */
			newuser = g_string_new(user);
			if ((element = g_slist_find_custom(bot.users, newuser, mfind_user)) != NULL) {
				m_ans = g_string_append(m_ans, "You are already logged in. Please, logout first.");
				return(m_ans->str);
			}
			bot.users = g_slist_append(bot.users, newuser);

			m_ans = g_string_append(m_ans, "Login OK. Executing user proccess...\n");

			m_ans = g_string_append(m_ans, create_user_proc(newuser, DEFAULT_PROCCESS));

			memset(&newact, 0, sizeof(newact));
			newact.sa_handler = sigchld_handler;
			sigaction(SIGCHLD, &newact, NULL);
		} else {
			m_ans = g_string_append(m_ans, "Invalid key :(!");
		}
	} else {
		m_ans = g_string_append(m_ans, ":( Command invalid syntax, use:\n    #login <password>\n");
	}
	
	return(m_ans->str);
}

/**
 * Logout at Master BOT
 */
static char *ctrl_logout(const char *user)
{
	GString *olduser;
	GSList *element;

	olduser = g_string_new(user);
	if ((element = g_slist_find_custom(bot.users, olduser, mfind_user)) != NULL) {
		bot.users = g_slist_remove_link(bot.users, element);
		m_ans = g_string_append(m_ans, "Bye Bye :)");
	} else {
		m_ans = g_string_append(m_ans, "You are not logged in! :(");
	}
	
	return(m_ans->str);
}


/**
 * Create an user proccess
 */
static char *create_user_proc(GString *user, GString *filename)
{
	userproc_t *newproc;
	GString *res;
	pid_t child;
	char *args[] = { NULL, NULL };
	int d1, d2;

	res = g_string_new("");
	newproc = (userproc_t*)g_malloc(sizeof(userproc_t));
	if (newproc != NULL) {
		newproc->user = user;
		newproc->filename = filename;
		
		/* Create pipes */
		if (pipe(newproc->in) < 0 || pipe(newproc->out) < 0) {
			res = g_string_append(res, "Error: could not create pipes.\n");
		}

		/* Make fork */
		if ((child = fork()) == 0) {
			/* Child */

			/* Redirect standard input and ouput */
			close(STDIN_FILENO);
			d1 = dup(newproc->in[0]);
			close(newproc->in[0]);
			close(newproc->in[1]);

			close(STDOUT_FILENO);
			d2 = dup(newproc->out[1]);
			close(newproc->out[0]);
			close(newproc->out[1]);
			
			if (d1 < 0 || d2 < 0) {
				g_warning("Could not redirect input/output descriptors.\n");
			}

			args[0] = newproc->filename->str;
			execvp(args[0], args);

			exit(EXIT_FAILURE);
		} else {
			/* Father */
			close(newproc->in[0]);
			close(newproc->out[1]);

			newproc->child = child;

			bot.running = g_slist_append(bot.running, newproc);

			/* Create thread to forward output proccess */
			g_thread_create(proccess_monitor, newproc, TRUE, NULL);
		}
	}

	return(res->str);
}


/**
 * Check if user is logged in
 */
static gboolean muser_islogged(const char *user)
{
	GString *tmp;
	GSList *element;

	tmp = g_string_new(user);
	if ((element = g_slist_find_custom(bot.users, tmp, mfind_user)) != NULL) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}


/**
 * Function to search user in users list
 */
static gint mfind_user(gconstpointer a, gconstpointer b)
{
	GString *astr = (GString*)a;
	GString *bstr = (GString*)b;

	if (astr != NULL && bstr != NULL) {
		if (astr->str != NULL && bstr->str != NULL) {
			return(strcmp(astr->str, bstr->str));
		} else {
			return(-1);
		}
	} else {
		return(-1);
	}
}


/**
 * Thread to read from pipe
 */
static gpointer proccess_monitor(gpointer user_proc)
{
	userproc_t *uproc = (userproc_t*)user_proc;
	ssize_t bytes;
	char buffer[BUFFER_LEN];
	GString *msg;

	while((bytes = read(uproc->out[0], buffer, BUFFER_LEN)) > 0) {
		buffer[bytes] = '\0';
		msg = g_string_new("\n");
		msg = g_string_append(msg, buffer);
		bot.im->sendMessage(uproc->user->str, msg->str);
		g_string_free(msg, TRUE);
	}

	return NULL;
}

/**
 * Signal handling
 */
void sigchld_handler(int sig)
{
	GSList *plist;
	userproc_t *proc;
	GString *puser;
	int status;

	if (sig != SIGCHLD)
		return;

	/* Check children status */
	for (plist = bot.running; plist != NULL; plist = g_slist_next(plist)) {
		proc = plist->data;
		if (proc != NULL) {
			puser = proc->user;

			if (waitpid(proc->child, &status, WNOHANG) >= 0) {
				if (WIFEXITED(status) || WIFSIGNALED(status)) {
					bot.im->sendMessage(proc->user->str, "Proccess finished.\n");
					bot.running = g_slist_remove_link(bot.running, plist);
					ctrl_logout(proc->user->str);
					g_free(proc);
				}
			}
		}
	}
}

