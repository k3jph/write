/*
 *  Write 1.0.0
 *  Copyright (C) 1997 James P. Howard, II <jh@jameshoward.us>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <errno.h>
#include <paths.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include "write.h"

int loop(struct person *from, struct person *to, struct person *auth)
{
	struct servent *service;
	time_t t; int c;
	struct hostent *host;
	char hostname[MAXHOSTNAMELEN], tty[UT_LINESIZE +
	    sizeof(_PATH_DEV)] = _PATH_DEV, data[1024] = "\0", *d = data,
	    clock[8];

	t = time(NULL);
	(void)strftime((char *)&clock, 8, "%H:%M", localtime(&t));
	if (to->pe_host == NULL) {
		(void)strcat((char *)&tty, to->pe_tty);
		if ((terminal = fopen(tty, "a")) == NULL) {
			perror("write");
			return 1;
		}
		if (net == 0) {
			(void)fprintf(terminal, 
			    "\n\a\a\aMessage from %s on %s at %s...\r\n", 
			    from->pe_user, from->pe_tty, clock);
			(void)fprintf(stderr, "Message to %s on %s at %s...\r\n",
			    to->pe_user, to->pe_tty, clock);
		}
		else {
			if (auth == NULL)
				(void)fprintf(terminal,
				    "\n\a\a\aMessage from %s@%s [UNAUTHENTICATED] on %s at %s...\r\n",
				    from->pe_user, from->pe_host, from->pe_tty,
				    clock);
			else
				(void)fprintf(terminal,
				    "\n\a\a\aMessage from %s@%s (%s@%s) on %s at %s...\r\n",
				    from->pe_user, from->pe_host, auth->pe_user, 
				    auth->pe_host, from->pe_tty, clock);
			gethostname((char *)&hostname, MAXHOSTNAMELEN);
			if ((host = gethostbyname(hostname)) == NULL) {
				herror("write");
				return -1;
			}
			to->pe_host = strdup((char *)host->h_name);
			(void)fprintf(stderr, 
			    "Message to %s@%s on %s at %s...\n\n",
			    to->pe_user, to->pe_host, to->pe_tty, clock);
		}
	}
	else {
		if ((service = getservbyname("write", "tcp")) == NULL) {
			fprintf(stderr, "write: Service unknown\n");
			return 1;
		}
		if ((terminal = openclient(to->pe_host, service->s_port,
		    SOCK_STREAM)) == NULL) 
			return 1;
		if (strncmp(to->pe_tty, "", UT_LINESIZE) == 0) 
			(void)fprintf(terminal, "%s@%s@%s %s\n",
			    from->pe_user, from->pe_host, from->pe_tty,
			    to->pe_user); 
		else 
			(void)fprintf(terminal, "%s@%s@%s %s %s\n",
			    from->pe_user, from->pe_host, from->pe_tty,
			    to->pe_user, to->pe_tty);
		signal(SIGPIPE, pipeerr);
		while (strcmp(data, "\n") != 0 && !feof(terminal)) {
			(void)fputs(data, stderr);
			(void)fgets(data, 1024, terminal);
		}
		if (feof(terminal))
			return 1;
	}
	signal(SIGHUP, messyexit);
	signal(SIGINT, messyexit);
	signal(SIGQUIT, messyexit);
	while (!feof(stdin) && !feof(terminal) && !ferror(terminal) &&
	    !fflush(terminal) && !ferror(stdin)) {
		(void)fgets((char *)&data, 1024, stdin);
		if (data[0] == '!' && net == 0 && !feof(stdin))
			(void)system(d + 1);
		else if (!feof(stdin)) 
			(void)fputs(data, terminal);
		if (ferror(terminal) || feof(terminal)) {
			fprintf(stderr, 
			    "write: Connection closed by foreign host.\n");
			return -1;		
		}
	}
	if (net == 1 && to->pe_host != NULL) 
		if (auth == NULL)
			fprintf(terminal, "EOF [%s@%s]\r\n",
			    from->pe_user, from->pe_host);
		else 
			fprintf(terminal, "EOF [%s@%s (%s@%s)]\r\n",
			    from->pe_user, from->pe_host, auth->pe_user, 
			    auth->pe_host);
	else if (to->pe_host == NULL)
		fprintf(terminal, "EOF [%s]\n", from->pe_user);
	return 0;
}

void messyexit(int nil) {

	fprintf(terminal, "write: Remote user has terminated\n");
	fflush(terminal);
	fclose(terminal);
	exit(1);
}

void pipeerr(int nil) {

	fprintf(stderr, "write: Network communication error\n");
	fclose(terminal);
	exit(1);
}

int getcaller(struct person *me)
{
	struct passwd *ruser, *euser;
	struct hostent *host;
	char hostname[MAXHOSTNAMELEN];

	/*
	 *  This fetches the caller's username from getlogin(2) (which is
	 *  undocumentation under Linux...?).  If that fails, it gets the
	 *  caller's name by looking up the username in passwd(5) with the
	 *  UID returned by getuid(2).
	 */
	if (strncmp(strncpy((char *)&me->pe_user, getlogin(),
	    UT_NAMESIZE), "", UT_NAMESIZE) == 0)
		if ((ruser = getpwuid(getuid())) == NULL) {
			fprintf(stderr, "write: Cannot determine caller\n");
			return -1;
		}
		else 
			strncpy((char *)&me->pe_user, ruser->pw_name,
			    UT_NAMESIZE);
	/*
	 *  This finds the localhost's address.  First we find our local
	 *  host name, then we do a DNS lookup to find the fully qualified
	 *  domain name.  Then we make a pointer in me to the static field
	 *  in host containing the domain name.
	 */
	gethostname((char *)&hostname, MAXHOSTNAMELEN);
	if ((host = gethostbyname(hostname)) == NULL) {
		herror("write");
		return -1;
	}
	me->pe_host = strdup((char *)host->h_name);
	/*
	 *  Here, we first find out whether or not the descriptor
	 *  associated with the standard error is a terminal.  If it is,
	 *  we fill the pe_tty field with the name of the device 
	 *  associated with the standard error.  We add the lenght of
	 *  _PATH_DEV in order to strip off the directory name and get
	 *  merely the device name.
	 */
	if (isatty(STDERR_FILENO) == 0) {
		fprintf(stderr, "write: Cannot locate terminal\n");
		return -1;
	}
	if (strncmp(strncpy((char *)&me->pe_tty, ttyname(STDERR_FILENO) + 
	    strlen(_PATH_DEV), UT_LINESIZE), "", UT_LINESIZE) == 0)
			return -1;
	return 0;
	
}

