# Makefile for dwm_statusbar
# author: d4ryus - https://github.com/d4ryus/
# X11 libs needed
# vim:noexpandtab

FLAGS = -lX11 -lasound

main: main.c
	gcc main.c -o main ${FLAGS}
