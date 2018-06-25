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
	
	return model;
}


//counts the number of vertices, normals and texture coordinates and allocates the memory to store them
static void obj_first_read( ObjModel *model, FILE *file )
{
	int i,g;
	int numObjects = 0;
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
                       else model->numVertices++;
            break;
            case 'f' : model->numFaces++;
            break;
			case 'g' : model->numGroups++;
						for(;;)
						{
							character = fgetc( file );
							if( character == '\n' ) break;
							if( character == EOF ) break;
						}
            break;
			case 'o' : numObjects++; 
						for(;;)
						{
							character = fgetc( file );
							if( character == '\n' ) break;
							if( character == EOF ) break;
						}
			break;
            case 'u' : //usemtl
			case 'm' : //mtllib
			case 's' : //s off
            case '#' : for(;;)
						{
							character = fgetc( file );
							if( character == '\n' ) break;
							if( character == EOF ) break;
						}
            break;
        }
    }


    g_print("-> %d vertices\n", model->numVertices );
    g_print("-> %d texture coordinates\n", model->numTexCoor );
    g_print("-> %d normals\n", model->numNorms );
    g_print("-> %d faces\n", model->numFaces );
    g_print("-> %d groups\n", model->numGroups );
	if( numObjects ) g_print("-> %d objects\n", numObjects);
	
	model->vertices = (vec3d*) malloc( model->numVertices * sizeof( vec3d ) );
    model->texCoor = (vec2d*) malloc( model->numTexCoor * sizeof( vec2d ) );
    model->normals = (vec3d*) malloc( model->numNorms * sizeof( vec3d) );
	model->faces = (face*) malloc( model->numFaces * sizeof( face ) );
	
	for( i=0; i < model->numVertices ; i++ ) { model->vertices[i].x = 0.0; model->vertices[i].y = 0.0; model->vertices[i].z = 0.0; }
    for( i=0; i < model->numTexCoor; i++ ) { model->texCoor[i].x = 0.0; model->texCoor[i].y = 0.0; }
	for( i=0; i < model->numNorms; i++ ) { model->normals[i].x = 0.0; model->normals[i].y = 0.0; model->normals[i].z = 0.0; }
	
	size = sizeof(ObjModel) + (model->numVertices * sizeof( vec3d )) + (model->numTexCoor * sizeof( vec2d ))
			+ (model->numNorms * sizeof( vec3d)) + (model->numFaces * sizeof( face ));
	g_print("%d bytes allocated\n", size);
}




