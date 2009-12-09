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

#define PE_NAMESIZE 32
#define PE_LINESIZE 32

struct person
{
	char pe_user[PE_NAMESIZE];
	char pe_euser[PE_NAMESIZE];
	char pe_tty[PE_LINESIZE];
	char *pe_host;
};

extern FILE *terminal;
extern struct person to, from, auth;
extern int net;

void help(void);
void pipeerr(int nil);
void messyexit(int nil);
