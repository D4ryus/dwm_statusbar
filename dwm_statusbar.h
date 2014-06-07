/* dwm_statusbar
 * author: d4ryus - https://github.com/d4ryus/
 * inspired by several ideas from dwm.suckless.org
 * vim:ts=4:sw=4:ai:foldmethod=syntax:
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
const static int   REFRESH         = 1;

typedef struct _info Info;

typedef struct
_info
{
    char* name;
    char* before;
    char* text;
    char* after;
    int   sleep;
    void (*fun) (Info*);
} Info;

void *update_time(Info*);
void *update_battery(Info*);
void *update_ram(Info*);
void *update_sound(Info*);
void *update_loadavg(Info*);
void *update_netdev(Info*);
void *update_stat(Info*);

static Info infos[] = {
//    name        displayed text            sleep function
    { "stat"    , "["        , NULL , "] " , 1  , update_netdev  },
    { "cpu"     , "["        , NULL , ""   , 1  , update_stat    },
    { "loadavg" , "["        , NULL , ""   , 10 , update_loadavg },
    { "ram"     , "["        , NULL , ""   , 5  , update_ram     },
    { "sound"   , "[ audio:" , NULL , ""   , 1  , update_sound   },
    { "battery" , "[ bat: "  , NULL , ""   , 60 , update_battery },
    { "time"    , "["        , NULL , ""   , 30 , update_time    },
};
