/*	--*- c -*--
 * Copyright (C) 2008 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 and/or 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef H_UTIL_VSERVER_LIB_INTERNAL_CRYPTO_WRAPPER_H
#define H_UTIL_VSERVER_LIB_INTERNAL_CRYPTO_WRAPPER_H

#define ENSC_CRYPTO_API_NSS		1
#define ENSC_CRYPTO_API_BEECRYPT	2

#if ENSC_CRYPTO_API == ENSC_CRYPTO_API_BEECRYPT
#include "crypto-wrapper-beecrypt.h"
#elif ENSC_CRYPTO_API == ENSC_CRYPTO_API_NSS
#include "crypto-wrapper-nss.h"
#else
#error undefined crypto API
#endif

#endif	/* H_UTIL_VSERVER_LIB_INTERNAL_CRYPTO_WRAPPER_H */
