/* dwm_statusbar
 * author: d4ryus - https://github.com/d4ryus/
 * inspired by several ideas from dwm.suckless.org
 * file: dwm_statusbar.h
 * vim:ts=4:sw=4:ai:foldmethod=syntax:
 */

// std inputs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
// input to display error messages
#include <errno.h>
// Alsa sound includes to get volume
#include <alsa/asoundlib.h>
// Xlibs to connect to xserver
#include <X11/Xlib.h>
// input to get current time
#include <time.h>
// net includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct _thread_info Info;

struct
_thread_info
{
    char* name;           // name of thread
    char* before;         // displayed text in front
    char* text;           // pointer to char array which will be updated by thread
    char* after;          // displayed text after dynamic text
    int   sleep;          // sleep time in seconds
    void* (*fun) (Info*); // pointer to function which will update the text
};

// forward declaration of used functions
void *update_netmsg(Info*);
void *update_netdev(Info*);
void *update_stat(Info*);
void *update_loadavg(Info*);
void *update_ram(Info*);
void *update_sound(Info*);
void *update_battery(Info*);
void *update_time(Info*);
