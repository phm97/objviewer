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

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "gtkglwidget.h"
#include "trackball.h"
#include "obj.h"
#include "utils.h"

#define GTK_GL_WIDGET_DEFAULT_SIZE 300

static void gtk_gl_widget_draw( GtkGlWidget *glWidget );

struct _GtkGlWidgetPrivate
{
	ObjModel *model;
	gboolean hasModel;
	
	GLuint texture;
	gboolean hasTexture;
	
	gboolean showBoundingBox;
	gboolean showOrigin;
	gboolean wiredMode;
	gboolean centerModel;
	gboolean showCentroid;
	gboolean enableLighting;
	
	float xBegin;
	float yBegin;
	float quatRot[4]; // the current rotation of the model is stored in a quaternion
	float scale;
	
	float x; //model original position
	float y;
	float z;
	
	float cx; //center of gravity of the model
	float cy;
	float cz;
	
	float bgColorRed; //Background Color
	float bgColorGreen;
	float bgColorBlue;
};

void gtk_gl_widget_enable_lighting( GtkGlWidget *glWidget, gboolean b )
{
	g_return_if_fail(glWidget != NULL);
	g_return_if_fail(GTK_IS_GL_WIDGET(glWidget));
	
	glWidget->priv->enableLighting = b;
	gtk_gl_widget_draw( glWidget );
}

void gtk_gl_widget_center_model( GtkGlWidget *glWidget, gboolean b )
{
	g_return_if_fail(glWidget != NULL);
	g_return_if_fail(GTK_IS_GL_WIDGET(glWidget));
	
	glWidget->priv->centerModel = b;
	gtk_gl_widget_draw( glWidget );
}

void gtk_gl_widget_show_centroid( GtkGlWidget *glWidget, gboolean b )
{
	g_return_if_fail(glWidget != NULL);
	g_return_if_fail(GTK_IS_GL_WIDGET(glWidget));
	
	glWidget->priv->showCentroid = b;
	gtk_gl_widget_draw( glWidget );
}

void gtk_gl_widget_set_wired_mode( GtkGlWidget *glWidget, gboolean b )
{
	g_return_if_fail(glWidget != NULL);
	g_return_if_fail(GTK_IS_GL_WIDGET(glWidget));
	
	glWidget->priv->wiredMode = b;
	gtk_gl_widget_draw( glWidget );
}

ObjModel *gtk_gl_widget_get_model( GtkGlWidget *glWidget )
{	
	g_return_if_fail(glWidget != NULL);
	g_return_if_fail(GTK_IS_GL_WIDGET(glWidget));

	if( glWidget->priv->hasModel)
		return glWidget->priv->model;
	else return NULL;
}

gboolean gtk_gl_widget_has_model( GtkGlWidget *glWidget )
{
	g_return_if_fail(glWidget != NULL);
	g_return_if_fail(GTK_IS_GL_WIDGET(glWidget));
	
	return glWidget->priv->hasModel;
}

void gtk_gl_widget_show_bounding_box( GtkGlWidget *glWidget, gboolean b )
{
	g_return_if_fail(glWidget != NULL);
	g_return_if_fail (GTK_IS_GL_WIDGET(glWidget));
	
	glWidget->priv->showBoundingBox = b;
	gtk_gl_widget_draw( glWidget );
}

void gtk_gl_widget_show_origin( GtkGlWidget *glWidget, gboolean b )
{
	g_return_if_fail(glWidget != NULL);
	g_return_if_fail (GTK_IS_GL_WIDGET(glWidget));
	
	glWidget->priv->showOrigin = b;
	gtk_gl_widget_draw( glWidget );
}

void gtk_gl_widget_set_background_color( GtkGlWidget *glWidget, GdkColor color )
{
	g_return_if_fail(glWidget != NULL);
	g_return_if_fail (GTK_IS_GL_WIDGET(glWidget));
	
	glWidget->priv->bgColorRed = (float)color.red/65535.0;
	glWidget->priv->bgColorGreen = (float)color.green/65535.0;
	glWidget->priv->bgColorBlue = (float)color.blue/65535.0;
	
	gtk_gl_widget_draw( glWidget );
}

