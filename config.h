/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#define FONT		"-*-terminus-medium-*-*-*-14-*-*-*-*-*-iso10646-*"
#define BGCOLOR		"#000000"
#define FGCOLOR		"#ffaa00"
#define BORDERCOLOR	"#000000"
#define STATUSCMD	"echo -n `date` `uptime | sed 's/.*://; s/,//g'`" \
					" `acpi | awk '{print $4}' | sed 's/,//'`"
