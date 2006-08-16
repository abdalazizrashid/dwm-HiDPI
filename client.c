/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"
#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

/* static functions */

static void
resizetitle(Client *c)
{
	int i;

	c->tw = 0;
	for(i = 0; i < ntags; i++)
		if(c->tags[i])
			c->tw += textw(tags[i]);
	c->tw += textw(c->name);
	if(c->tw > c->w)
		c->tw = c->w + 2;
	c->tx = c->x + c->w - c->tw + 2;
	c->ty = c->y;
	if(isvisible(c))
		XMoveResizeWindow(dpy, c->title, c->tx, c->ty, c->tw, c->th);
	else
		XMoveResizeWindow(dpy, c->title, c->tx + 2 * sw, c->ty, c->tw, c->th);

}

static int
xerrordummy(Display *dsply, XErrorEvent *ee)
{
	return 0;
}

/* extern functions */

void
ban(Client *c)
{
	XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
	XMoveWindow(dpy, c->title, c->tx + 2 * sw, c->ty);
}

void
focus(Client *c)
{
	Client *old = sel;

	if (!issel)
		return;
	if(sel && sel->ismax)
		togglemax(NULL);
	sel = c;
	if(old && old != c)
		drawtitle(old);
	drawtitle(c);
	XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
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

Client *
getclient(Window w)
{
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->win == w)
			return c;
	return NULL;
}

Client *
getctitle(Window w)
{
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->title == w)
			return c;
	return NULL;
}

void
gravitate(Client *c, Bool invert)
{
	int dx = 0, dy = 0;

	switch(c->grav) {
	default:
		break;
	case StaticGravity:
	case NorthWestGravity:
	case NorthGravity:
	case NorthEastGravity:
		dy = c->border;
		break;
	case EastGravity:
	case CenterGravity:
	case WestGravity:
		dy = -(c->h / 2) + c->border;
		break;
	case SouthEastGravity:
	case SouthGravity:
	case SouthWestGravity:
		dy = -(c->h);
		break;
	}

	switch (c->grav) {
	default:
		break;
	case StaticGravity:
	case NorthWestGravity:
	case WestGravity:
	case SouthWestGravity:
		dx = c->border;
		break;
	case NorthGravity:
	case CenterGravity:
	case SouthGravity:
		dx = -(c->w / 2) + c->border;
		break;
	case NorthEastGravity:
	case EastGravity:
	case SouthEastGravity:
		dx = -(c->w + c->border);
		break;
	}

	if(invert) {
		dx = -dx;
		dy = -dy;
	}
	c->x += dx;
	c->y += dy;
}

void
killclient(Arg *arg)
{
	if(!sel)
		return;
	if(sel->proto & PROTODELWIN)
		sendevent(sel->win, wmatom[WMProtocols], wmatom[WMDelete]);
	else
		XKillClient(dpy, sel->win);
}

void
manage(Window w, XWindowAttributes *wa)
{
	Client *c, *tc;
	Window trans;
	XSetWindowAttributes twa;

	c = emallocz(sizeof(Client));
	c->tags = emallocz(ntags * sizeof(Bool));
	c->win = w;
	c->x = c->tx = wa->x;
	c->y = c->ty = wa->y;
	c->w = c->tw = wa->width;
	c->h = wa->height;
	c->th = bh;

	c->border = 0;
	setsize(c);

	if(c->h != sh && c->y < bh)
		c->y = c->ty = bh;

	c->proto = getproto(c->win);
	XSelectInput(dpy, c->win,
		StructureNotifyMask | PropertyChangeMask | EnterWindowMask);
	XGetTransientForHint(dpy, c->win, &trans);
	twa.override_redirect = 1;
	twa.background_pixmap = ParentRelative;
	twa.event_mask = ExposureMask | EnterWindowMask;

	c->title = XCreateWindow(dpy, root, c->tx, c->ty, c->tw, c->th,
			0, DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &twa);

	if(clients)
		clients->prev = c;
	c->next = clients;
	clients = c;

	XGrabButton(dpy, Button1, MODKEY, c->win, False, BUTTONMASK,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button1, MODKEY | LockMask, c->win, False, BUTTONMASK,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button1, MODKEY | numlockmask, c->win, False, BUTTONMASK,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button1, MODKEY | numlockmask | LockMask, c->win, False, BUTTONMASK,
			GrabModeAsync, GrabModeSync, None, None);

	XGrabButton(dpy, Button2, MODKEY, c->win, False, BUTTONMASK,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button2, MODKEY | LockMask, c->win, False, BUTTONMASK,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button2, MODKEY | numlockmask, c->win, False, BUTTONMASK,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button2, MODKEY | numlockmask | LockMask, c->win, False, BUTTONMASK,
			GrabModeAsync, GrabModeSync, None, None);

	XGrabButton(dpy, Button3, MODKEY, c->win, False, BUTTONMASK,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button3, MODKEY | LockMask, c->win, False, BUTTONMASK,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button3, MODKEY | numlockmask, c->win, False, BUTTONMASK,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button3, MODKEY | numlockmask | LockMask, c->win, False, BUTTONMASK,
			GrabModeAsync, GrabModeSync, None, None);

	settags(c);
	if(!c->isfloat)
		c->isfloat = trans
			|| (c->maxw && c->minw &&
				c->maxw == c->minw && c->maxh == c->minh);
	settitle(c);

	if(trans && (tc = getclient(trans))) {
		c->x = (tc->x + tc->w / 2) - (c->w / 2);
		c->y = (tc->y + tc->h / 2) - (c->h / 2);
		if(c->x < 0 || c->x >= sw)
			c->x = (sw / 2) - (c->w / 2);
		if(c->y < 0 || c->y >= sh)
			c->y = ((sh - bh) / 2) - (c->h / 2) + bh;
	}
	else {
		c->x = (sw / 2) - (c->w / 2);
		c->y = ((sh - bh) / 2) - (c->h / 2) + bh;
	}

	if(isvisible(c))
		sel = c;
	arrange(NULL);
	XMapWindow(dpy, c->win);
	XMapWindow(dpy, c->title);
	if(isvisible(c))
		focus(c);
}

