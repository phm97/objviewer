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

#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <math.h>
#include <ctype.h>
#include <GL/gl.h>


#include "obj.h"
#include "utils.h"

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif


static void obj_give_normals_flat( ObjModel *model );
static void obj_give_normals_smooth( ObjModel *model );
void obj_smooth_groups( ObjModel *model );

//--------------------------------------------------------------------------
//fonctions for loading obj 3D models


static ObjModel* obj_new()
{
	ObjModel* model;
	
	model = malloc( sizeof(ObjModel) );
	
    model->numVertices = 0;
    model->numTexCoor = 0;
    model->numNorms = 0;
    model->numFaces = 0;
    model->numGroups = 0;
    model->vertices = NULL;
    model->texCoor = NULL;
    model->normals = NULL;
    model->faces = NULL;
	model->materialList = NULL;
	
	return model;
}

//reads the obj file and import all the mtl files specified by mtllib
static void obj_import_mtl( ObjModel *model, FILE *file, const char *objFileName )
{
	char *dirName, *mtlFileName; 
	int character;
	char buf[256];
	int numMtl;
	int i=0;
	
	dirName = g_path_get_dirname( objFileName );
	
	rewind(file);
	while( character != EOF )
	{
		character = fgetc( file );
		switch( character )
		{
			
			case 'm' : fgets( buf, 7, file );
						if( strcmp( buf, "tllib " ) == 0 )
						{
							character = f_read_line( file, 256, buf );
							mtlFileName = g_build_filename( dirName, buf, NULL );
							mtl_load_from_file( &model->materialList, mtlFileName );
							g_free(mtlFileName);
						}
						else fseek( file, -6, SEEK_CUR );
			break;
			
			case 'o' :
			case 'u' :
			case 'g' :
			case '#' : character = f_skip_line( file ); //comment
            break;
		}
	}
	
	numMtl = mtl_get_list_size( model->materialList );
	g_print("-> %d materials imported\n", numMtl );
	
	g_free( dirName );
}

//count the number of vertices, normals and texture coordinates and allocate the memory to store them
static void obj_first_read( ObjModel *model, FILE *file )
{
	int i;
	int size;
	int character = 0;
	
	
    rewind( file );
    while( character != EOF )
    {
        character = fgetc( file );
        switch( character )
        {
            case 'v' : character = fgetc( file );
                       if( character == 'n' ) model->numNorms++;
                       else if( character == 't' ) model->numTexCoor++;
					   else if( character == 'p' ) character = f_skip_line(file); //vertex parameter. not implemented
                       else model->numVertices++;
            break;
            case 'f' : model->numFaces++;
            break;
			case 'g' : model->numGroups++;
						character = f_skip_line(file);
            break;
			
			
			case 'm' : 
			case 'o' :
            case 'u' : 
			case 's' : //smoothing group
            case '#' : character = f_skip_line(file);
            break;
        }
    }

	
    g_print("-> %d vertices\n", model->numVertices );
    g_print("-> %d texture coordinates\n", model->numTexCoor );
    g_print("-> %d normals\n", model->numNorms );
    g_print("-> %d faces\n", model->numFaces );
    g_print("-> %d groups\n", model->numGroups );
	
	
	model->vertices = (vec3f*) malloc( model->numVertices * sizeof( vec3f ) );
    model->texCoor = (vec2f*) malloc( model->numTexCoor * sizeof( vec2f ) );
    model->normals = (vec3f*) malloc( model->numNorms * sizeof( vec3f) );
	model->faces = (face*) malloc( model->numFaces * sizeof( face ) );
	
	for( i=0; i < model->numVertices ; i++ ) { model->vertices[i].x = 0.0; model->vertices[i].y = 0.0; model->vertices[i].z = 0.0; }
    for( i=0; i < model->numTexCoor; i++ ) { model->texCoor[i].x = 0.0; model->texCoor[i].y = 0.0; }
	for( i=0; i < model->numNorms; i++ ) { model->normals[i].x = 0.0; model->normals[i].y = 0.0; model->normals[i].z = 0.0; }
	
	
	size = sizeof(ObjModel) + (model->numVertices * sizeof( vec3f )) + (model->numTexCoor * sizeof( vec2f ))
			+ (model->numNorms * sizeof( vec3f)) + (model->numFaces * sizeof( face ));
	g_print("%d bytes allocated\n", size);
	
}




