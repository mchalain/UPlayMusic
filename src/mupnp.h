/* mupnp.h - Music Player definition
 *
 * Copyright (C) 2014-2016   Marc Chalain
 *
 * This file is part of UPlayMusic.
 *
 * GMediaRender is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GMediaRender is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GMediaRender; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA 02110-1301, USA.
 *
 */

#ifndef _MUPNP_H
#define _MUPNP_H
void * upnp_start(char *friendly_name,
                        char *uuid,
                        char *serial_number,
                        char *ip_address,
                        int listen_port);
void upnp_stop(void *upnp);

#endif /* _UPNP_H */
