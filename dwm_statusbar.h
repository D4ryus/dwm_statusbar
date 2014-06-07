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

typedef struct
_info
{
    char* name;
    char* before;
    char* text;
    char* end;
    int   sleep;
} info;

typedef struct
_big_box
{
    //               name        displayed text            sleep
    info stat    = { "stat"    , "["        , NULL , "] " , 1  };
    info cpu     = { "cpu"     , "["        , NULL , ""   , 1  };
    info loadavg = { "loadavg" , "["        , NULL , ""   , 10 };
    info ram     = { "ram"     , "["        , NULL , ""   , 5  };
    info sound   = { "sound"   , "[ audio:" , NULL , ""   , 1  };
    info battery = { "battery" , "[ bat: "  , NULL , ""   , 60 };
    info time    = { "time"    , "["        , NULL , ""   , 30 };
} big_box;