//load obj datas
static void obj_parse( ObjModel *model, FILE *file )
{
    int n=0; //current normal
    int v=0; //current vertex
    int t=0; //current texCoord
    int currentFace = 0; 
	short s = 1; //smooth is on by default
	char buf[128];
    int character = 0;
	Material *currentMaterial = NULL;
	
	/* a b c : These variables are used to parse faces. A square face in the file is defined that way :
	   f  a/b/c  a/b/c  a/b/c  a/b/c */
	int a, b, c;
	int i;

    rewind( file );

    g_print("Loading datas...\n");

    while( character != EOF )
    {
        character = fgetc( file );
        switch( character )
        {
			
            case 'v': 	character = fgetc( file );
						if( character == 'n' ) //normal
						{
							fscanf( file, " %f %f %f", &model->normals[n].x, &model->normals[n].y, &model->normals[n].z );
							n++;
						}
						else if( character == 't' ) //TexCoor
						{
							fscanf( file, " %f %f", &model->texCoor[t].x, &model->texCoor[t].y );
							t++;
						}
						else //vertex
						{
							fscanf( file, "%f %f %f", &model->vertices[v].x, &model->vertices[v].y, &model->vertices[v].z );
							v++;
						}
            break;

			case 's' : 	fscanf( file, " %s", buf ); //smoothing group
						if( strcmp( buf, "off" ) == 0 ) s = 0;
						else s = (short) atol( buf );
			break;	
			
			case 'u' : fscanf( file, "semtl %s", buf );
						currentMaterial = mtl_find_with_name( model->materialList, buf );
			break;
			
			case 'o' :
			case 'g' : 	//groups are not implemented yet
			case 'm' :	//mtllib
            case '#' :  character = f_skip_line( file );  
            break;
            case 'f' :  model->faces[currentFace].type = f_line_count_strings( file );
			
						//we must remember this shema :   f  a/b/c a/b/c a/b/c etc...
						if( model->numNorms == 0 && model->numTexCoor == 0 ) //only vertices
						{
							model->faces[currentFace].v = (unsigned int *) malloc( model->faces[currentFace].type * sizeof(unsigned int) ); 
							for( i=0; i<model->faces[currentFace].type; i++ )
							{
								fscanf( file, " %d", &a );
								if( a < 0 ) model->faces[currentFace].v[i] = v + a;		//relatice indices
								else model->faces[currentFace].v[i] =a - 1;			//absolute indices
							}
						}
						else if( model->numTexCoor == 0 ) //only vertices and normals
						{
							model->faces[currentFace].v = (unsigned int *) malloc( model->faces[currentFace].type * sizeof(unsigned int) );
							model->faces[currentFace].vn = (unsigned int *) malloc( model->faces[currentFace].type * sizeof(unsigned int) );
							for( i=0; i<model->faces[currentFace].type; i++ )
							{
								fscanf( file, " %d//%d", &a, &c );
								if( a < 0 )  //relative indices
								{
									model->faces[currentFace].v[i] = v + a;
									model->faces[currentFace].vn[i] = n + c;
								}
								else //absolute indices
								{
									model->faces[currentFace].v[i] = a - 1;
									model->faces[currentFace].vn[i] = c - 1;
								}
							}
						}
						else if( model->numNorms == 0 ) //only vertices and texture coordinates
						{
							model->faces[currentFace].v = (unsigned int *) malloc( model->faces[currentFace].type * sizeof(unsigned int) );
							model->faces[currentFace].vt = (unsigned int *) malloc( model->faces[currentFace].type * sizeof(unsigned int) );
							for( i=0; i<model->faces[currentFace].type; i++ )
							{
								fscanf( file, " %d/%d", &a, &b );
								if( a < 0 )
								{
									model->faces[currentFace].v[i] = v + a;
									model->faces[currentFace].vt[i] = t + b;
								}
								else
								{
									model->faces[currentFace].v[i] = a - 1;
									model->faces[currentFace].vt[i] = b - 1;
								}
							}
						}
						else //vertices texture coordinates and normals
						{
							model->faces[currentFace].v = (unsigned int *) malloc( model->faces[currentFace].type * sizeof(unsigned int) );
							model->faces[currentFace].vt = (unsigned int *) malloc( model->faces[currentFace].type * sizeof(unsigned int) );
							model->faces[currentFace].vn = (unsigned int *) malloc( model->faces[currentFace].type * sizeof(unsigned int) );
							for( i=0; i<model->faces[currentFace].type; i++ )
							{
								fscanf( file, " %d/%d/%d", &a, &b, &c );
								if( a < 0 )
								{
									model->faces[currentFace].v[i] = v + a;
									model->faces[currentFace].vt[i] = t + b;
									model->faces[currentFace].vn[i] = n + c;
								}
								else
								{
									model->faces[currentFace].v[i] = a - 1;
									model->faces[currentFace].vt[i] = b - 1;
									model->faces[currentFace].vn[i] = c - 1;
								}
							}
						}
						
						model->faces[currentFace].smooth = s;
						model->faces[currentFace].mtl = currentMaterial;
						currentFace++;
			break;
			default :
			break;
        }
    }
}



