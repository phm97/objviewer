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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <GL/gl.h>

#include "mtl.h"
#include "utils.h"


Material* mtl_new( Material **materialList, char *name )
{
	Material *mtl = (Material*) malloc( sizeof(Material) );
	
	
	strcpy( mtl->name, name );
	mtl->ambiant[0] = 0.2f; mtl->ambiant[1] = 0.2f; mtl->ambiant[2] = 0.2f; mtl->ambiant[3] = 1.0f;
	mtl->diffuse[0] = 0.8f; mtl->diffuse[1] = 0.8f; mtl->diffuse[2] = 0.8f; mtl->diffuse[3] = 1.0f;
	mtl->specular[0] = 0.0f; mtl->specular[1] = 0.0f; mtl->specular[2] = 0.0f; mtl->specular[3] = 1.0f;
	mtl->shininess = 0.0;
	mtl->texture = 0;
	
	mtl->next = *materialList;
	*materialList = mtl;
	
	return *materialList;
}

int mtl_load_from_file( Material **materialList, const char *filename )
{
	FILE *file;
	char buf[512];
	int character = 0;
	Material* currentMaterial;
	float d=0;
	char *dirName, *textureFileName;
	
	dirName = g_path_get_dirname( filename );
	
	g_print("Opening file %s...\n", filename );
	file = fopen( filename, "r" );
	if( !file )
	{
		g_print("Error : could not load file : %s\n", filename );
		return 1;
	}

	while( character != EOF )
	{
		character = fgetc(file);
		switch( character )
		{
		
			
			case 'n' : fscanf( file, "ewmtl %s", buf );
						currentMaterial = mtl_new( materialList, buf );
			break;
			case 'K' : character = fgetc( file );
						if( character == 'a' ) fscanf( file, " %f %f %f", &currentMaterial->ambiant[0], &currentMaterial->ambiant[1], &currentMaterial->ambiant[2] );
						else if( character == 'd' ) fscanf( file, " %f %f %f", &currentMaterial->diffuse[0], &currentMaterial->diffuse[1], &currentMaterial->diffuse[2] );
						else if( character == 's' ) fscanf( file, " %f %f %f", &currentMaterial->specular[0], &currentMaterial->specular[1], &currentMaterial->specular[2] );
						//else if( character == 'e' ) emissive light, not implemented yet
			break;
			case 'N' : character = fgetc( file );
						if( character == 's' ) 
						{
							fscanf( file, " %f", &currentMaterial->shininess );
						}
			break;
			case 'd' : fscanf( file, " %f", &d );
						currentMaterial->ambiant[3] = d;
						currentMaterial->diffuse[3] = d;
						currentMaterial->specular[3] = d;
			break;
			case 'm' : fgets( buf, 7, file );
						if( strcmp( buf, "ap_Kd " ) == 0 )
						{
							character = f_read_line( file, 512, buf );
							textureFileName = g_build_filename( dirName, buf, NULL );
							currentMaterial->texture = load_texture( textureFileName );
							g_free( textureFileName );
						}
						else fseek( file, -6, SEEK_CUR );
			break;
			case 'i' :
			case '#' : character = f_skip_line( file );
			break;
		}
	}

	g_free( dirName );
	
	fclose(file);
	return 0;
}

int mtl_get_list_size( Material* materialList )
{
	Material* mtl;
	int i=0;
	
	for( mtl=materialList; mtl!= NULL; mtl=mtl->next ) i++;
	
	return i;
}

Material* mtl_find_with_name( Material* materialList, char *name )
{
	Material* mtl;
	
	for( mtl=materialList; mtl!=NULL; mtl=mtl->next )
	{
		if( strcmp( name, mtl->name ) == 0 ) break;
	}
	
	return mtl;
}

void mtl_use( Material* mtl )
{	
	if( mtl == NULL ) return;
	
	glColor4fv( mtl->diffuse );
	
	glMaterialfv( GL_FRONT, GL_AMBIENT, mtl->ambiant );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, mtl->diffuse );
	glMaterialfv( GL_FRONT, GL_SPECULAR, mtl->specular );
	glMaterialf( GL_FRONT, GL_SHININESS, mtl->shininess );
	
	if( mtl->texture )
	{
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, mtl->texture );
	}
}

void mtl_delete_all( Material *mtl1 )
{
	Material *mtl2 = mtl1;
	
	while( mtl2 != NULL )
	{
		mtl2 = mtl1->next;
		free(mtl1);
		mtl1 = mtl2;
	}
}