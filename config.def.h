/* See LICENSE file for copyright and license details. */

/* appearance */
#define BORDERPX		1
#define FONT			"-*-terminus-medium-r-normal-*-14-*-*-*-*-*-*-*"
#define NORMBORDERCOLOR		"#cccccc"
#define NORMBGCOLOR		"#cccccc"
#define NORMFGCOLOR		"#000000"
#define SELBORDERCOLOR		"#0066ff"
#define SELBGCOLOR		"#0066ff"
#define SELFGCOLOR		"#ffffff"

/* tagging */
const char tags[][MAXTAGLEN] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

Rule rules[] = {
	/* class	instance	title		tags ref	isfloating */
	{ NULL,		NULL,		"Firefox",	tags[8],	False },
	{ NULL,		NULL,		"Gimp",		NULL,		True },
	{ NULL,		NULL,		"MPlayer",	NULL,		True },
	{ NULL,		NULL,		"Acroread",	NULL,		True },
};

/* geometries, s{x,y,w,h} and bh are already initualized here */
/*   func name  bx  by  bw  wx  wy  ww     wh  mx  my       mw  mh     tx  ty     tw  th  mox moy mow moh */
DEFGEOM(single,  0,  0, sw,  0, bh, sw, sh-bh, wx, wy, 0.55*sw, wh, mx+mw, wy, ww-mw, wh,  wx, wy, ww, wh)
DEFGEOM(dual,    0,  0,1280, 0, bh, ww, wh-bh, wx, wy, 1280,800-bh,  1280,  0, ww-mw, sh,  mx, my, mw, mh)

Geom geoms[] = {
	/* symbol	function */
	{ "[]",		single },	/* first entry is default */
	{ "[][]",	dual },
};

/* layout(s) */
#define RESIZEHINTS		True	/* False - respect size hints in tiled resizals */
#define SNAP			32	/* snap pixel */

Layout layouts[] = {
	/* symbol		function	isfloating */
	{ "[]=",		tilev,		False }, /* first entry is default */
	{ "[]|",		tileh,		False },
	{ "><>",		floating,	True },
	{ "[M]",		monocle,	True },
};

/* key definitions */
#define MODKEY			Mod1Mask
Key keys[] = {
	/* modifier			key		function	argument */
	{ MODKEY,			XK_a,		setgeom,	"[][]" },
	{ MODKEY,			XK_d,		setgeom,	"[]" },
	{ MODKEY,			XK_p,		spawn,
		"exec dmenu_run -fn '"FONT"' -nb '"NORMBGCOLOR"' -nf '"NORMFGCOLOR"' -sb '"SELBGCOLOR"' -sf '"SELFGCOLOR"'" },
	{ MODKEY|ShiftMask,		XK_Return,	spawn, "exec uxterm" },
	{ MODKEY,			XK_j,		focusnext,	NULL },
	{ MODKEY,			XK_k,		focusprev,	NULL },
	{ MODKEY,			XK_r,		reapply,	NULL },
	{ MODKEY,			XK_Return,	zoom,		NULL },
	{ MODKEY,			XK_Tab,		viewprevtag,	NULL },
	{ MODKEY,			XK_m,		setlayout,	"[M]" },
	{ MODKEY,			XK_f,		setlayout,	"><>" },
	{ MODKEY,			XK_v,		setlayout,	"[]=" },
	{ MODKEY,			XK_h,		setlayout,	"[]|" },
	{ MODKEY|ShiftMask,		XK_space,	togglefloating,	NULL },
	{ MODKEY|ShiftMask,		XK_c,		killclient,	NULL },
	{ MODKEY,			XK_0,		view,		NULL },
	{ MODKEY,			XK_1,		view,		tags[0] },
	{ MODKEY,			XK_2,		view,		tags[1] },
	{ MODKEY,			XK_3,		view,		tags[2] },
	{ MODKEY,			XK_4,		view,		tags[3] },
	{ MODKEY,			XK_5,		view,		tags[4] },
	{ MODKEY,			XK_6,		view,		tags[5] },
	{ MODKEY,			XK_7,		view,		tags[6] },
	{ MODKEY,			XK_8,		view,		tags[7] },
	{ MODKEY,			XK_9,		view,		tags[8] },
	{ MODKEY|ControlMask,		XK_1,		toggleview,	tags[0] },
	{ MODKEY|ControlMask,		XK_2,		toggleview,	tags[1] },
	{ MODKEY|ControlMask,		XK_3,		toggleview,	tags[2] },
	{ MODKEY|ControlMask,		XK_4,		toggleview,	tags[3] },
	{ MODKEY|ControlMask,		XK_5,		toggleview,	tags[4] },
	{ MODKEY|ControlMask,		XK_6,		toggleview,	tags[5] },
	{ MODKEY|ControlMask,		XK_7,		toggleview,	tags[6] },
	{ MODKEY|ControlMask,		XK_8,		toggleview,	tags[7] },
	{ MODKEY|ControlMask,		XK_9,		toggleview,	tags[8] },
	{ MODKEY|ShiftMask,		XK_0,		tag,		NULL },
	{ MODKEY|ShiftMask,		XK_1,		tag,		tags[0] },
	{ MODKEY|ShiftMask,		XK_2,		tag,		tags[1] },
	{ MODKEY|ShiftMask,		XK_3,		tag,		tags[2] },
	{ MODKEY|ShiftMask,		XK_4,		tag,		tags[3] },
	{ MODKEY|ShiftMask,		XK_5,		tag,		tags[4] },
	{ MODKEY|ShiftMask,		XK_6,		tag,		tags[5] },
	{ MODKEY|ShiftMask,		XK_7,		tag,		tags[6] },
	{ MODKEY|ShiftMask,		XK_8,		tag,		tags[7] },
	{ MODKEY|ShiftMask,		XK_9,		tag,		tags[8] },
	{ MODKEY|ControlMask|ShiftMask,	XK_1,		toggletag,	tags[0] },
	{ MODKEY|ControlMask|ShiftMask,	XK_2,		toggletag,	tags[1] },
	{ MODKEY|ControlMask|ShiftMask,	XK_3,		toggletag,	tags[2] },
	{ MODKEY|ControlMask|ShiftMask,	XK_4,		toggletag,	tags[3] },
	{ MODKEY|ControlMask|ShiftMask,	XK_5,		toggletag,	tags[4] },
	{ MODKEY|ControlMask|ShiftMask,	XK_6,		toggletag,	tags[5] },
	{ MODKEY|ControlMask|ShiftMask,	XK_7,		toggletag,	tags[6] },
	{ MODKEY|ControlMask|ShiftMask,	XK_8,		toggletag,	tags[7] },
	{ MODKEY|ControlMask|ShiftMask,	XK_9,		toggletag,	tags[8] },
	{ MODKEY|ShiftMask,		XK_q,		quit,		NULL },
};