ObjModel* obj_load_from_file( const char* name )
{
    ObjModel* model = NULL;
    FILE *file = NULL;
	char *locale, *savedLocale;
	int i;
	
	
	/* Check extension. You can enable it wether you want
	int l;
	char *extension;
	
	l = strlen( name );
	extension = name + l - 4;
	if( strcmp(extension, ".obj") )
	{
		g_print("Error : the filename specified does not contain the .obj extension\n\n");
		return NULL;
	}
	*/
	
	
	//we must comme back to C standard locale to make sure fscanf work correctly with float numbers
	locale = setlocale( LC_ALL, NULL );
	savedLocale = strdup( locale );
	setlocale( LC_ALL, "C" );

	
    //opening file
    g_print("Opening %s...\n", name );
    file = g_fopen( name, "r" );
    if( !file )
    {
        g_print("Impossible to open %s\n", name );
        return NULL;
    }
	
	
    model = obj_new();
	obj_import_mtl( model, file, name );
    obj_first_read( model, file );
    obj_parse( model, file );
	
    fclose( file );
	
	setlocale( LC_ALL, savedLocale );
	free( savedLocale );
	
	
	if( model->numNorms == 0 ) 
	{
		if( model->numVertices > 30000 ) obj_give_normals_flat( model );
		else
		{
			char hasSmoothingGroups = 0;
			
			for( i=0; i < model->numFaces; i++ )
			{
				if( model->faces[i].smooth > 1 )
				{
					hasSmoothingGroups = 1;
					break;
				}
			}

			if( hasSmoothingGroups ) obj_smooth_groups( model );
			else obj_give_normals_smooth( model );
		}
	}
	

	g_print("done\n\n");
		
    return model;
}

//----------------------------------------------------------------------------------------------------

static void obj_draw_face_vn( ObjModel *model, int f )
{
	int i;
	
	if( model->faces[f].smooth ) glShadeModel( GL_SMOOTH );
	else glShadeModel( GL_FLAT );
	
	mtl_use( model->faces[f].mtl );
	
	switch( model->faces[f].type )
	{
		case 3 : 	glBegin( GL_TRIANGLES );

					glNormal3fv( (float*)&model->normals[model->faces[f].vn[0]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[0]] );

					glNormal3fv( (float*)&model->normals[model->faces[f].vn[1]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[1]] );

					glNormal3fv( (float*)&model->normals[model->faces[f].vn[2]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[2]] );

					glEnd();
		break;
		case 4 : 	glBegin( GL_QUADS );

					glNormal3fv( (float*)&model->normals[model->faces[f].vn[0]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[0]] );

					glNormal3fv( (float*)&model->normals[model->faces[f].vn[1]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[1]] );

					glNormal3fv( (float*)&model->normals[model->faces[f].vn[2]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[2]] );
					
					glNormal3fv( (float*)&model->normals[model->faces[f].vn[3]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[3]] );

					glEnd();
		break;
		default :  glBegin( GL_POLYGON );
					for( i=0; i<model->faces[f].type; i++ )
					{
						glNormal3fv( (float*)&model->normals[model->faces[f].vn[i]] );
						glVertex3fv( (float*)&model->vertices[model->faces[f].v[i]] );
					}
					glEnd();
		break;
	}
}

