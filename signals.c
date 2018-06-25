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

#include "gtkglwidget.h"
#include "utils.h"


void screenshot( GtkWidget *widget, GtkGlWidget *glWidget )
{
	GtkWidget *dialog;
	GtkWindow *parentWindow;
	GtkFileChooser *chooser;
	GtkFileFilter *filter;
	int res;
	char *filename;
	
	GdkGLContext *glcontext = gtk_widget_get_gl_context( GTK_WIDGET(glWidget) );
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable( GTK_WIDGET(glWidget) );
	
	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) return;
	
	parentWindow = GTK_WINDOW( gtk_widget_get_toplevel(widget) );
	dialog = gtk_file_chooser_dialog_new ("Open File", parentWindow, GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);
	chooser = GTK_FILE_CHOOSER (dialog);
	
	gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);
	gtk_file_chooser_set_current_name (chooser, "Untitled.bmp" );
	
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern (filter, "*.bmp");
	gtk_file_filter_set_name( filter, "Windows  bitmap");
	gtk_file_chooser_add_filter( chooser, filter );
	
	res = gtk_dialog_run (GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		filename = gtk_file_chooser_get_filename(chooser);
		take_screenshot(filename);
		g_free (filename);
	}
	
	gtk_widget_destroy (dialog);
	gdk_gl_drawable_gl_end (gldrawable);
}

void view_bounding_box( GtkWidget *widget, GtkGlWidget *glWidget )
{
	gboolean b;
	
	b = gtk_check_menu_item_get_active( GTK_CHECK_MENU_ITEM(widget) );
	gtk_gl_widget_show_bounding_box( glWidget, b);
	gtk_gl_widget_actualize( glWidget );
}

void recenter( GtkWidget *widget, GtkGlWidget *glWidget )
{
	gboolean b;
	
	b = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget) );
	gtk_gl_widget_center_model( glWidget, b );
	gtk_gl_widget_actualize( glWidget );
}

void lines( GtkWidget *widget, GtkGlWidget *glWidget )
{
	gboolean mode;
	
	mode = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget) );
	gtk_gl_widget_set_wired_mode( glWidget, mode );
	gtk_gl_widget_actualize(glWidget);
}

void centroid( GtkWidget *widget, GtkGlWidget *glWidget )
{
	gboolean b;
	
	b = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget) );
	gtk_gl_widget_show_centroid( glWidget, b );
	gtk_gl_widget_actualize( glWidget );
}

void view_origin( GtkWidget *widget, GtkGlWidget *glWidget )
{
	gboolean a;
	
	a = gtk_check_menu_item_get_active( GTK_CHECK_MENU_ITEM(widget) );
	gtk_gl_widget_show_origin( glWidget, a);
	gtk_gl_widget_actualize( glWidget );
}

void open( GtkWidget *widget, GtkGlWidget *glWidget )
{
	GtkWidget *dialog, *parentWindow;
	GtkFileChooser *chooser;
	GtkFileFilter *filter;
	int res;
	char *filename;

	parentWindow = gtk_widget_get_toplevel(widget);
	dialog = gtk_file_chooser_dialog_new ("Open File", GTK_WINDOW(parentWindow), GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);
	chooser = GTK_FILE_CHOOSER (dialog);
	gtk_file_chooser_set_select_multiple( chooser, FALSE );
	
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern (filter, "*.obj");
	gtk_file_filter_set_name( filter, "Wavefront 3d model");
	gtk_file_chooser_add_filter( chooser, filter );
	
	res = gtk_dialog_run (GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{

		filename = gtk_file_chooser_get_filename (chooser);
		gtk_gl_widget_open_model( glWidget, filename );
		g_free (filename);
	}

	gtk_widget_destroy (dialog);
}

void open_texture( GtkWidget *widget, GtkGlWidget *glWidget )
{
	GtkWidget *dialog, *parentWindow;
	int res;
	char *filename;

	parentWindow = gtk_widget_get_toplevel(widget);
	
	if( gtk_gl_widget_has_model(glWidget) ==  TRUE )
	{
		dialog = gtk_file_chooser_dialog_new ("Open Texture File", GTK_WINDOW(parentWindow), GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);
		res = gtk_dialog_run (GTK_DIALOG (dialog));
		if (res == GTK_RESPONSE_ACCEPT)
		{
			GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
			filename = gtk_file_chooser_get_filename (chooser);
			gtk_gl_widget_open_model_texture( glWidget, filename );
			g_free (filename);
		}
	}
	else
	{
		dialog = gtk_message_dialog_new( GTK_WINDOW(parentWindow), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "You must load a model first" );					
		gtk_dialog_run(GTK_DIALOG(dialog));
	}
	gtk_widget_destroy (dialog);

}

void about( GtkWidget *widget, gpointer data )
{
	GtkWidget *parentWindow;
	
	const gchar *authors[] = {
		"Pierre Vilain",
		NULL
	};
	
	parentWindow = gtk_widget_get_toplevel(widget);
	
	gtk_show_about_dialog (GTK_WINDOW (parentWindow),
                        "program-name", "Obj Viewer",
						"logo-icon-name", "help-about",
                        "comments", "This program uses GTK+ 2 and OpenGL\n\nThe trackball camera used in this software is developped by Silicon Graphics. See trackball.h in the source code for more information",
                        "authors", authors,
                        NULL);
}


void connect_signals( GtkBuilder *builder, GdkGLConfig *glConfig )
{
	GObject *widget;
	GtkWidget *glWidget;

	
	widget = gtk_builder_get_object ( builder, "window_main" );
	g_signal_connect( widget, "destroy", G_CALLBACK (gtk_main_quit), NULL );
	
	widget = gtk_builder_get_object ( builder, "vbox1" );
	glWidget = gtk_gl_widget_new( glConfig );
	gtk_box_pack_start( GTK_BOX(widget), glWidget, TRUE, TRUE, 0 );
	gtk_box_reorder_child( GTK_BOX(widget), glWidget, 1 );
	
	widget = gtk_builder_get_object ( builder, "bb" );
	g_signal_connect( widget, "toggled", G_CALLBACK (view_bounding_box), glWidget );
	
	widget = gtk_builder_get_object ( builder, "origin" );
	g_signal_connect( widget, "toggled", G_CALLBACK (view_origin), glWidget );
	
	widget = gtk_builder_get_object ( builder, "open" );
	g_signal_connect( widget, "activate", G_CALLBACK (open), glWidget );
	
	widget = gtk_builder_get_object ( builder, "opentexture" );
	g_signal_connect( widget, "activate", G_CALLBACK (open_texture), glWidget );
	
	widget = gtk_builder_get_object ( builder, "screenshot" );
	g_signal_connect( widget, "activate", G_CALLBACK (screenshot), glWidget );
	
	widget = gtk_builder_get_object ( builder, "about" );
	g_signal_connect( widget, "activate", G_CALLBACK (about), NULL );
	
	widget = gtk_builder_get_object ( builder, "recenter" );
	g_signal_connect( widget, "toggled", G_CALLBACK (recenter), glWidget );
	
	widget = gtk_builder_get_object ( builder, "centroid" );
	g_signal_connect( widget, "clicked", G_CALLBACK (centroid), glWidget );
	
	widget = gtk_builder_get_object ( builder, "lines" );
	g_signal_connect( widget, "toggled", G_CALLBACK (lines), glWidget );
}



