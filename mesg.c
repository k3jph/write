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

/*
 *  Mesg(1) toggles the group-write permissions on a user's tty.  An on
 *  status allows for another user to write to the tty using write(1),
 *  talk(1), etc.
 */

#include "mesg.h"

void main(int argc, char *argv[]) 
{
	struct stat buf;
	int ch, quiet = 0;
	char *status = NULL, *tty;

	/*  Sets the default terminal to the current standards error.  */
	tty = ttyname(STDERR_FILENO);
	while ((ch = getopt(argc, argv, "t:q?")) != EOF)
		switch(ch) {
		case 'q':
			quiet = 1;
				break;
		case 't':
			tty = optarg;
			break;
		case '?':
			help();
			/* NOTREACHED */
		}
	while (--argc != 0)
		if (argv[argc][0] != '-' && argv[argc] != tty)
			status = argv[argc];
	/* 
	 *  Moves to the device directory so that calls such as ``mesg -t
	 *  ttyp0'' work without having to prepend the directory name.
	 */
	(void)chdir(_PATH_DEV);
	if(stat(tty, &buf) != 0) {
		if(!quiet)
			(void)perror(tty);
		(void)exit(2);
	}
	if(status == NULL) {
		if(buf.st_mode & S_IWGRP) {
			if(!quiet)
				(void)fprintf(stderr, "is y\n");
			(void)exit(0);
		}
		if(!quiet)
			(void)fprintf(stderr, "is n\n");
		(void)exit(1);
	}		

	/*  By checking status[0], "y", "yes", "yummy", etc., all are 'y'  */
	if(status[0] == 'y') {
		if(chmod(tty, buf.st_mode | S_IWGRP) != 0) {
			if(!quiet) 
				(void)perror(tty);
			(void)exit(2);
		}
		(void)exit(0);
	}
	if(status[0] == 'n') {
		if(chmod(tty, buf.st_mode & ~S_IWGRP) != 0) {
			if(!quiet)
				(void)perror(tty);
			(void)exit(2);
		}
		(void)exit(1);
	}
	/*  
	 *  This is only reached if the user entered an invalid parameter 
	 *  for the new status of the tty.
	 */
	(void)exit(2);
}

void help() 
{

	(void)fprintf(stderr, "\
usage: mesg [-q?] [-t tty] [y | n]\n\
	-q		-- quiet mode, suppress all output\n\
	-t tty		-- specify the tty to check/modify\n\
	-?		-- print this help message\n");
	(void)exit(2);
}