static void obj_draw_face_vt( ObjModel *model, int f )
{
	int i;
	
	if( model->faces[f].smooth ) glShadeModel( GL_SMOOTH );
	else glShadeModel( GL_FLAT );
	
	mtl_use( model->faces[f].mtl );
	
	switch( model->faces[f].type )
	{
		case 3 : 	glBegin( GL_TRIANGLES );

					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[0]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[0]] );

					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[1]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[1]] );

					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[2]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[2]] );

					glEnd();
		break;
		case 4 : 	glBegin( GL_QUADS );

					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[0]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[0]] );

					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[1]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[1]] );

					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[2]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[2]] );
					
					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[3]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[3]] );

					glEnd();
		break;
		default : glBegin( GL_POLYGON );
					for( i=0; i<model->faces[f].type; i++ )
					{
						glNormal3fv( (float*)&model->normals[model->faces[f].vt[i]] );
						glVertex3fv( (float*)&model->vertices[model->faces[f].v[i]] );
					}
					glEnd();
	}
}


static void obj_draw_face_vn_vt( ObjModel *model, int f )
{
	int i;
	
	if( model->faces[f].smooth ) glShadeModel( GL_SMOOTH );
	else glShadeModel( GL_FLAT );
	
	mtl_use( model->faces[f].mtl );
	
	switch( model->faces[f].type )
	{
		case 3 : 	glBegin( GL_TRIANGLES );

					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[0]] );
					glNormal3fv( (float*)&model->normals[model->faces[f].vn[0]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[0]] );

					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[1]] );
					glNormal3fv( (float*)&model->normals[model->faces[f].vn[1]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[1]] );

					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[2]] );
					glNormal3fv( (float*)&model->normals[model->faces[f].vn[2]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[2]] );

					glEnd();
		break;
		case 4 : 	glBegin( GL_QUADS );

					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[0]] );
					glNormal3fv( (float*)&model->normals[model->faces[f].vn[0]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[0]] );

					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[1]] );
					glNormal3fv( (float*)&model->normals[model->faces[f].vn[1]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[1]] );

					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[2]] );
					glNormal3fv( (float*)&model->normals[model->faces[f].vn[2]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[2]] );
					
					glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[3]] );
					glNormal3fv( (float*)&model->normals[model->faces[f].vn[3]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[3]] );

					glEnd();
		break;
		default :  glBegin( GL_POLYGON );
					for( i=0; i<model->faces[f].type; i++ )
					{
						glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[i]] );
						glNormal3fv( (float*)&model->normals[model->faces[f].vn[i]] );
						glVertex3fv( (float*)&model->vertices[model->faces[f].v[i]] );
					}
					glEnd();
		break;
	}
}


static void obj_draw_face( ObjModel *model, int f )
{
	int i;
	
	if( model->faces[f].smooth ) glShadeModel( GL_SMOOTH );
	else glShadeModel( GL_FLAT );
	
	mtl_use( model->faces[f].mtl );
	
	switch( model->faces[f].type )
	{
		case 3 : 	glBegin( GL_TRIANGLES );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[0]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[1]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[2]] );
					glEnd();
		break;
		case 4 : 	glBegin( GL_QUADS );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[0]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[1]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[2]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[3]] );
					glEnd();
		break;
		default :  glBegin( GL_POLYGON );
					for( i=0; i<model->faces[f].type; i++ )
						glVertex3fv( (float*)&model->vertices[model->faces[f].v[i]] );
					glEnd();
		break;
	}
}


void obj_draw( ObjModel *model )
{
    int i;
	
	if( (model->numNorms == 0) && (model->numTexCoor == 0) ) //only vertices
	{
		for( i=0; i < model->numFaces; i++ ) obj_draw_face( model, i );
	}
	else if( model->numTexCoor == 0 ) //only normals and vertices
	{
		for( i=0; i< model->numFaces; i++ ) obj_draw_face_vn( model, i );
	}
	else if( model->numNorms == 0 ) //only texture coordinates and vertices
	{
		for( i=0; i< model->numFaces; i++ ) obj_draw_face_vt( model, i );
	}
    else //texture coordinates, normals and vertices
	{
		for( i=0; i < model->numFaces; i++ ) obj_draw_face_vn_vt( model, i );
	}
}