//loads obj datas
static void obj_parse( ObjModel *model, FILE *file )
{
    int n=0;
    int v=0;
    int t=0;
    int g=0;
    int f = 0;
	int s = 1; //smooth is ON by default
	char buf[16];
    int character = 0;

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
							fscanf( file, " %lf %lf %lf", &model->normals[n].x, &model->normals[n].y, &model->normals[n].z );
							n++;
						}
						else if( character == 't' ) //TexCoor
						{
							fscanf( file, " %lf %lf", &model->texCoor[t].x, &model->texCoor[t].y );
							t++;
						}
						else //vertex
						{
							fscanf( file, "%lf %lf %lf", &model->vertices[v].x, &model->vertices[v].y, &model->vertices[v].z );
							v++;
						}
            break;

			case 's' : 	fscanf( file, " %s\n", buf ); //smooth on/off
						if( strstr( buf, "off" ) != NULL ) s = 0;
						else s = 1;
			break;	
			
			case 'o' :  //objects are not implemented yet. simply ignoring them should work
			case 'g' : 	//groups are not implemented yet
			case 'u' : 	//usemtl
			case 'm' :	//mtllib
            case '#' :  for(;;) //ignore les commentaires
                        {
                            character = fgetc( file );
                            if( character == '\n' ) break;
                            if( character == '\t' ) break;
							if( character == EOF ) break;
                        }   
            break;
            case 'f' :
						if( model->numNorms == 0 && model->numTexCoor == 0 ) //only vertices
						{

							fscanf( file, " %d %d %d", &model->faces[f].v1, &model->faces[f].v2, &model->faces[f].v3 );
							model->faces[f].v1 -= 1;
							model->faces[f].v2 -= 1;
							model->faces[f].v3 -= 1;
						}
						else if( model->numTexCoor == 0 ) //only vertices and normals
						{
							fscanf( file, " %d//%d %d//%d %d//%d",
									&model->faces[f].v1, &model->faces[f].vn1,
									&model->faces[f].v2, &model->faces[f].vn2,
									&model->faces[f].v3, &model->faces[f].vn3 );

							model->faces[f].v1 -= 1; model->faces[f].vn1 -= 1;
							model->faces[f].v2 -= 1; model->faces[f].vn2 -= 1;
							model->faces[f].v3 -= 1; model->faces[f].vn3 -= 1;
						}
						else if( model->numNorms == 0 ) //only vertices and texture coordinates
						{
							fscanf( file, " %d/%d %d/%d %d/%d",
								&model->faces[f].v1, &model->faces[f].vt1,
								&model->faces[f].v2, &model->faces[f].vt2,
								&model->faces[f].v3, &model->faces[f].vt3 );

								model->faces[f].v1 -= 1; model->faces[f].vt1 -= 1;
								model->faces[f].v2 -= 1; model->faces[f].vt2 -= 1;
								model->faces[f].v3 -= 1; model->faces[f].vt3 -= 1;
						}
						else //vertices texture coordinates and normals
						{
							fscanf( file, " %d/%d/%d %d/%d/%d %d/%d/%d",
								&model->faces[f].v1, &model->faces[f].vt1, &model->faces[f].vn1,
								&model->faces[f].v2, &model->faces[f].vt2, &model->faces[f].vn2,
								&model->faces[f].v3, &model->faces[f].vt3, &model->faces[f].vn3 );

							model->faces[f].v1 -= 1; model->faces[f].vt1 -= 1; model->faces[f].vn1 -= 1;
							model->faces[f].v2 -= 1; model->faces[f].vt2 -= 1; model->faces[f].vn2 -= 1;
							model->faces[f].v3 -= 1; model->faces[f].vt3 -= 1; model->faces[f].vn3 -= 1;
						}
						
						character = fgetc(file);
						character = fgetc(file);
						if( isdigit(character) != 0 ) //square face
						{
							fseek( file, -1, SEEK_CUR );
							model->faces[f].type = 4;
							if( model->numNorms == 0 && model->numTexCoor == 0 )
							{
								fscanf( file, " %d", &model->faces[f].v4 );
								model->faces[f].v4 -= 1;
							}
							else if( model->numTexCoor == 0 )
							{
								fscanf( file, " %d//%d", &model->faces[f].v4, &model->faces[f].vn4 );
								model->faces[f].v4 -= 1; model->faces[f].vn4 -= 1;
							}
							else if( model->numNorms == 0 )
							{
								fscanf( file, " %d/%d", &model->faces[f].v4, &model->faces[f].vt4 );
								model->faces[f].v4 -= 1; model->faces[f].vt4 -= 1;
							}
							else
							{
								fscanf( file, " %d/%d/%d", &model->faces[f].v4, &model->faces[f].vt4, &model->faces[f].vn4 );
								model->faces[f].v4 -= 1; model->faces[f].vt4 -= 1; model->faces[f].vn4 -= 1;
							}
						}
						else //triangular face
						{
							fseek( file, -2, SEEK_CUR );
							model->faces[f].type = 3;
						}
						
						//all the faces following "s 1" or "s off" are afected by s
						model->faces[f].smooth = s;
						f++;
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
	
	
	//we must comme back to C standard locale to make sure fscanf work correctly with float numbers
	locale = setlocale( LC_ALL, NULL );
	savedLocale = strdup( locale );
	setlocale( LC_ALL, "C" );
	
	/* check extension
	if( strstr(name, ".obj") == NULL ) {
		if( strstr(name, ".OBJ") == NULL )
		{
			printf("Error : the filename specified does not contain the .obj extension\n\n");
			return NULL;
		}
	}
	*/

	
    //opening file
    g_print("Opening %s...\n", name );
    file = g_fopen( name, "r" );
    if( !file )
    {
        g_print("Impossible to open %s\n", name );
        return NULL;
    }
	
	
    model = obj_new();
    obj_first_read( model, file );
    obj_parse( model, file );

	
    fclose( file );
    g_print("done\n\n");
	
	
	setlocale( LC_ALL, savedLocale );
	free( savedLocale );

	
    return model;
}

//----------------------------------------------------------------------------------------------------


