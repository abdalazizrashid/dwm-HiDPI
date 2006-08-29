/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <X11/Xutil.h>


typedef struct {
	const char *clpattern;
	const char *tpattern;
	Bool isfloat;
} Rule;

typedef struct {
	regex_t *clregex;
	regex_t *tregex;
} RReg;

/* static */

TAGS
RULES

static RReg *rreg = NULL;
static unsigned int len = 0;

static void
applytag()
{
	/* asserts sel != NULL */
	settitle(sel);
	if(!isvisible(sel))
		arrange(NULL);
	else
		drawstatus();
}

/* extern */

Client *
getnext(Client *c)
{
	for(; c && !isvisible(c); c = c->next);
	return c;
}

Client *
getprev(Client *c)
{
	for(; c && !isvisible(c); c = c->prev);
	return c;
}

void
initrregs()
{
	unsigned int i;
	regex_t *reg;

	if(rreg)
		return;
	len = sizeof(rule) / sizeof(rule[0]);
	rreg = emallocz(len * sizeof(RReg));

	for(i = 0; i < len; i++) {
		if(rule[i].clpattern) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, rule[i].clpattern, 0))
				free(reg);
			else
				rreg[i].clregex = reg;
		}
		if(rule[i].tpattern) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, rule[i].tpattern, 0))
				free(reg);
			else
				rreg[i].tregex = reg;
		}
	}
}

void
settags(Client *c)
{
	char prop[512];
	unsigned int i, j;
	regmatch_t tmp;
	Bool matched = False;
	XClassHint ch;

	if(XGetClassHint(dpy, c->win, &ch)) {
		snprintf(prop, sizeof(prop), "%s:%s:%s",
				ch.res_class ? ch.res_class : "",
				ch.res_name ? ch.res_name : "", c->name);
		for(i = 0; !matched && i < len; i++)
			if(rreg[i].clregex && !regexec(rreg[i].clregex, prop, 1, &tmp, 0)) {
				c->isfloat = rule[i].isfloat;
				for(j = 0; rreg[i].tregex && j < ntags; j++) {
					if(!regexec(rreg[i].tregex, tags[j], 1, &tmp, 0)) {
						matched = True;
						c->tags[j] = True;
					}
				}
			}
		if(ch.res_class)
			XFree(ch.res_class);
		if(ch.res_name)
			XFree(ch.res_name);
	}
	if(!matched)
		for(i = 0; i < ntags; i++)
			c->tags[i] = seltag[i];
	for(i = 0; i < ntags && !c->tags[i]; i++);
	c->weight = i;
}

void
tag(Arg *arg)
{
	unsigned int i;

	if(!sel)
		return;

	for(i = 0; i < ntags; i++)
		sel->tags[i] = False;
	sel->tags[arg->i] = True;
	applytag();
}

void
toggletag(Arg *arg)
{
	unsigned int i;

	if(!sel)
		return;

	sel->tags[arg->i] = !sel->tags[arg->i];
	for(i = 0; i < ntags && !sel->tags[i]; i++);
	if(i == ntags)
		sel->tags[arg->i] = True;
	applytag();
}