void gtk_gl_widget_get_background_color( GtkGlWidget *glWidget, GdkColor *color )
{
	g_return_if_fail(glWidget != NULL);
	g_return_if_fail(color != NULL);
	g_return_if_fail (GTK_IS_GL_WIDGET(glWidget));
	
	color->pixel = 0;
	color->red = (guint16)(glWidget->priv->bgColorRed * 65535);
	color->green = (guint16)(glWidget->priv->bgColorGreen * 65535);
	color->blue = (guint16)(glWidget->priv->bgColorBlue * 65535);
}


static void gtk_gl_widget_init_gl( GtkGlWidget *glWidget )
{	
	g_return_if_fail(glWidget != NULL);
	g_return_if_fail (GTK_IS_GL_WIDGET(glWidget));

	GdkGLContext *glcontext = gtk_widget_get_gl_context( GTK_WIDGET(glWidget) );
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable( GTK_WIDGET(glWidget) );
	
	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) return;
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	g_print ("Graphic card   : %s\n", (char *) glGetString (GL_RENDERER));
	g_print ("OpenGL version : %s\n", (char *) glGetString (GL_VERSION));
	g_print ("OpenGL vendor  : %s\n\n", (char *) glGetString (GL_VENDOR));
	
	gdk_gl_drawable_gl_end (gldrawable);
}

static void gtk_gl_widget_draw( GtkGlWidget *glWidget )
{
	GdkGLContext *glcontext;
	GdkGLDrawable *gldrawable;
	float matrix[4][4];
	
	g_return_if_fail(glWidget != NULL);
	g_return_if_fail (GTK_IS_GL_WIDGET(glWidget));
	
	glcontext = gtk_widget_get_gl_context( GTK_WIDGET(glWidget) );
	gldrawable = gtk_widget_get_gl_drawable( GTK_WIDGET(glWidget) );
	
	//OpenGL Begin
	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) return;
	
	glClearColor( glWidget->priv->bgColorRed, glWidget->priv->bgColorGreen, glWidget->priv->bgColorBlue, 1.0 ); 
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

	glDisable(GL_LIGHTING);
	
	build_rotmatrix( matrix, glWidget->priv->quatRot );
	if( glWidget->priv->hasModel )
	{
		gluLookAt(0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.); 
		
		if( glWidget->priv->showOrigin )
		{
			glPushMatrix();
			draw_origin();
			glPopMatrix();
		}
		
		if( glWidget->priv->showBoundingBox )
		{
			vec3d boundingBox[2];
			
			glPushMatrix();
			glMultMatrixf(&matrix[0][0]);
			glScalef( glWidget->priv->scale, glWidget->priv->scale, glWidget->priv->scale );
			if( glWidget->priv->centerModel == FALSE ) glTranslatef( glWidget->priv->x, glWidget->priv->y, glWidget->priv->z );
			obj_get_bounding_box( glWidget->priv->model, boundingBox );
			glColor3ub( 255, 0, 0 );
			draw_bounding_box( boundingBox );
			glPopMatrix();
		}
		
		if( glWidget->priv->enableLighting ) glEnable(GL_LIGHTING);
		
		if( glWidget->priv->showCentroid )  //draw a blue sphere at the location of the center of gravity
		{
			float blueCentroid[] = { 0.0, 0.0, 1.0, 1.0 };
			
			glMaterialfv( GL_FRONT, GL_DIFFUSE, blueCentroid );
			glPushMatrix();
			glMultMatrixf(&matrix[0][0]);
			glScalef( glWidget->priv->scale, glWidget->priv->scale, glWidget->priv->scale );
			if( glWidget->priv->centerModel == FALSE ) glTranslatef( glWidget->priv->x, glWidget->priv->y, glWidget->priv->z );
			glTranslatef( glWidget->priv->cx, glWidget->priv->cy, glWidget->priv->cz );
			glColor3ub( 0, 0, 255 );
			gdk_gl_draw_sphere( TRUE, 0.055/glWidget->priv->scale, 16, 16 );
			glPopMatrix();
		}
		
		if( glWidget->priv->hasTexture )
		{
			glEnable( GL_TEXTURE_2D );
			glBindTexture( GL_TEXTURE_2D, glWidget->priv->texture );
		}
		else glDisable( GL_TEXTURE_2D );
		
		float modelMaterialDefaultDiffuse[] = { 0.8, 0.8, 0.8, 1.0 };
		float modelMaterialDefaultAmbient[] = { 0.2, 0.2, 0.2, 1.0 };
		
		glMaterialfv( GL_FRONT, GL_DIFFUSE, modelMaterialDefaultDiffuse );
		glMaterialfv( GL_FRONT, GL_AMBIENT, modelMaterialDefaultAmbient );
		glPushMatrix();
		glMultMatrixf(&matrix[0][0]);
		glScalef( glWidget->priv->scale, glWidget->priv->scale, glWidget->priv->scale );
		if( glWidget->priv->centerModel == FALSE ) glTranslatef( glWidget->priv->x, glWidget->priv->y, glWidget->priv->z );
		glColor3ub( 255, 255, 255 );
		if( glWidget->priv->wiredMode ) obj_draw_wired( glWidget->priv->model );
		else obj_draw( glWidget->priv->model );
		glPopMatrix();
	}
	else
	{
		

		float teapotMaterial[] = { 1.0, 0.0, 0.0, 1.0 };
		
		gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.); 
		
		if( glWidget->priv->enableLighting ) glEnable( GL_LIGHTING );
		glMaterialfv( GL_FRONT, GL_DIFFUSE, teapotMaterial );
		glPushMatrix();
		glMultMatrixf(&matrix[0][0]);
		glScalef( glWidget->priv->scale, glWidget->priv->scale, glWidget->priv->scale );
		glColor3ub(255,0,0);
		if( glWidget->priv->wiredMode ) gdk_gl_draw_teapot( FALSE, 1.8 );
		else gdk_gl_draw_teapot( TRUE, 1.8 );
		glPopMatrix();
	}
	
	if (gdk_gl_drawable_is_double_buffered (gldrawable))
		gdk_gl_drawable_swap_buffers (gldrawable);
	else
		glFlush ();
	
	//OpenGL end
	gdk_gl_drawable_gl_end (gldrawable);
}


