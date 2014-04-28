/* dwm_statusbar
 * author: d4ryus - https://github.com/d4ryus/
 * inspired by several ideas from dwm.suckless.org
 * vim:ts=4:sw=4:ai:
 */

/* std inputs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
/* input to display error messages */
#include <errno.h>
/* Alsa sound includes to get volume */
#include <alsa/asoundlib.h>
/* Xlibs to connect to xserver */
#include <X11/Xlib.h>
/* input to get current time */
#include <time.h>

/* config values, change them so that they fit ur system. if u change these to
 * wrong values the statusbar will not work properly, or not at all */
const static char* BATTERY_STATUS  = "/sys/class/power_supply/BAT0/status";
const static char* BATTERY_FULL    = "/sys/class/power_supply/BAT0/energy_full";
const static char* BATTERY_NOW     = "/sys/class/power_supply/BAT0/energy_now";
const static char* RAM             = "/proc/meminfo";
const static char* NETDEV          = "/proc/net/dev";
const static char* STAT            = "/proc/stat";
const static int   CPU_CORES       = 4;

/* displayed strings, feel free to change them to whatever you like, changes 
 * will just affect the displayed information. */
const static char* displayed_begin        = "[";
const static char* displayed_between      = "] [";
const static char* displayed_end          = "]";
const static char* displayed_battery_info = "bat: ";
const static char* displayed_ram_info     = "";
const static char* displayed_time_info    = "";
const static char* displayed_sound_info   = "vol: ";
const static char* displayed_loadavg_info = "";
const static char* displayed_netdev_info  = "";
const static char* displayed_stat_info    = "";

/* thread sleep config */
const static int fast_refresh  = 200000; // microseconds
const static int update_sleep  = 1; // seconds
const static int battery_sleep = 60;
const static int ram_sleep     = 5;
const static int time_sleep    = 60;
const static int sound_sleep   = 1;
const static int loadavg_sleep = 10;
const static int stat_sleep    = 1;

/* these values will update during execution */
static int  fast_refresh_flag = 0;
static char displayed_text[512];

static char displayed_battery[17];
static char displayed_ram[9];
static char displayed_time[17];
static char displayed_sound[9];
static char displayed_loadavg[12];
static char displayed_netdev[20];
static char displayed_stat[30];

void error(char*);
void *update_time(void*);
void *update_battery(void*);
void *update_ram(void*);
void *update_sound(void*);
void *update_loadavg(void*);
void *update_netdev(void*);
void *update_stat(void*);
void *update_status(void*);
