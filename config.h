#include "defs.h"
/* 
 * libconspack
 * Copyright (C) 2012  Ryan Pavlik
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

#ifndef CPK_CONFIG_H
#define CPK_CONFIG_H

#ifndef __GNUC__
     /* We can use this extension to get rid of some superfluous warnings */
#    define __attribute__ (x)
#endif

#define _GNU_SOURCE

#if WORDS_BIGENDIAN
#  define net16(x) (x)
#  define net32(x) (x)
#  define net64(x) (x)
#else
#  define net16(x) __bswap_16(x)
#  define net32(x) __bswap_32(x)
#  define net64(x) __bswap_64(x)
#endif

#endif /* CPK_CONFIG_H */