static gboolean gtk_gl_widget_button_press_event (GtkWidget *widget, GdkEventButton *event )
{
	GtkGlWidget *glWidget = GTK_GL_WIDGET( widget );
	
	if( event->button == 1 )
	{
		glWidget->priv->xBegin = event->x;
		glWidget->priv->yBegin = event->y;
	}
	
	return FALSE; //I don't know wether it must return TRUE or FALSE
}

//mouse wheel handler function
static gboolean gtk_gl_widget_scroll_event (GtkWidget *widget, GdkEventScroll *event )
{
	GtkGlWidget *glWidget = GTK_GL_WIDGET(widget);
	
	switch( event->direction )
	{
		case GDK_SCROLL_UP : glWidget->priv->scale += 0.1;
		break;
		case GDK_SCROLL_DOWN : glWidget->priv->scale -= 0.1;
		break;
		default :
		break;
	}
	
	gtk_gl_widget_draw( glWidget );
	
	return TRUE;
}

//mouse motion handler
static gboolean gtk_gl_widget_motion_notify_event (GtkWidget *widget, GdkEventMotion *event )
{
	GtkGlWidget *glWidget = GTK_GL_WIDGET(widget);
	
	float w = (float)widget->allocation.width;
	float h = (float)widget->allocation.height;
	float x = (float)event->x;
	float y = (float)event->y;
	float quat[4];
	
	
	/* Rotation. */
	if (event->state & GDK_BUTTON1_MASK)
	{
		//thank you Silicon Graphics !
		trackball (quat, (2.0 * glWidget->priv->xBegin - w) / w, (h - 2.0 * glWidget->priv->yBegin) / h, (2.0 * x - w) / w, (h - 2.0 * y) / h);
		add_quats( quat, glWidget->priv->quatRot, glWidget->priv->quatRot );

		gtk_gl_widget_draw( glWidget );
    }

	glWidget->priv->xBegin = x;
	glWidget->priv->yBegin = y;

	return TRUE;
}


