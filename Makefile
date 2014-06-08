# Makefile for dwm_statusbar
# author: d4ryus - https://github.com/d4ryus/
# X11 libs needed, also posix threads and alsa libs
# vim:noexpandtab

FLAGS = -lX11 -lasound -lpthread -Wall -g
HEADER = config.h dwm_statusbar.h

all: dwm_statusbar stat_msg

dwm_statusbar: dwm_statusbar.c ${HEADER}
	gcc dwm_statusbar.c -o dwm_statusbar ${FLAGS}

stat_msg: stat_msg.c
	gcc stat_msg.c -o stat_msg
