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

#include "write.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

int ident(int fd, struct person *user)
{
	FILE *authentic;
	int socklen;
	struct sockaddr_in local, remote;
	struct servent *port;
	struct hostent *host;
	char dat[1024] = "\0", *tmp;

	socklen = sizeof(struct sockaddr_in);
	if (getpeername(STDIN_FILENO, (struct sockaddr *)&remote, (int
	    *)&socklen) == -1)
		return -1;
	socklen = sizeof(struct sockaddr_in);
	if (getsockname(STDIN_FILENO, (struct sockaddr *)&local, (int
	    *)&socklen) == -1)
		return -1;
	if ((port = getservbyname("auth", "tcp")) == NULL)
		return -1;
	if ((host = gethostbyaddr((char *)&remote.sin_addr,
	    sizeof(remote.sin_addr), PF_INET)) == NULL)
		user->pe_host = inet_ntoa(remote.sin_addr);
	else
		user->pe_host = strdup(host->h_name);
	if ((authentic = openclient(user->pe_host, port->s_port,
	    SOCK_STREAM)) == NULL)
		return -1;
	fprintf(authentic, "%u , %u \r\n", htons(remote.sin_port),
	    htons(local.sin_port));
	(void)fgets(dat, 1024, authentic);
	(void)strtok(dat, ":");
	(void)strtok((char *)NULL, ":");
	(void)strtok((char *)NULL, " :");
	if ((tmp = strtok((char *)NULL, " :\n\r")) == NULL)
		return -1; 
	(void)strncpy((char *)&user->pe_user, tmp, UT_NAMESIZE);
	return 0;
}

void main(int argc, char *argv[])
{
	int opt, noauth = 0;
	char cmdline[1024], *str;

	while ((opt  = getopt(argc, argv, "f")) != EOF)
		switch (opt) {
		case 'f':
			noauth = 1;
			break;
		}
	dup2(0, 1);
	dup2(0, 2);
	net = 1;
	fgets((char *)&cmdline, 1024, stdin);
	if ((str = strtok((char *)&cmdline, " \n\t\v\r")) == NULL)
		help();
	(void)makeaddr(&from, str);
	if ((str = strtok((char *)NULL, " \n\t\v\r")) == NULL)
		help();
	(void)makeaddr(&to, str);
	if ((str = strtok((char *)NULL, " \n\t\v\r")) != NULL)
		(void)strncpy((char *)&to.pe_tty, str, UT_LINESIZE);
	if (to.pe_host == NULL) 
		if (getTTY(&to) == -1)
			exit(1);
	if (strncmp(from.pe_user, "", UT_NAMESIZE) == 0 || from.pe_host ==
	    NULL || strncmp(from.pe_tty, "", UT_LINESIZE) == 0)
		help();
	if (ident(0, &auth) == 0)
		exit(loop(&from, &to, &auth));
	else 
		if (noauth == 0)
			exit(loop(&from, &to, (struct person *)NULL));
		else
			fprintf(stderr, "write: You cannot be authenticated\n");
	exit(1);
}

void help(void)
{

	(void)fprintf(stderr, "write: Invalid communications protocol\n");
	exit(1);
}
