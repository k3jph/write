/*-
 * Copyright (c) 1997 James P. Howard, II <jh@jameshoward.us>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 *  Mesg(1) toggles the group-write permissions on a user's tty.  An on
 *  status allows for another user to write to the tty using write(1),
 *  talk(1), etc.
 */

#include <errno.h>
#include <paths.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

static void help();

int main(int argc, char *argv[]) 
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