int gtk_gl_widget_open_model( GtkGlWidget *glWidget, const char *name )
{	
	vec3d centroid;
	vec3d boundingBox[2];
	
	g_return_if_fail (glWidget != NULL);
	g_return_if_fail (GTK_IS_GL_WIDGET (glWidget));
	
	if( glWidget->priv->hasModel ) obj_delete( glWidget->priv->model ); //remove the previous model
	glWidget->priv->model = obj_load_from_file( name );
	if( !glWidget->priv->model )
	{
		glWidget->priv->hasModel = 0;
		return 1;
	}
	else glWidget->priv->hasModel = 1;
	glWidget->priv->hasTexture = 0; //reset the texture
	
	obj_resize( glWidget->priv->model );
	
	//calculate the center of graviy of the model
	obj_get_centroid( glWidget->priv->model, &centroid );
	glWidget->priv->cx = centroid.x;
	glWidget->priv->cy = centroid.y;
	glWidget->priv->cz = centroid.z;
	
	//save the original position of the model
	obj_get_bounding_box( glWidget->priv->model, boundingBox );
	bounding_box_centroid( boundingBox, &centroid );
	glWidget->priv->x = centroid.x;
	glWidget->priv->y = centroid.y;
	glWidget->priv->z = centroid.z;
	
	//recenter the model so that the center of the bounding box is on the origin
	obj_replace( glWidget->priv->model );
	
	//re-position the center of gravity of the model so that it follows the refocusing of the model
	glWidget->priv->cx -= glWidget->priv->x;
	glWidget->priv->cy -= glWidget->priv->y;
	glWidget->priv->cz -= glWidget->priv->z;
	
	//re-init view and scale
	glWidget->priv->quatRot[0] = 0.0;
	glWidget->priv->quatRot[1] = 0.0;
	glWidget->priv->quatRot[2] = 0.0;
	glWidget->priv->quatRot[3] = 1.0;
	glWidget->priv->scale = 1.0;
	
	//actualize view
	gtk_gl_widget_draw( glWidget );
	
	return 0;
}

int gtk_gl_widget_open_model_texture( GtkGlWidget *glWidget, const char *name )
{
	if( glWidget->priv->hasModel == FALSE ) return 1;
	
	glWidget->priv->texture = load_texture( name );
	glWidget->priv->hasTexture = TRUE;
	gtk_gl_widget_draw( glWidget );
	return 0;
}


static void gtk_gl_widget_size_request( GtkWidget *widget, GtkRequisition *requisition )
{
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_GL_WIDGET (widget));
	g_return_if_fail (requisition != NULL);
	
	requisition->width = GTK_GL_WIDGET_DEFAULT_SIZE;
	requisition->height = GTK_GL_WIDGET_DEFAULT_SIZE;	
}

static void gtk_gl_widget_size_allocate( GtkWidget * widget, GtkAllocation * allocation )
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_GL_WIDGET(widget));
	g_return_if_fail(allocation != NULL );
	
	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_move_resize ( widget->window, allocation->x, allocation->y, allocation->width, allocation->height );
	
}

static void gtk_gl_widget_realize (GtkWidget * widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_GL_WIDGET(widget));


	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	widget->window = gdk_window_new ( gtk_widget_get_parent_window (widget), &attributes, attributes_mask );

	gdk_window_set_user_data (widget->window, widget);

	widget->style = gtk_style_attach (widget->style, widget->window);
	gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

static gboolean gtk_gl_widget_expose( GtkWidget * widget, GdkEventExpose * event )
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail (GTK_IS_GL_WIDGET(widget));
	g_return_if_fail (event != NULL);
	
	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) return FALSE;

	glViewport( 0, 0, (GLfloat)widget->allocation.width, (GLfloat)widget->allocation.height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective(70,(double)widget->allocation.width/(double)widget->allocation.height,1,1000);

	gdk_gl_drawable_gl_end (gldrawable);
	
	gtk_gl_widget_draw( GTK_GL_WIDGET(widget) );
	
	return FALSE;
}


