/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/* static */

static void
bad_malloc(unsigned int size)
{
	eprint("fatal: could not malloc() %u bytes\n", size);
}

/* extern */

void *
emallocz(unsigned int size)
{
	void *res = calloc(1, size);

	if(!res)
		bad_malloc(size);
	return res;
}

void
eprint(const char *errstr, ...)
{
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void
spawn(Arg *arg)
{
	static char *shell = NULL;

	if(!shell && !(shell = getenv("SHELL")))
		shell = "/bin/sh";

	if(!arg->cmd)
		return;
	if(fork() == 0) {
		if(fork() == 0) {
			if(dpy)
				close(ConnectionNumber(dpy));
			setsid();
			execl(shell, shell, "-c", arg->cmd, NULL);
			fprintf(stderr, "dwm: execl '%s'", arg->cmd);
			perror(" failed");
		}
		exit(0);
	}
	wait(0);
}
