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

void (*arrange)(Arg *) = DEFMODE;

/* extern */

void
appendtag(Arg *arg)
{
	if(!sel)
		return;

	sel->tags[arg->i] = True;
	settitle(sel);
}

void
dofloat(Arg *arg)
{
	Client *c;

	for(c = clients; c; c = c->next) {
		c->ismax = False;
		if(isvisible(c)) {
			resize(c, True, TopLeft);
		}
		else
			ban(c);
	}
	if((sel = getnext(clients))) {
		focus(sel);
		restack();
	}
	else
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
}

void
dotile(Arg *arg)
{
	int h, i, n, w;
	Client *c;

	w = sw - mw;
	for(n = 0, c = clients; c; c = c->next)
		if(isvisible(c) && !c->isfloat)
			n++;

	if(n > 1)
		h = (sh - bh) / (n - 1);
	else
		h = sh - bh;

	for(i = 0, c = clients; c; c = c->next) {
		c->ismax = False;
		if(isvisible(c)) {
			if(c->isfloat) {
				resize(c, True, TopLeft);
				continue;
			}
			if(n == 1) {
				c->x = sx;
				c->y = sy + bh;
				c->w = sw - 2;
				c->h = sh - 2 - bh;
			}
			else if(i == 0) {
				c->x = sx;
				c->y = sy + bh;
				c->w = mw - 2;
				c->h = sh - 2 - bh;
			}
			else if(h > bh) {
				c->x = sx + mw;
				c->y = sy + (i - 1) * h + bh;
				c->w = w - 2;
				if(i + 1 == n)
					c->h = sh - c->y - 2;
				else
					c->h = h - 2;
			}
			else { /* fallback if h < bh */
				c->x = sx + mw;
				c->y = sy + bh;
				c->w = w - 2;
				c->h = sh - 2 - bh;
			}
			resize(c, False, TopLeft);
			i++;
		}
		else
			ban(c);
	}
	if((sel = getnext(clients)))
		focus(sel);
	else
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	restack();
}

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

Bool
isvisible(Client *c)
{
	unsigned int i;

	for(i = 0; i < ntags; i++)
		if(c->tags[i] && seltag[i])
			return True;
	return False;
}

void
replacetag(Arg *arg)
{
	int i;

	if(!sel)
		return;

	for(i = 0; i < ntags; i++)
		sel->tags[i] = False;
	appendtag(arg);
}

void
restack()
{
	static unsigned int nwins = 0;
	static Window *wins = NULL;
	unsigned int f, fi, m, mi, n;
	Client *c;
	XEvent ev;

	for(f = 0, m = 0, c = clients; c; c = c->next)
		if(isvisible(c)) {
			if(c->isfloat || arrange == dofloat)
				f++;
			else
				m++;
		}
	if(!(n = 2 * (f + m))) {
		drawstatus();
		return;
	}
	if(nwins < n) {
		nwins = n;
		wins = erealloc(wins, nwins * sizeof(Window));
	}

	fi = 0;
	mi = 2 * f;
	if(sel->isfloat || arrange == dofloat) {
		wins[fi++] = sel->title;
		wins[fi++] = sel->win;
	}
	else {
		wins[mi++] = sel->title;
		wins[mi++] = sel->win;
	}
	for(c = clients; c; c = c->next)
		if(isvisible(c) && c != sel) {
			if(c->isfloat || arrange == dofloat) {
				wins[fi++] = c->title;
				wins[fi++] = c->win;
			}
			else {
				wins[mi++] = c->title;
				wins[mi++] = c->win;
			}
		}
	XRestackWindows(dpy, wins, n);
	drawall();
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
settags(Client *c)
{
	char classinst[256];
	unsigned int i, j;
	regmatch_t tmp;
	Bool matched = False;
	XClassHint ch;

	if(XGetClassHint(dpy, c->win, &ch)) {
		snprintf(classinst, sizeof(classinst), "%s:%s",
				ch.res_class ? ch.res_class : "",
				ch.res_name ? ch.res_name : "");
		for(i = 0; !matched && i < len; i++)
			if(rreg[i].clregex && !regexec(rreg[i].clregex, classinst, 1, &tmp, 0)) {
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
}

void
togglemode(Arg *arg)
{
	arrange = arrange == dofloat ? dotile : dofloat;
	arrange(NULL);
}

void
view(Arg *arg)
{
	unsigned int i;

	for(i = 0; i < ntags; i++)
		seltag[i] = False;
	seltag[arg->i] = True;
	arrange(NULL);
}

void
toggleview(Arg *arg)
{
	unsigned int i;

	seltag[arg->i] = !seltag[arg->i];
	for(i = 0; !seltag[i] && i < ntags; i++);
	if(i == ntags)
		seltag[arg->i] = True; /* cannot toggle last view */
	arrange(NULL);
}
