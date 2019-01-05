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
#include "mtl.h"

#ifndef OBJ_H_INCLUDED
#define OBJ_H_INCLUDED



typedef struct face face;
struct face
{
    unsigned int *v;
	unsigned int *vt;
	unsigned int *vn;
	unsigned short smooth;		//smoothing group
	unsigned short type;		//the number of vertices
	Material *mtl;
};



typedef struct ObjModel ObjModel;
struct ObjModel
{
	Material *materialList;
	
    int numVertices;
    vec3f *vertices;

    int numTexCoor;
    vec2f *texCoor;

    int numNorms;
    vec3f *normals;

    int numFaces;
    face *faces;

    int numGroups;
};


ObjModel* obj_load_from_file( const char* name );
void obj_draw( ObjModel *model );
void obj_draw_wired( ObjModel *model );
void obj_delete( ObjModel* model );
void obj_scale( ObjModel *model, float s );
void obj_translate( ObjModel *model, float x, float y, float z );
void obj_get_centroid( ObjModel *model, vec3f *centroid );
void obj_recenter( ObjModel *model );
void obj_replace( ObjModel *model );
void obj_resize( ObjModel *model );
void obj_get_bounding_box( ObjModel *model, vec3f box[2] );

#endif // LOBJ_H_INCLUDED
