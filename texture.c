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
	
	
	pixbuf1 = gdk_pixbuf_new_from_file( name, &error );
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