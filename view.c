/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"

/* extern */

void (*arrange)(Arg *) = DEFMODE;

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
	if(!sel || !isvisible(sel))
		sel = getnext(clients);
	if(sel)
		focus(sel);
	else
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	restack();
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
	if(!sel || !isvisible(sel))
		sel = getnext(clients);
	if(sel)
		focus(sel);
	else
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	restack();
}

void
focusnext(Arg *arg)
{
	Client *c;
   
	if(!sel)
		return;

	if(!(c = getnext(sel->next)))
		c = getnext(clients);
	if(c) {
		focus(c);
		restack();
	}
}

void
focusprev(Arg *arg)
{
	Client *c;

	if(!sel)
		return;

	if(!(c = getprev(sel->prev))) {
		for(c = clients; c && c->next; c = c->next);
		c = getprev(c);
	}
	if(c) {
		focus(c);
		restack();
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
togglemode(Arg *arg)
{
	arrange = arrange == dofloat ? dotile : dofloat;
	if(sel)
		arrange(NULL);
	else
		drawstatus();
}

void
toggleview(Arg *arg)
{
	unsigned int i;

	seltag[arg->i] = !seltag[arg->i];
	for(i = 0; i < ntags && !seltag[i]; i++);
	if(i == ntags)
		seltag[arg->i] = True; /* cannot toggle last view */
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
zoom(Arg *arg)
{
	Client *c;

	if(!sel || (arrange != dotile) || sel->isfloat || sel->ismax)
		return;

	if(sel == getnext(clients))  {
		if((c = getnext(sel->next)))
			sel = c;
		else
			return;
	}

	/* pop */
	sel->prev->next = sel->next;
	if(sel->next)
		sel->next->prev = sel->prev;
	sel->prev = NULL;
	clients->prev = sel;
	sel->next = clients;
	clients = sel;
	focus(sel);
	arrange(NULL);
}
