/*
 * Copyright (C) 2007 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

FILE_LICENCE ( GPL2_OR_LATER );

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ipxe/process.h>
#include <console.h>
#include <ipxe/keys.h>
#include <ipxe/job.h>
#include <ipxe/monojob.h>
#include <ipxe/timer.h>

/** @file
 *
 * Single foreground job
 *
 */

static int monojob_rc;

static void monojob_close ( struct interface *intf, int rc ) {
	monojob_rc = rc;
	intf_restart ( intf, rc );
}

static struct interface_operation monojob_intf_op[] = {
	INTF_OP ( intf_close, struct interface *, monojob_close ),
};

static struct interface_descriptor monojob_intf_desc =
	INTF_DESC_PURE ( monojob_intf_op );

struct interface monojob = INTF_INIT ( monojob_intf_desc );

/**
 * Wait for single foreground job to complete
 *
 * @v string		Job description to display
 * @ret rc		Job final status code
 */
int monojob_wait ( const char *string ) {
	int key;
	int rc;
	unsigned long last_progress_dot;
	unsigned long elapsed;

	printf ( "%s.", string );
	monojob_rc = -EINPROGRESS;
	last_progress_dot = currticks();
	while ( monojob_rc == -EINPROGRESS ) {
		step();
		if ( iskey() ) {
			key = getchar();
			switch ( key ) {
			case CTRL_C:
				monojob_close ( &monojob, -ECANCELED );
				break;
			default:
				break;
			}
		}
		elapsed = ( currticks() - last_progress_dot );
		if ( elapsed >= TICKS_PER_SEC ) {
			printf ( "." );
			last_progress_dot = currticks();
		}
	}
	rc = monojob_rc;

	if ( rc ) {
		printf ( " %s\n", strerror ( rc ) );
	} else {
		printf ( " ok\n" );
	}
	return rc;
}