static void gtk_gl_widget_destroy( GtkObject * object )
{
	
	GtkGlWidget * glWidget;
	GtkGlWidgetClass * klass;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_GL_WIDGET(object));

	glWidget = GTK_GL_WIDGET(object);
	if( glWidget->priv != NULL )
	{
		if( glWidget->priv->hasModel ) obj_delete( glWidget->priv->model );
		g_free( glWidget->priv );
		glWidget->priv = NULL;
	}
	
	klass = gtk_type_class(gtk_widget_get_type ());

	if (GTK_OBJECT_CLASS (klass)->destroy)
	{
		(* GTK_OBJECT_CLASS (klass)->destroy) (object);
	}
}

static void gtk_gl_widget_class_init( GtkGlWidgetClass *klass )
{
	
	GtkWidgetClass * widget_class;
	GtkObjectClass * object_class;

	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;

	widget_class->realize = gtk_gl_widget_realize;
	widget_class->size_request = gtk_gl_widget_size_request;
	widget_class->size_allocate = gtk_gl_widget_size_allocate;
	widget_class->expose_event = gtk_gl_widget_expose;
	widget_class->button_press_event = gtk_gl_widget_button_press_event;
	widget_class->motion_notify_event = gtk_gl_widget_motion_notify_event;
	widget_class->scroll_event = gtk_gl_widget_scroll_event;

	object_class->destroy = gtk_gl_widget_destroy;
}


static void gtk_gl_widget_init( GtkGlWidget *glWidget )
{	
	glWidget->priv = g_malloc( sizeof(GtkGlWidgetPrivate) );
	
	glWidget->priv->model = NULL;
	glWidget->priv->hasModel = FALSE;
	glWidget->priv->hasTexture = FALSE;
	glWidget->priv->showBoundingBox = FALSE;
	glWidget->priv->showOrigin = FALSE;
	glWidget->priv->showCentroid = FALSE;
	glWidget->priv->wiredMode = FALSE;
	glWidget->priv->centerModel = FALSE;
	glWidget->priv->enableLighting = FALSE;

	glWidget->priv->xBegin = 0.0;
	glWidget->priv->yBegin = 0.0;
	glWidget->priv->quatRot[0] = 0.0;
	glWidget->priv->quatRot[1] = 0.0;
	glWidget->priv->quatRot[2] = 0.0;
	glWidget->priv->quatRot[3] = 1.0;
	glWidget->priv->scale = 1.0;
	glWidget->priv->x = 0.0;
	glWidget->priv->y = 0.0;
	glWidget->priv->z = 0.0;
	glWidget->priv->cx = 0.0;
	glWidget->priv->cy = 0.0;
	glWidget->priv->cz = 0.0;
	glWidget->priv->bgColorRed = 0.0;
	glWidget->priv->bgColorGreen = 0.0;
	glWidget->priv->bgColorBlue = 0.0;
}

GtkType gtk_gl_widget_get_type()
{
	static GtkType gtk_gl_widget_type = 0;

	if ( !gtk_gl_widget_type )
	{
		static const GtkTypeInfo gtk_gl_widget_info = {
			"GtkGlWidget",
			sizeof (GtkGlWidget),
			sizeof (GtkGlWidgetClass),
			(GtkClassInitFunc) gtk_gl_widget_class_init,
			(GtkObjectInitFunc) gtk_gl_widget_init,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL
		};
		
		gtk_gl_widget_type = gtk_type_unique (GTK_TYPE_WIDGET, &gtk_gl_widget_info);
	}
	  
	return gtk_gl_widget_type;
}

GtkWidget *gtk_gl_widget_new( GdkGLConfig *glConfig )
{
	GtkGlWidget *glWidget;
	
	glWidget = gtk_type_new( GTK_GL_WIDGET_TYPE );
	
	gtk_widget_set_gl_capability ( GTK_WIDGET(glWidget), glConfig, NULL, TRUE, GDK_GL_RGBA_TYPE );
	
	gtk_widget_add_events ( GTK_WIDGET(glWidget), GDK_BUTTON1_MOTION_MASK | GDK_BUTTON2_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK );

	//I didn't find an other way
	g_signal_connect_after (G_OBJECT (glWidget), "realize", G_CALLBACK (gtk_gl_widget_init_gl), NULL);
	
	return GTK_WIDGET( glWidget );
}
