# Makefile for wmJupiter
# 
CC     = gcc
CFLAGS = -O2 -Wall
INCDIR = -I/usr/X11R6/include/X11 -I/usr/X11R6/include
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib
# for linux
LIBS   = -lXpm -lX11 -lXext -lm
# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lsocket
OBJS   = wmJupiter.o \
         xutils.o


.c.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)


all:	wmJupiter.o wmJupiter

wmJupiter.o: wmJupiter_master.xpm wmJupiter_mask.xbm
wmJupiter:	$(OBJS) 
	$(CC) $(CFLAGS) $(SYSTEM) -o wmJupiter $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmJupiter

install:: wmJupiter
	install -c -s -m 0755 wmJupiter $(DESTDIR)/bin

