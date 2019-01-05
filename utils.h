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

#include "types.h"

#ifndef DEF_UTILS
#define DEF_UTILS

//x red ; y green ; z blue;
void draw_origin();

//draws bounding box with lines
void draw_bounding_box( vec3f box[2] );

//Calculates the center of the bounding box 
void bounding_box_centroid( vec3f box[2], vec3f *centroid );

//Loads a texture with GdkPixbuf
unsigned int load_texture( const char *nom );

//reads the pixels of the drawing area and store them in a .bmp file
int take_screenshot( const char *filename );

/*an equivalent of fgets, but that deals with CRLF character.
  Returns the last character ridden. If the end of file is reach, it returns EOF */
int f_read_line( FILE *stream, int n, char *p );

//go to the next line and return the last character ridden. If end of file, it returns EOF
int f_skip_line( FILE *stream );

unsigned short f_line_count_strings( FILE *stream );

//Operations on vectors
short vec3f_cmp( vec3f v1, vec3f v2 );
void vec3f_cross_product( vec3f *v1, vec3f *v2, vec3f *out );
void vec3f_normalize( vec3f *v );
void vec3f_sub( vec3f *v1, vec3f *v2, vec3f *out );
void vec3f_add( vec3f *v1, vec3f *v2, vec3f *out );
void vec3f_scale( vec3f *v, double s, vec3f *out );
void vec3f_copy( vec3f *v1, vec3f *v2 );
void vec3f_zero( vec3f *v );

#endif