void obj_draw_wired( ObjModel *model )
{
	int f, i;
	
	for( f=0; f < model->numFaces; f++ )
	{
		if( model->faces[f].smooth ) glShadeModel( GL_SMOOTH );
		else glShadeModel( GL_FLAT );
		
		mtl_use( model->faces[f].mtl );
	
		switch( model->faces[f].type )
		{
			case 3 : glBegin( GL_LINE_LOOP );

					if( model->numTexCoor ) glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[0]] );
					if( model->numNorms ) glNormal3fv( (float*)&model->normals[model->faces[f].vn[0]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[0]] );

					if( model->numTexCoor ) glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[1]] );
					if( model->numNorms ) glNormal3fv( (float*)&model->normals[model->faces[f].vn[1]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[1]] );

					if( model->numTexCoor ) glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[2]] );
					if( model->numNorms ) glNormal3fv( (float*)&model->normals[model->faces[f].vn[2]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[2]] );

					glEnd();
			break;
			case 4 : glBegin( GL_LINE_LOOP );

					if( model->numTexCoor ) glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[0]] );
					if( model->numNorms ) glNormal3fv( (float*)&model->normals[model->faces[f].vn[0]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[0]] );

					if( model->numTexCoor ) glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[1]] );
					if( model->numNorms ) glNormal3fv( (float*)&model->normals[model->faces[f].vn[1]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[1]] );

					if( model->numTexCoor ) glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[2]] );
					if( model->numNorms ) glNormal3fv( (float*)&model->normals[model->faces[f].vn[2]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[2]] );
					
					if( model->numTexCoor ) glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[3]] );
					if( model->numNorms ) glNormal3fv( (float*)&model->normals[model->faces[f].vn[3]] );
					glVertex3fv( (float*)&model->vertices[model->faces[f].v[3]] );

					glEnd();
			break;
			default : glBegin( GL_LINE_LOOP );
						for( i=0; i<model->faces[f].type; i++ )
						{
							if( model->numTexCoor ) glTexCoord2fv( (float*)&model->texCoor[model->faces[f].vt[i]] );
							if( model->numNorms ) glNormal3fv( (float*)&model->normals[model->faces[f].vn[i]] );
							glVertex3fv( (float*)&model->vertices[model->faces[f].v[i]] );
						}
						glEnd();
			break;
		}
	}
}

//------------------------------------------------------------------------------------------------------------


/*This function overwrite the smoothing group of each face.
  Consequently, once this function has been called, you should not call obj_give_normals_smooth() afterward */
static void obj_give_normals_flat( ObjModel *model )
{
	
	int i, j;
	vec3f v1, v2, norm;
	
	g_print("Calculating flat normals vectors...\n");
	
	model->normals = malloc( model->numFaces * sizeof(vec3f) );
	if( model->normals ==  NULL ) return;
	model->numNorms = model->numFaces;
	for( i=0; i < model->numFaces; i++ )
		model->faces[i].vn = (unsigned int*) malloc( model->faces[i].type * sizeof( unsigned int ) );
	
	for( i=0; i < model->numFaces; i++ )
	{
		vec3f_sub( &model->vertices[model->faces[i].v[0]], &model->vertices[model->faces[i].v[1]], &v1 );
		vec3f_sub( &model->vertices[model->faces[i].v[1]], &model->vertices[model->faces[i].v[2]], &v2 );
		vec3f_cross_product( &v1, &v2, &norm );
		vec3f_normalize( &norm );
		vec3f_copy( &norm, &model->normals[i] );
		
		for( j=0; j < model->faces[i].type; j++ ) model->faces[i].vn[j] = i;
		model->faces[i].smooth = 0;
	}
}