void
resize(Client *c, Bool sizehints, Corner sticky)
{
	int bottom = c->y + c->h;
	int right = c->x + c->w;
	/*XConfigureEvent e;*/
	XWindowChanges wc;

	if(sizehints) {
		if(c->incw)
			c->w -= (c->w - c->basew) % c->incw;
		if(c->inch)
			c->h -= (c->h - c->baseh) % c->inch;
		if(c->minw && c->w < c->minw)
			c->w = c->minw;
		if(c->minh && c->h < c->minh)
			c->h = c->minh;
		if(c->maxw && c->w > c->maxw)
			c->w = c->maxw;
		if(c->maxh && c->h > c->maxh)
			c->h = c->maxh;
	}
	if(c->x > right) /* might happen on restart */
		c->x = right - c->w;
	if(c->y > bottom)
		c->y = bottom - c->h;
	if(sticky == TopRight || sticky == BotRight)
		c->x = right - c->w;
	if(sticky == BotLeft || sticky == BotRight)
		c->y = bottom - c->h;

	resizetitle(c);
	wc.x = c->x;
	wc.y = c->y;
	wc.width = c->w;
	wc.height = c->h;
	if(c->w == sw && c->h == sh)
		wc.border_width = 0;
	else
		wc.border_width = 1;
	XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
	XSync(dpy, False);
}

void
setsize(Client *c)
{
	long msize;
	XSizeHints size;

	if(!XGetWMNormalHints(dpy, c->win, &size, &msize) || !size.flags)
		size.flags = PSize;
	c->flags = size.flags;
	if(c->flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	}
	else
		c->basew = c->baseh = 0;
	if(c->flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	}
	else
		c->incw = c->inch = 0;
	if(c->flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	}
	else
		c->maxw = c->maxh = 0;
	if(c->flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	}
	else
		c->minw = c->minh = 0;
	if(c->flags & PWinGravity)
		c->grav = size.win_gravity;
	else
		c->grav = NorthWestGravity;
}

void
settitle(Client *c)
{
	char **list = NULL;
	int n;
	XTextProperty name;

	name.nitems = 0;
	c->name[0] = 0;
	XGetTextProperty(dpy, c->win, &name, netatom[NetWMName]);
	if(!name.nitems)
		XGetWMName(dpy, c->win, &name);
	if(!name.nitems)
		return;
	if(name.encoding == XA_STRING)
		strncpy(c->name, (char *)name.value, sizeof(c->name));
	else {
		if(XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success
				&& n > 0 && *list)
		{
			strncpy(c->name, *list, sizeof(c->name));
			XFreeStringList(list);
		}
	}
	XFree(name.value);
	resizetitle(c);
}

void
togglemax(Arg *arg)
{
	int ox, oy, ow, oh;
	XEvent ev;

	if(!sel)
		return;

	if((sel->ismax = !sel->ismax)) {
		ox = sel->x;
		oy = sel->y;
		ow = sel->w;
		oh = sel->h;
		sel->x = sx;
		sel->y = sy + bh;
		sel->w = sw - 2;
		sel->h = sh - 2 - bh;

		restack();
		resize(sel, arrange == dofloat, TopLeft);

		sel->x = ox;
		sel->y = oy;
		sel->w = ow;
		sel->h = oh;
	}
	else
		resize(sel, False, TopLeft);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
unmanage(Client *c)
{
	XGrabServer(dpy);
	XSetErrorHandler(xerrordummy);

	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
	XDestroyWindow(dpy, c->title);

	if(c->prev)
		c->prev->next = c->next;
	if(c->next)
		c->next->prev = c->prev;
	if(c == clients)
		clients = c->next;
	if(sel == c)
		sel = getnext(clients);
	free(c->tags);
	free(c);

	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XUngrabServer(dpy);
	if(sel)
		focus(sel);
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
