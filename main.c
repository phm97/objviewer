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

#include "signals.h"


int main( int argc, char *argv[] )
{ 	
	GtkWidget *window;
	GtkBuilder *builder;
	GdkGLConfig *glConfig;

	
	gtk_init(&argc, &argv);
	gtk_gl_init (&argc, &argv);
	
	
	//Initializing OpenGL
	//Try double-buffered visual
	glConfig = gdk_gl_config_new_by_mode( GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE );
	if (glConfig == NULL)
	{
		g_print ("Cannot find the double-buffered visual.\n");
		g_print ("Trying single-buffered visual.\n");

		//Try single-buffered visual
		glConfig = gdk_gl_config_new_by_mode( GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH );
		if (glConfig == NULL)
		{
			g_print ("No appropriate OpenGL-capable visual found.\n");
			return 1;
		}
		else g_print("OpenGL Single Buffered mode enabled");
	}
	else g_print("OpenGL double buffered mode enabled\n");
	
	
	//Load all the widgets from a XML file
	builder = gtk_builder_new();
	if( !gtk_builder_add_from_file (builder, "mainwindow.glade", NULL)) 
	{
		g_print("error : could not load mainwindow.glade\n");
		g_object_unref(builder);
		return 1;
	}	
	
	
	//Connect the signals
	gtk_builder_connect_signals( builder, NULL ); //for gtk_main_quit
	connect_signals( builder, glConfig );
	
	//Make sure all the widgets are visible
	window = GTK_WIDGET( gtk_builder_get_object( builder, "window_main" ) );
	gtk_widget_show_all(window);

	//Let's run the Application !!!
	gtk_main();
	
	
	g_object_unref(builder);
	
	return 0;
}
