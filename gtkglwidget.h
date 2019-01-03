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

#ifndef __GTK_GL_WIDGET_APP_H
#define __GTK_GL_WIDGET_APP_H

#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>
#include "obj.h"

G_BEGIN_DECLS

#define GTK_GL_WIDGET_TYPE 				(gtk_gl_widget_get_type())
#define GTK_GL_WIDGET(obj)        		(GTK_CHECK_CAST((obj), GTK_GL_WIDGET_TYPE, GtkGlWidget))
#define GTK_GL_WIDGET_CLASS(klass)    	(GTK_CHECK_CLASS_CAST((klass), GTK_GL_WIDGET_TYPE, GtkGlWidgetClass))
#define GTK_IS_GL_WIDGET(obj)         	(GTK_CHECK_TYPE((obj), GTK_GL_WIDGET_TYPE))
#define GTK_IS_GL_WIDGET_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_GL_WIDGET_TYPE))
#define GTK_GL_WIDGET_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GTK_GL_WIDGET_TYPE, GtkGlWidgetClass))


typedef struct _GtkGlWidgetPrivate GtkGlWidgetPrivate;

typedef struct _GtkGlWidget GtkGlWidget;
struct _GtkGlWidget 
{
	GtkWidget widget;
	
	GtkGlWidgetPrivate *priv;
};

typedef struct _GtkGlWidgetClass GtkGlWidgetClass;
struct _GtkGlWidgetClass
{
	GtkWidgetClass parent_class;
};


GtkType gtk_gl_widget_get_type();
GtkWidget *gtk_gl_widget_new();
int gtk_gl_widget_open_model( GtkGlWidget *glWidget, const char *name );
int gtk_gl_widget_open_model_texture( GtkGlWidget *glWidget, const char *name );
gboolean gtk_gl_widget_has_model( GtkGlWidget *glWidget );
ObjModel *gtk_gl_widget_get_model( GtkGlWidget *glWidget );
void gtk_gl_widget_show_origin( GtkGlWidget *glWidget, gboolean b );
void gtk_gl_widget_show_bounding_box( GtkGlWidget *glWidget, gboolean b );
void gtk_gl_widget_show_centroid( GtkGlWidget *glWidget, gboolean b );
void gtk_gl_widget_set_wired_mode( GtkGlWidget *glWidget, gboolean b );
void gtk_gl_widget_center_model( GtkGlWidget *glWidget, gboolean b );
void gtk_gl_widget_enable_lighting( GtkGlWidget *glWidget, gboolean b );
void gtk_gl_widget_set_background_color( GtkGlWidget *glWidget, GdkColor color );
void gtk_gl_widget_get_background_color( GtkGlWidget *glWidget, GdkColor *color );

G_END_DECLS

#endif