static void obj_give_normals_smooth( ObjModel *model )
{
	int i, j, k;
	vec3f v1, v2, norm;
	vec3f *flatNorms;
	vec3f *smoothNorms;
	
	
	g_print("Calculating flat and smooth normal vectors...\n");
	
	
	//Calculate flat normals and store them in flatNorms array. One face -> one normal
	flatNorms = malloc( model->numFaces * sizeof(vec3f) );
	for( i=0; i < model->numFaces; i++ )
	{
		vec3f_sub( &model->vertices[model->faces[i].v[0]], &model->vertices[model->faces[i].v[1]], &v1 );
		vec3f_sub( &model->vertices[model->faces[i].v[1]], &model->vertices[model->faces[i].v[2]], &v2 );
		vec3f_cross_product( &v1, &v2, &norm );
		vec3f_normalize( &norm );
		vec3f_copy( &norm, &flatNorms[i] );
	}
	

	/*calculate smooth normals and store them in the smoothNorms array. One vertex -> one normal.
	  To do so, take each vertex and look for all the faces that it belongs to.
	  Then take all the corresponding flat normals and calculate the average normal. 
	  Then add the result to the smooth norm array */
	smoothNorms = malloc( model->numVertices * sizeof(vec3f) );
	for( i=0; i < model->numVertices; i++ )
	{	
		vec3f_zero( &norm );
		for( j=0; j < model->numFaces; j++ )
		{
			for( k=0; k < model->faces[j].type; k++ )
			{
				if( model->faces[j].v[k] == i ) vec3f_add( &norm, &flatNorms[j], &norm );
			}
		}
		vec3f_normalize( &norm );
		vec3f_copy( &norm, &smoothNorms[i] ); 
	}
	
	
	//then count the number of normals that we will put in the model
	for( i=0, k=0; i < model->numFaces; i++ )
	{
		if( model->faces[i].smooth ) k += model->faces[i].type;
		else k++; //one normal per face
	}
	
	
	//allocate the memory in the model to store the normals
	model->numNorms = k;
	model->normals = malloc( k * sizeof(vec3f) );
	for( i=0; i < model->numFaces; i++ ) model->faces[i].vn = (unsigned int*) malloc( model->faces[i].type * sizeof( unsigned int ) );
	
	
	//then, copy the normals in the model, and attribute each normal to a face
	for( i=0, k=0; i < model->numFaces; i++ )
	{
		if( model->faces[i].smooth )
		{
			for( j=0; j < model->faces[i].type; j++ )
			{
				vec3f_copy( &smoothNorms[model->faces[i].v[j]], &model->normals[k] );
				model->faces[i].vn[j] = k;
				k++;
			}
		}
		else
		{
			vec3f_copy( &flatNorms[i], &model->normals[k] );
			for( j=0; j < model->faces[i].type; j++ ) model->faces[i].vn[j] = k;
			k++;
		}
	}
	
	
	free( smoothNorms );
	free( flatNorms );
}


void obj_smooth_groups( ObjModel *model )
{

	int i, j, k, l, m;
	vec3f v1, v2, norm;
	vec3f *flatNorms;
	
	
	g_print("Calculating normal vectors using smoothing groups...\n");
	
	
	//First, Calculate normals flat. One face -> one normal
	flatNorms = malloc( model->numFaces * sizeof(vec3f) );
	for( i=0; i < model->numFaces; i++ )
	{
		vec3f_sub( &model->vertices[model->faces[i].v[0]], &model->vertices[model->faces[i].v[1]], &v1 );
		vec3f_sub( &model->vertices[model->faces[i].v[1]], &model->vertices[model->faces[i].v[2]], &v2 );
		vec3f_cross_product( &v1, &v2, &norm );
		vec3f_normalize( &norm );
		vec3f_copy( &norm, &flatNorms[i] );
	}
	
	
	//then count the number of normals that we will put in the model
	for( i=0, m=0; i < model->numFaces; i++ )
	{
		if( model->faces[i].smooth ) m += model->faces[i].type;
		else m++; //one normal
	}
	
	//allocate the memory in the model to store the normals
	model->normals = malloc( m * sizeof(vec3f) );
	model->numNorms = m;
	for( i=0; i < model->numFaces; i++ ) model->faces[i].vn = (unsigned int*) malloc( model->faces[i].type * sizeof( unsigned int ) );
	
	//calculate the normals
	for( i=0, m=0; i < model->numFaces; i++ )
	{
		if( model->faces[i].smooth )
		{
			for( j=0; j < model->faces[i].type; j++)
			{
				vec3f_zero( &norm);
				for( k=0; k < model->numFaces; k++ )
				{
					if( model->faces[k].smooth != model->faces[i].smooth ) continue;
					
					for( l=0; l < model->faces[k].type; l++ )
						if( model->faces[k].v[l] == model->faces[i].v[j] ) vec3f_add( &norm, &flatNorms[k], &norm );
				}
				vec3f_normalize( &norm );
				vec3f_copy( &norm, &model->normals[m] );
				model->faces[i].vn[j] = m;
				m++;
			}
		}
		else
		{
			vec3f_copy( &flatNorms[i], &model->normals[m] );
			for( j=0; j < model->faces[i].type; j++) model->faces[i].vn[j] = m;
			m++;
		}
	}
	
	free( flatNorms );
}




void obj_scale( ObjModel *model, float s )
{
	int i;
	
	for( i=0; i < model->numVertices; i++ )
	{
		model->vertices[i].x = model->vertices[i].x * s;
		model->vertices[i].y = model->vertices[i].y * s;
		model->vertices[i].z = model->vertices[i].z * s;
	}
}

