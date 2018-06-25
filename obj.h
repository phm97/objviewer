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

#ifndef OBJ_H_INCLUDED
#define OBJ_H_INCLUDED


typedef struct vec3d vec3d;
struct vec3d
{
    double x;
    double y;
    double z;
};

typedef struct vec2d vec2d;
struct vec2d
{
    double x;
    double y;
};

typedef struct face face;
struct face
{
    unsigned int v1, vt1, vn1;
    unsigned int v2, vt2, vn2;
    unsigned int v3, vt3, vn3;
	unsigned int v4, vt4, vn4;
	int smooth;
	int type;
};



typedef struct ObjModel ObjModel;
struct ObjModel
{
    int numVertices;
    vec3d *vertices;

    int numTexCoor;
    vec2d *texCoor;

    int numNorms;
    vec3d *normals;

    int numFaces;
    face *faces;

    int numGroups;
};


/*Loads a Wavefront object file, ignoring groups and objects. 
  Mtl files are not supported yet. 
  The texture must be loaded separately and specified that way :  
	glEnable( GL_TEXTURE_2D );
	GLuint texture = load_texture("mytexture"); 
	glBindTexture( GL_TEXTURE_2D, texture );
	obj_draw( model );
  If the model loaded does not contain any normals, they won't be calculated. 
  You should then disable lighting by doing
	glDisable( GL_LIGHTING ); */
ObjModel* obj_load_from_file( const char* name );

//Renders the model with texture coordinates and normals if present
void obj_draw( ObjModel *model );

//Renders the model using GL_LINE_LOOP
void obj_draw_wired( ObjModel *model );

//Deletes the ObjModel struct and all his contents
void obj_delete( ObjModel* model );

//Scales
void obj_scale( ObjModel *model, float s );

//Translates
void obj_translate( ObjModel *model, float x, float y, float z );

//Calculates the center of gravity of the model
void obj_get_centroid( ObjModel *model, vec3d *centroid );

//Recenters the model so that the center of gravity is in the same place as the origin
void obj_recenter( ObjModel *model );

//Recenters the model so that the center of its bounding box is in the same place as the origin
void obj_replace( ObjModel *model );

//Scales the model to fit in a unit cube
void obj_resize( ObjModel *model );

/*Find the limits of the model and put it in "box"
  box[0] contains Xmin, Ymin and Zmin ;
  box[1] contains Xmax, Ymax and Zmax. */
void obj_get_bounding_box( ObjModel *model, vec3d box[2] );

#endif // LOBJ_H_INCLUDED