int makeaddr(struct person *user, char *addr)
{
	char *tmp1, *tmp2;

	/*
	 *  First we break up the address based on the standard Internet
	 *  email addressing scheme ("user@host").
	 */
	tmp1 = strsep((char **)&addr, "@");
	tmp2 = strsep((char **)&addr, "");
	/*
	 *  Next we look for an address that is actually in the format
	 *  "user#tty@host".
	 */
	if (rindex(tmp1, '#') != NULL) {
		(void)strncpy((char *)&user->pe_user, strsep((char
		    **)&tmp1, "#"), UT_NAMESIZE);
		user->pe_host = tmp2;
		(void)strncpy((char *)&user->pe_tty, strsep((char
		    **)&tmp1, ""), UT_LINESIZE);
		return 0;
	}
	/*
	 *  Our next case looks for an address like "user@host#tty".  We
	 *  also examine for an address in the form "user@host@tty".  This
	 *  may seem odd, but the orginal network-aware write(1) 
	 *  implementation from Project Athena used this format to 
	 *  transfer the identity of the caller to the receiver.
	 */
	if (tmp2 != NULL && (index(tmp2, '#') != NULL || index(tmp2, '@')
	    != NULL)) {
		(void)strncpy((char *)&user->pe_user, tmp1, UT_NAMESIZE);
		user->pe_host = strsep((char **)&tmp2, "#@");
		(void)strncpy((char *)&user->pe_tty, strsep((char
		    **)&tmp2, ""), UT_LINESIZE);
		return 0;
	}
	/*
	 *  Now we go back to the default Internet email address pattern.
	 */
	(void)strncpy((char *)&user->pe_user, tmp1, UT_NAMESIZE);
	user->pe_host = tmp2;
	return 0;
}

int getTTY(struct person *user)
{
	struct utmp *utent, best;
	struct stat ttystat, beststat;
	int count = 0;
	char tty[UT_LINESIZE + sizeof(_PATH_DEV)] = _PATH_DEV;

	beststat.st_atime = (time_t)0;
	setutent();
	/*
	 *  First, we check to see if we have been passed a blank tty
	 *  field or not.  If it is not blank, we find out who is logged
	 *  in on that device and what thier permissions are.
	 */
	if(strncmp(user->pe_tty, "", UT_LINESIZE) != 0) {
		(void)strncpy((char *)&best.ut_line, user->pe_tty,
		    UT_LINESIZE);
		if ((utent = getutline(&best)) != NULL && strncmp(
		    utent->ut_user, user->pe_user, UT_NAMESIZE) == 0) {
			(void)stat(strncat(tty, utent->ut_line,
			    UT_LINESIZE + strlen(_PATH_DEV)), &ttystat);
			if (!(ttystat.st_mode & S_IWGRP) && access(tty,
			    W_OK)) {
				(void)fprintf(stderr, 
				    "write: Permission denied\n");
				return -1;
			}
			endutent();
			return 0;
		}
		(void)fprintf(stderr, "write: User not logged in\n");
		endutent();
		return -1;
	}
	/*
	 *  Otherwise, we jump down here and begin seaching for ttys the
	 *  target is logged in on.  We find the one that has the most
	 *  recent access time and we are permitted to write to.
	 */
	for(;;) {
		if ((utent = getutent()) == NULL) {
			endutent();
			if (strncmp(user->pe_tty, "", UT_NAMESIZE) != 0)
				return 0;
			else {
				if (count > 0)
					(void)fprintf(stderr,
					    "write: Permission denied\n");
				else
					(void)fprintf(stderr,
					    "write: User not logged in\n");
				return -1;
			}
		}
		if (strncmp(utent->ut_user, user->pe_user, UT_NAMESIZE) ==
		    0 && utent->ut_type == USER_PROCESS) {
			count++;
			(void)stat(strcat((char *)&tty, utent->ut_line),
			    &ttystat);
			if ((ttystat.st_mode & S_IWGRP || !access(tty,
			    W_OK)) && ttystat.st_atime >=
			    beststat.st_atime) {
				(void)memcpy((void *)&beststat, (void
				    *)&ttystat, sizeof(struct stat));
				(void)strncpy((char *)&user->pe_tty,
				    utent->ut_line, UT_LINESIZE);
			}
		}
		(void)strcpy((char *)&tty, _PATH_DEV);
	}
}