void obj_translate( ObjModel *model, float x, float y, float z )
{
	int i;
	
	for( i=0; i < model->numVertices; i++ )
	{
		model->vertices[i].x += x;
		model->vertices[i].y += y;
		model->vertices[i].z += z;
	}
}


void obj_get_centroid( ObjModel *model, vec3f *centroid )
{
	vec3f v;
	int i;
	
	v.x = v.y = v.z = 0.0;
	
	for( i=0; i < model->numVertices; i++ )
	{
		v.x += model->vertices[i].x;
		v.y += model->vertices[i].y;
		v.z += model->vertices[i].z;
	}
	
	v.x = v.x / (float)model->numVertices;
	v.y = v.y / (float)model->numVertices;
	v.z = v.z / (float)model->numVertices;
	
	centroid->x = v.x;
	centroid->y = v.y;
	centroid->z = v.z;
}


void obj_recenter( ObjModel *model )
{
	vec3f v;
	
	obj_get_centroid( model, &v );
	
	v.x = - v.x;
	v.y = - v.y;
	v.z = - v.z;
	
	obj_translate( model, v.x, v.y, v.z );
}

void obj_replace( ObjModel *model )
{
	vec3f box[2];
	vec3f v;
	
	obj_get_bounding_box( model, box );
	
	v.x = ( box[0].x + box[1].x ) / 2;
	v.y = ( box[0].y + box[1].y ) / 2;
	v.z = ( box[0].z + box[1].z ) / 2;
	
	v.x = - v.x;
	v.y = - v.y;
	v.z = - v.z;
	
	obj_translate( model, v.x, v.y, v.z );
}

void obj_get_bounding_box( ObjModel *model, vec3f box[2] )
{
	int i;
	float xMin, yMin, zMin;
	float xMax, yMax, zMax;

	xMax = xMin = model->vertices[0].x;
	yMax = yMin = model->vertices[0].y;
	zMax = zMin = model->vertices[0].z;
	
	for( i=0; i < model->numVertices; i++ )
	{
		if( model->vertices[i].x > xMax ) xMax = model->vertices[i].x;
		if( model->vertices[i].y > yMax ) yMax = model->vertices[i].y;
		if( model->vertices[i].z > zMax ) zMax = model->vertices[i].z;
		if( model->vertices[i].x < xMin ) xMin = model->vertices[i].x;
		if( model->vertices[i].y < yMin ) yMin = model->vertices[i].y;
		if( model->vertices[i].z < zMin ) zMin = model->vertices[i].z;
	}
	
	box[0].x = xMin;
	box[0].y = yMin;
	box[0].z = zMin;
	box[1].x = xMax;
	box[1].y = yMax;
	box[1].z = zMax;
}


void obj_resize( ObjModel *model )
{
	vec3f box[2];
	vec3f v;
	float scale;
	
	obj_get_bounding_box( model, box );
	
	//calcul the measurements
	//v.x = box[1].x - box[0].x;
	//v.y = box[1].y - box[0].y;
	//v.z = box[1].z - box[0].z;
	vec3f_sub( &box[1], &box[0], &v );
	
	scale = max( v.x, v.y );
	scale = max( scale, v.z );
	scale = 1.0/scale;
	
	obj_scale( model, scale );
}

//--------------------------------------------------------------------------------

void obj_delete( ObjModel* model )
{
	int i;
	
	mtl_delete_all( model->materialList );
	
    free( model->vertices );
    if( model->numTexCoor ) free( model->texCoor );
    if( model->numNorms ) free( model->normals );
	
	if( model->numNorms == 0 && model->numTexCoor == 0 ) //only vertices
	{
		for( i=0; i < model->numFaces; i++ ) free( model->faces[i].v );
	}
	else if( model->numTexCoor == 0 ) //only vertices and normals
	{
		for( i=0; i < model->numFaces; i++ )
		{
			free( model->faces[i].v );
			free( model->faces[i].vn );
		}
	}
	else if( model->numNorms == 0 ) //only vertices and texture coordinates
	{
		for( i=0; i < model->numFaces; i++ )
		{
			free( model->faces[i].v );
			free( model->faces[i].vt );
		}
	}
	else
	{
		for( i=0; i < model->numFaces; i++ ) //vertices, normals and texture coordinates
		{
			free( model->faces[i].v );
			free( model->faces[i].vt );
			free( model->faces[i].vn );
		}
	}
    free( model->faces );
	
    free( model );
}
