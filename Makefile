# Makefile for dwm_statusbar
# author: d4ryus - https://github.com/d4ryus/
# X11 libs needed
# vim:noexpandtab

FLAGS = -lX11 -lasound -lpthread

dwm_statusbar: dwm_statusbar.c
	gcc dwm_statusbar.c -o dwm_statusbar ${FLAGS}
