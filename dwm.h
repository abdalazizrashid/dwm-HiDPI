/* (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance.  Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * Calls to fetch an X event from the event queue are blocking.  Due reading
 * status text from standard input, a select()-driven main loop has been
 * implemented which selects for reads on the X connection and STDIN_FILENO to
 * handle all data smoothly. The event handlers of dwm are organized in an
 * array which is accessed whenever a new event has been fetched. This allows
 * event dispatching in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag.  Clients are organized in a global
 * doubly-linked client list, the focus history is remembered through a global
 * stack list. Each client contains an array of Bools of the same size as the
 * global tags array to indicate the tags of a client.  For each client dwm
 * creates a small title window, which is resized whenever the (_NET_)WM_NAME
 * properties are updated or the client is moved/resized.
 *
 * Keys and tagging rules are organized as arrays and defined in the config.h
 * file. These arrays are kept static in event.o and tag.o respectively,
 * because no other part of dwm needs access to them.  The current layout is
 * represented by the lt pointer.
 *
 * To understand everything else, start reading main.c:main().
 */

#include "config.h"
#include <X11/Xlib.h>

/* mask shorthands, used in event.c and client.c */
#define BUTTONMASK		(ButtonPressMask | ButtonReleaseMask)

enum { NetSupported, NetWMName, NetLast };		/* EWMH atoms */
enum { WMProtocols, WMDelete, WMState, WMLast };	/* default atoms */
enum { CurNormal, CurResize, CurMove, CurLast };	/* cursor */
enum { ColBorder, ColFG, ColBG, ColLast };		/* color */

typedef union {
	const char *cmd;
	int i;
} Arg; /* argument type */

typedef struct {
	int ascent;
	int descent;
	int height;
	XFontSet set;
	XFontStruct *xfont;
} Fnt;

typedef struct {
	int x, y, w, h;
	unsigned long norm[ColLast];
	unsigned long sel[ColLast];
	Drawable drawable;
	Fnt font;
	GC gc;
} DC; /* draw context */

typedef struct Client Client;
struct Client {
	char name[256];
	int x, y, w, h;
	int rx, ry, rw, rh; /* revert geometry */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, minay, maxax, maxay;
	long flags; 
	unsigned int border;
	Bool isbanned, isfixed, ismax, versatile;
	Bool *tags;
	Client *next;
	Client *prev;
	Client *snext;
	Window win;
};

typedef struct {
	const char *symbol;
	void (*arrange)(void);
} Layout;

extern const char *tags[];			/* all tags */
extern char stext[256];				/* status text */
extern int screen, sx, sy, sw, sh;		/* screen geometry */
extern int wax, way, wah, waw;			/* windowarea geometry */
extern unsigned int bh, blw;			/* bar height, bar layout label width */
extern unsigned int master, nmaster;		/* master percent, number of master clients */
extern unsigned int ntags, numlockmask;		/* number of tags, dynamic lock mask */
extern void (*handler[LASTEvent])(XEvent *);	/* event handler */
extern Atom wmatom[WMLast], netatom[NetLast];
extern Bool running, selscreen, *seltag;	/* seltag is array of Bool */
extern Client *clients, *sel, *stack;		/* global client list and stack */
extern Cursor cursor[CurLast];
extern DC dc;					/* global draw context */
extern Display *dpy;
extern Layout *lt;
extern Window root, barwin;

/* client.c */
extern void configure(Client *c);		/* send synthetic configure event */
extern void focus(Client *c);			/* focus c, c may be NULL */
extern void focusnext(Arg *arg);		/* focuses next visible client, arg is ignored  */
extern void focusprev(Arg *arg);		/* focuses previous visible client, arg is ignored */
extern void killclient(Arg *arg);		/* kill c nicely */
extern void manage(Window w, XWindowAttributes *wa);	/* manage new client */
extern Client *nexttiled(Client *c);		/* returns tiled successor of c */
extern void resize(Client *c, int x, int y,
		int w, int h, Bool sizehints);	/* resize with given coordinates c*/
extern void updatesizehints(Client *c);		/* update the size hint variables of c */
extern void updatetitle(Client *c);		/* update the name of c */
extern void unmanage(Client *c);		/* destroy c */
extern void zoom(Arg *arg);			/* zooms the focused client to master area, arg is ignored */

/* event.c */
extern void grabkeys(void);			/* grab all keys defined in config.h */

/* main.c */
extern void drawstatus(void);			/* draw the bar */
extern unsigned int textw(const char *text);	/* return the width of text in px*/
extern void quit(Arg *arg);			/* quit dwm nicely */
extern void sendevent(Window w, Atom a, long value);	/* send synthetic event to w */
extern int xerror(Display *dsply, XErrorEvent *ee);	/* dwm's X error handler */

/* screen.c */
extern void compileregs(void);			/* initialize regexps of rules defined in config.h */
extern void incnmaster(Arg *arg);		/* increments nmaster with arg's index value */
extern void initlayouts(void);			/* initialize layout array */
extern Bool isvisible(Client *c);		/* returns True if client is visible */
extern void resizemaster(Arg *arg);		/* resizes the master percent with arg's index value */
extern void restack(void);			/* restores z layers of all clients */
extern void settags(Client *c, Client *trans);	/* sets tags of c */
extern void tag(Arg *arg);			/* tags c with arg's index */
extern void toggleversatile(Arg *arg);		/* toggles focusesd client between versatile/and non-versatile state */
extern void togglelayout(Arg *arg);		/* toggles layout */
extern void toggletag(Arg *arg);		/* toggles c tags with arg's index */
extern void toggleview(Arg *arg);		/* toggles the tag with arg's index (in)visible */
extern void versatile(void);			/* arranges all windows versatile */
extern void view(Arg *arg);			/* views the tag with arg's index */

/* util.c */
extern void *emallocz(unsigned int size);	/* allocates zero-initialized memory, exits on error */
extern void eprint(const char *errstr, ...);	/* prints errstr and exits with 1 */
extern void spawn(Arg *arg);			/* forks a new subprocess with to arg's cmd */

