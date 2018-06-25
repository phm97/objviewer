/*
Copyright 2018 VILAIN Pierre


Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <GL/gl.h>
#include <stdlib.h>

#include "obj.h"


void draw_origin()
{
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	
	glBegin( GL_LINES );
	
	glColor3ub( 255, 0, 0 );
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( 1.0, 0.0, 0.0 );
	
	glColor3ub( 0, 255, 0 );
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( 0.0, 1.0, 0.0 );
	
	glColor3ub( 0, 0, 255 );
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( 0.0, 0.0, 1.0 );
	
	glEnd();
}


void draw_bounding_box( vec3d box[2] )
{
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	
	glBegin( GL_LINE_LOOP );
	glVertex3d( box[0].x, box[0].y, box[0].z );
	glVertex3d( box[1].x, box[0].y, box[0].z );
	glVertex3d( box[1].x, box[1].y, box[0].z );
	glVertex3d( box[0].x, box[1].y, box[0].z );
	glEnd();
	
	glBegin( GL_LINE_LOOP );
	glVertex3d( box[0].x, box[0].y, box[1].z );
	glVertex3d( box[1].x, box[0].y, box[1].z );
	glVertex3d( box[1].x, box[1].y, box[1].z );
	glVertex3d( box[0].x, box[1].y, box[1].z );
	glEnd();
	
	glBegin( GL_LINES );
	glVertex3d( box[0].x, box[0].y, box[0].z );
	glVertex3d( box[0].x, box[0].y, box[1].z );
	glVertex3d( box[1].x, box[0].y, box[0].z );
	glVertex3d( box[1].x, box[0].y, box[1].z );
	glVertex3d( box[1].x, box[1].y, box[0].z );
	glVertex3d( box[1].x, box[1].y, box[1].z );
	glVertex3d( box[0].x, box[1].y, box[0].z );
	glVertex3d( box[0].x, box[1].y, box[1].z );
	glEnd();
}


void bounding_box_centroid( vec3d box[2], vec3d *centroid )
{
	centroid->x = ( box[0].x + box[1].x ) / 2;
	centroid->y = ( box[0].y + box[1].y ) / 2;
	centroid->z = ( box[0].z + box[1].z ) / 2;
}


int take_screenshot( const char *filename )
{
	GdkPixbuf *pixbuf1, *pixbuf2;
	GError *error = NULL;
	GLint viewport[4];
	unsigned char *pxls;
	int width;
	int height;
	
	
	glGetIntegerv(GL_VIEWPORT, viewport);
	width = viewport[2] - viewport[0];
	height = viewport[3] - viewport[1];
	
	pxls = malloc( width * height * 4 );
	glReadPixels( viewport[0],viewport[1],viewport[2],viewport[3],GL_RGBA, GL_UNSIGNED_BYTE, pxls );
	
	pixbuf1 = gdk_pixbuf_new_from_data( pxls, GDK_COLORSPACE_RGB, TRUE, 8, width, height, width * 4, NULL, NULL );
	pixbuf2 = gdk_pixbuf_flip( pixbuf1, FALSE );
	
	g_print("Writing %s\n\n", filename );
	if( gdk_pixbuf_save( pixbuf2, filename, "bmp", &error, NULL ) == FALSE )
	{
		g_printerr ("Error loading file: %s\n\n", error->message);
		g_clear_error (&error);
		g_object_unref(pixbuf2);
		g_object_unref(pixbuf1);
		free(pxls);
		return 1;
	}
	
	g_object_unref(pixbuf2);
	g_object_unref(pixbuf1);
	free(pxls);
	
	return 0;
}

