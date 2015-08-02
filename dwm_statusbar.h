/* dwm_statusbar
 * author: d4ryus - https://github.com/d4ryus/
 * inspired by several ideas from dwm.suckless.org
 * file: dwm_statusbar.h
 * vim:ts=8:sw=8:noet
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
#include <sys/socket.h>
#include <netinet/in.h>

struct Info {
    char* name;   // name of thread
    char* before; // displayed text in front
    char* text;   // pointer to char array which will be updated by thread
    char* after;  // displayed text after dynamic text
    uint8_t sleep;  // sleep time in seconds
    void* (*fun) (struct Info*); // pointer to function which will update the text
};

void error(char *);
char *fmt_size(uint64_t, char[7]);
void *update_status(void);
int pthread_setname_np(pthread_t, char *);
void swap_text(struct Info *, char *);

// forward declaration of used functions
void *update_netmsg(struct Info*);
void *update_netdev(struct Info*);
void *update_stat(struct Info*);
void *update_loadavg(struct Info*);
void *update_ram(struct Info*);
void *update_sound(struct Info*);
void *update_battery(struct Info*);
void *update_time(struct Info*);
