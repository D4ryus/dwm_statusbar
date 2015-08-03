# Makefile for dwm_statusbar
# author: d4ryus - https://github.com/d4ryus/
# X11 libs needed, also posix threads and alsa libs
# vim:ts=8:sw=8:noet:

# magic place

CC = gcc

EXECUTABLE = dwm_statusbar

OBJS = dwm_statusbar.o

GPROF_FILE = performance.txt

LFLAGS = -lX11 -lasound -lpthread -pg

# default flags
CFLAGS = -Wall \
         -Wstrict-prototypes \
         -Wmissing-prototypes \
         -Wno-main \
         -Wno-uninitialized \
         -Wbad-function-cast \
         -Wcast-align \
         -Wcast-qual \
         -Wextra \
         -Wmissing-declarations \
         -Wpointer-arith \
         -Wshadow \
         -Wsign-compare \
         -Wuninitialized \
         -Wunused \
         -Wno-unused-parameter \
         -Wnested-externs \
         -Wunreachable-code \
         -Winline \
         -Wdisabled-optimization \
         -Wconversion \
         -Wfloat-equal \
         -Wswitch \
         -Wswitch-default \
         -Wtrigraphs \
         -Wsequence-point \
         -Wimplicit \
         -Wstack-protector \
         -Woverlength-strings \
         -Waddress \
         -Wdeclaration-after-statement

# CFLAGS += -std=c99
# CFLAGS += -pedantic
# CFLAGS += -Wredundant-decls
# CFLAGS += -Werror
# CFLAGS += -Wpadded
CFLAGS += -fPIC
CFLAGS += -ggdb
CFLAGS += -pg
CFLAGS += -O3
CFLAGS += -D_FILE_OFFSET_BITS=64

.PHONY : all
all : depend $(EXECUTABLE)

.PHONY : $(EXECUTABLE)
$(EXECUTABLE) : $(OBJS)
	$(CC) -o $@ $(OBJS) $(LFLAGS)

-include .depend

.PHONY : depend
depend:
	$(CC) -E -MM *.c > .depend

.PHONY : clean
clean :
	rm $(OBJS) $(EXECUTABLE) .depend

# targets to run
run_$(EXECUTABLE) : $(EXECUTABLE)
	./$(EXECUTABLE)

perf :
	gprof $(EXECUTABLE) gmon.out > $(GPROF_FILE)

graph : perf
	gprof2dot $(GPROF_FILE) -n0 -e0 > graph.dot
	dot -Tsvg graph.dot -o graph.svg
	sfdp -Gsize=100! \
             -Gsplines=true \
             -Goverlap=prism \
             -Tpng graph.dot \
             > graph.png
