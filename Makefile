# Makefile for KevEdit

# Choose your compiler
CC = i586-pc-msdosdjgpp-gcc
#CC = gcc

# Uncomment next line to optimize kevedit
# Uncomment second line to not optimize and include debugging information
OPTIMIZE = -s -O3 -fexpensive-optimizations -fomit-frame-pointer -finline-functions -funroll-loops -march=pentium
#OPTIMIZE = -g -Wall

# Set CGI to ON to enable GGI display
GGI =
# Set VCSA to ON to enable VCSA display
VCSA =
# Set DOS to ON to enable DOS display
DOS = ON

CFLAGS = $(OPTIMIZE) $(GGI) $(VCSA) $(DOS) -DVERSION=\"0.3.4\"

# No more modifications below this line
# -------------------------------------

# Set up display settings
ifeq ($(GGI),ON)
  GGI = -DGGI -lggi
  GGIOBJ = display_ggi.o
endif

ifeq ($(VCSA),ON)
  VCSA = -DVCSA
  VCSAOBJ = display_vcsa.o
endif

ifeq ($(DOS),ON)
  DOS = -DDOS
	DOSOBJ = display_dos.o
endif

# Objects

CENTRALOBJS = main.o misc.o menu.o editbox.o screen.o
LIBRARYOBJS = libzzt.o svector.o files.o zzm.o zzl.o selection.o zlaunch.o helplist.o hypertxt.o gradient.o
MISCOBJS    = patbuffer.o help.o infobox.o register.o
DRAWOBJS    = panel.o panel_f1.o panel_f2.o panel_f3.o panel_ed.o panel_hl.o panel_bi.o panel_wi.o panel_g1.o panel_g2.o scroll.o tbox.o cbox.o
DISPLAYOBJS = display.o $(GGIOBJ) $(VCSAOBJ) $(DOSOBJ)

OBJECTS = $(CENTRALOBJS) $(LIBRARYOBJS) $(MISCOBJS) $(DRAWOBJS) $(DISPLAYOBJS)

.SUFFIXES: .o .c .h

# Targets
all: kevedit

kevedit: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(CFLAGS)

.cpp.o:
	$(CC) -o $@ $< $(CFLAGS) -c

clean:
	rm -f *.o kevedit

# Dependancies

# Central KevEditing routines
main.o: main.c kevedit.h misc.h menu.h editbox.h screen.h zzt.h patbuffer.h help.h register.h infobox.h display.h
misc.o: misc.c misc.h kevedit.h editbox.h screen.h svector.h hypertxt.h selection.h patbuffer.h display.h
menu.o: menu.c menu.h kevedit.h screen.h editbox.h zzt.h svector.h files.h zzl.h hypertxt.h patbuffer.h display.h
editbox.o: editbox.c editbox.h screen.h svector.h zzm.h colours.h register.h help.h scroll.h panel_ed.h display.h
screen.o: screen.c screen.h kevedit.h editbox.h zzt.h hypertxt.h zlaunch.h panel.h panel_f1.h panel_f2.h panel_f3.h scroll.h tbox.h cbox.h

# libzzt
libzzt.o: libzzt.c zzt.h

# Other libraries
svector.o: svector.c svector.h
files.o: files.c files.h svector.h
zzm.o: zzm.c zzm.h svector.h editbox.h kevedit.h
zzl.o: zzl.c zzl.h svector.h editbox.h kevedit.h
selection.o: selection.c
zlaunch.o: zlaunch.c zlaunch.h svector.h
helplist.o: helplist.c helplist.h svector.h
hypertxt.o: hypertxt.c hypertxt.h svector.h
gradient.o: gradient.c gradient.h

# Misc
patbuffer.o: patbuffer.c kevedit.h zzt.h display.h
help.o: help.c help.h svector.h editbox.h hypertxt.h panel_hl.h helplist.h
infobox.o: infobox.c infobox.h zzt.h display.h
register.o: register.c register.h editbox.h

# Draw data structures
panel.o: panel.c panel.h
panel_f1.o: panel_f1.c panel_f1.h
panel_f2.o: panel_f2.c panel_f2.h
panel_f3.o: panel_f3.c panel_f3.h
panel_ed.o: panel_ed.c panel_ed.h
panel_hl.o: panel_hl.c panel_hl.h
panel_bi.o: panel_bi.c panel_bi.h
panel_wi.o: panel_wi.c panel_wi.h
panel_g1.o: panel_g1.c panel_g1.h
panel_g2.o: panel_g2.c panel_g2.h
scroll.o: scroll.c scroll.h
tbox.o: tbox.c tbox.h
cbox.o: cbox.c cbox.h

# Display libraries
display.o: display.c display.h
display_ggi.o: display_ggi.c display.h display_ggi.h
display_vcsa.o: display_vcsa.c display.h display_vcsa.h
display_dos.o: display_dos.c display.h display_dos.h
