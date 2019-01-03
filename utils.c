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
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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

GLuint load_texture( const char *name )
{
	GdkPixbuf * pixbuf1;
	GdkPixbuf * pixbuf2;
	GError *error = NULL;
	int width;
	int height;
	int c;
	int bps;
	unsigned char *pxls;
	GLuint texture = 0;
	
	
	pixbuf1 = gdk_pixbuf_new_from_file( name, &error ); //this generate a GLib warning. I don't know why...
	if( !pixbuf1 )
	{
		g_printerr ("Error loading file: %s\n\n", error->message);
		g_clear_error (&error);
		return 0;
	}
	else g_print("file %s successfully opened\n", name );
	
	
	pixbuf2 = gdk_pixbuf_flip( pixbuf1, FALSE );
	
	width = gdk_pixbuf_get_width(pixbuf2);
	height = gdk_pixbuf_get_height(pixbuf2);
	bps = gdk_pixbuf_get_bits_per_sample(pixbuf2);
	c = gdk_pixbuf_get_n_channels(pixbuf2);
	pxls = gdk_pixbuf_get_pixels(pixbuf2);
	
	g_print("-> %d channels\n", c);
	g_print("-> %d bits per sample\n", bps );
	g_print("-> width : %d\n", width );
	g_print("-> height : %d\n\n", height);
	

	glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	
	
	switch( bps )
	{
		case 8 : 	if( c == 4 ) gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGBA,GL_UNSIGNED_BYTE, pxls );
					else if( c == 3 ) gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB,GL_UNSIGNED_BYTE, pxls );
		break;
		default : 	g_print("Error : only 8bps files are supported\n\n");
		break;
	}
	
	g_object_unref(pixbuf1);
	g_object_unref(pixbuf2);
	
	return texture;
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

/*an equivalent of fgets, but that deals with the different newline characters
  returns the last character ridden */
int f_read_line( FILE *stream, int n, char *p )
{
	int i;
	int c;
	
	for( i=0; i<n; i++ )
	{
		c = fgetc(stream);
		if( c == EOF ) break;         //End of File
		else if( c == '\n' ) break;   //Line Feed
		else if ( c== '\r' ) break;   //Cariage Return
		else *p = (char) c;
		p++;
	}
	
	*p = '\0'; //add the 0x00 character
	return c;
}

int f_skip_line( FILE *stream )
{
	int c;
	
	for(;;)
	{
		c = fgetc(stream);
		if( c == EOF ) break;
		else if( c == '\n' ) break;
		else if( c == '\r' ) break;
	}
	
	return c;
}

void vec3d_cross_product( vec3d *v1, vec3d *v2, vec3d *out )
{
	out->x = ( v1->y * v2->z ) - ( v1->z * v2->y );
	out->y = ( v1->z * v2->x ) - ( v1->x * v2->z );
	out->z = ( v1->x * v2->y ) - ( v1->y * v2->x );
}

void vec3d_normalize( vec3d *v )
{
	float a, b;
	
	a = (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
	b = (float) sqrt(a);
	
	if( b == 0.0 ) return;
	
	v->x /= b;
	v->y /= b;
	v->z /= b;
}

void vec3d_sub( vec3d *v1, vec3d *v2, vec3d *out )
{
	out->x = v1->x - v2->x;
	out->y = v1->y - v2->y;
	out->z = v1->z - v2->z;
}

void vec3d_copy( vec3d *v1, vec3d *v2 )
{
	v2->x = v1->x;
	v2->y = v1->y;
	v2->z = v1->z;
}