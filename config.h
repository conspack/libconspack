#include "defs.h"
/* $Id: config.h,v 1.3 2002/05/01 23:38:26 radkade1 Exp $ -*- C -*-
 *
 * The Unity Project
 * Copyright (C) 2001  Ryan Pavlik
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

#ifndef __GNUC__
     /* We can use this extension to get rid of some superfluous warnings */
#    define __attribute__ (x)
#endif

#define _GNU_SOURCE

/* Later we will fully enable gettext */
#if 0
#    include <libintl.h>
#    define _(S)  gettext(S)
#    define gettext_noop(S) (S)
#    define N_(S) gettext_noop(S)
#else
#    define _(S)  (S)
#    define N_(S) (S)
#    define textdomain(Domain)
#    define bindtextdomain(Package, Directory)
#endif

/***
 *** Settings
 **/
#define DEF_TCP_PORT      42421
#define DEF_SOCKET_PATH   "/tmp/.s.unity"
#define DEF_PROCESS_LIMIT 0

#endif /* UNITY_CONFIG_H */
