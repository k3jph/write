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

#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

FILE *openclient(char *server, int port, int proto)
{
	struct hostent *serv;
	struct sockaddr_in addr;
	int s;

	if (!(serv = gethostbyname(server))) {
		if (net == 0)
			herror("net");
		return NULL;
	}
	memcpy(&addr.sin_addr, serv->h_addr, serv->h_length);
	addr.sin_family = serv->h_addrtype;
	addr.sin_port = port;
	if ((s = socket(addr.sin_family, proto, 0)) < 0) {
		if (net == 0)
			perror("net");
		return NULL;
	}
	if (connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr))) { 
		if (net == 0)
			perror("net");
		return NULL;
	}
	return fdopen(s, "a+");
}
