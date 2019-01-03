# objviewer makefile

TARGET=objviewer

CC = gcc
LDFLAGS = `pkg-config --libs gtkglext-1.0` -lm
CFLAGS = `pkg-config --cflags gtkglext-1.0` -O6

RELEASEDIR = bin
OBJSDIR = obj



OBJ = $(OBJSDIR)/main.o \
	$(OBJSDIR)/signals.o \
	$(OBJSDIR)/obj.o \
	$(OBJSDIR)/trackball.o \
	$(OBJSDIR)/utils.o \
	$(OBJSDIR)/gtkglwidget.o \
	$(OBJSDIR)/mtl.o
HEADERFILES = signals.h \
	obj.h \
	gtkglwidget.h \
	trackball.h \
	utils.h \
	mtl.h


all: $(OBJSDIR) $(OBJ) $(HEADERFILES)
	$(CC) $(OBJ) -o $(RELEASEDIR)/$(TARGET) $(LDFLAGS)
	
$(OBJSDIR)/%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
	
$(OBJSDIR):
	mkdir $(OBJSDIR)

.PHONY: clean, mrproper

clean: 
	rm -rf $(OBJSDIR)/*.o
	
mrproper: clean
	rm -rf $(RELEASEDIR)/$(TARGET)
