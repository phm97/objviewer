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


void obj_enable_verbose( int boolean );
ObjModel* obj_load_from_file( const char* name );
void obj_draw( ObjModel *model );
void obj_draw_wired( ObjModel *model );
void obj_delete( ObjModel* model );
void obj_print_face( ObjModel *model, int f );
void obj_scale( ObjModel *model, float s );
void obj_translate( ObjModel *model, float x, float y, float z );
void obj_get_centroid( ObjModel *model, vec3d *centroid );
void obj_recenter( ObjModel *model );
void obj_replace( ObjModel *model );
void obj_resize( ObjModel *model );
void obj_get_bounding_box( ObjModel *model, vec3d box[2] );

#endif // LOBJ_H_INCLUDED