static void obj_draw_face_vn_vt( ObjModel *model, int f )
{
	if( model->faces[f].smooth ) glShadeModel( GL_SMOOTH );
	else glShadeModel( GL_FLAT );
	
	
	switch( model->faces[f].type )
	{
		case 3 : 	glBegin( GL_TRIANGLES );

					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt1] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn1] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v1] );

					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt2] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn2] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v2] );

					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt3] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn3] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v3] );

					glEnd();
		break;
		case 4 : 	glBegin( GL_QUADS );

					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt1] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn1] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v1] );

					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt2] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn2] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v2] );

					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt3] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn3] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v3] );
					
					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt4] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn4] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v4] );

					glEnd();
		break;
	}
}


static void obj_draw_face( ObjModel *model, int f )
{
	if( model->faces[f].smooth ) glShadeModel( GL_SMOOTH );
	else glShadeModel( GL_FLAT );
	
	switch( model->faces[f].type )
	{
		case 3 : 	glBegin( GL_TRIANGLES );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v1] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v2] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v3] );
					glEnd();
		break;
		case 4 : 	glBegin( GL_QUADS );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v1] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v2] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v3] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v4] );
					glEnd();
		break;
	}
}


void obj_draw( ObjModel *model )
{
    int i;
    int j=0;
	
	if( (model->numNorms == 0) && (model->numTexCoor == 0) )
	{
		for( i=0; i < model->numFaces; i++ ) obj_draw_face( model, i );
	}
    else
	{
		for( i=0; i < model->numFaces; i++ ) obj_draw_face_vn_vt( model, i );
	}
}

void obj_draw_wired( ObjModel *model )
{
	int f;
	
	for( f=0; f < model->numFaces; f++ )
	{
		if( model->faces[f].smooth ) glShadeModel( GL_SMOOTH );
		else glShadeModel( GL_FLAT );
	
		switch( model->faces[f].type )
		{
			case 3 : glBegin( GL_LINE_LOOP );

					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt1] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn1] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v1] );

					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt2] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn2] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v2] );

					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt3] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn3] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v3] );

					glEnd();
			break;
			case 4 : glBegin( GL_LINE_LOOP );

					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt1] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn1] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v1] );

					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt2] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn2] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v2] );

					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt3] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn3] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v3] );
					
					if( model->numTexCoor ) glTexCoord2dv( (double*)&model->texCoor[model->faces[f].vt4] );
					if( model->numNorms ) glNormal3dv( (double*)&model->normals[model->faces[f].vn4] );
					glVertex3dv( (double*)&model->vertices[model->faces[f].v4] );

					glEnd();
			break;
		}
	}
}

//------------------------------------------------------------------------------------------------------------



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


void obj_get_centroid( ObjModel *model, vec3d *centroid )
{
	vec3d v;
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
	vec3d v;
	
	obj_get_centroid( model, &v );
	
	v.x = - v.x;
	v.y = - v.y;
	v.z = - v.z;
	
	obj_translate( model, v.x, v.y, v.z );
}

void obj_replace( ObjModel *model )
{
	vec3d box[2];
	vec3d v;
	
	obj_get_bounding_box( model, box );
	
	v.x = ( box[0].x + box[1].x ) / 2;
	v.y = ( box[0].y + box[1].y ) / 2;
	v.z = ( box[0].z + box[1].z ) / 2;
	
	v.x = - v.x;
	v.y = - v.y;
	v.z = - v.z;
	
	obj_translate( model, v.x, v.y, v.z );
}

void obj_get_bounding_box( ObjModel *model, vec3d box[2] )
{
	int i;
	double xMin, yMin, zMin;
	double xMax, yMax, zMax;

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
	vec3d box[2];
	vec3d v;
	float scale;
	
	obj_get_bounding_box( model, box );
	
	//calcul the measurements
	v.x = box[1].x - box[0].x;
	v.y = box[1].y - box[0].y;
	v.z = box[1].z - box[0].z;
	
	scale = max( v.x, v.y );
	scale = max( scale, v.z );
	scale = 1/scale;
	
	obj_scale( model, scale );
}

//--------------------------------------------------------------------------------

void obj_delete( ObjModel* model )
{
    free( model->vertices );
    if( model->numTexCoor ) free( model->texCoor );
    if( model->numNorms ) free( model->normals );
    free( model->faces );
    free( model );
}
