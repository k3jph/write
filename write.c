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

void main(int argc, char *argv[])
{

	if (argc < 2)
		help();
	if (getcaller(&from) == -1)
		exit(1);
	(void)makeaddr(&to, argv[1]);
	if (argc > 2)
		(void)strncpy((char *)&to.pe_tty, argv[2], UT_LINESIZE);
	if (to.pe_host == NULL)
		if (getTTY(&to) == -1)
			exit(1);
	exit(loop(&from, &to, NULL));
}

void help(void)
{

	(void)fprintf(stderr, "write: Invalid communications protocol\n");
	exit(1);
}
