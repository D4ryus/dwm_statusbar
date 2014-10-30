/* dwm_statusbar
 * author: d4ryus - https://github.com/d4ryus/
 * inspired by several ideas from dwm.suckless.org
 * file: config.h
 * vim:ts=4:sw=4:ai:foldmethod=syntax:
 */

// config values, change them so that they fit ur system. if u change these to
// wrong values the statusbar will not work properly, or not at all
const static char* BATTERY_STATUS = "/sys/class/power_supply/BAT0/status";
const static char* BATTERY_FULL   = "/sys/class/power_supply/BAT0/energy_full";
const static char* BATTERY_NOW    = "/sys/class/power_supply/BAT0/energy_now";
const static char* RAM            = "/proc/meminfo";
const static char* NETDEV         = "/proc/net/dev";
const static char* STAT           = "/proc/stat";
const static int   CPU_CORES      = 4;
const static int   NETDEVCOUNT    = 3;
const static int   REFRESH        = 1;
const static char* PORT           = "8383"; // port for net message
static const int   MSG_LENGTH     = 256;   // msg length for net message

// array with all values, just outcommend stuff u do not want to display
static struct Info infos[] = {
//    name      , first char* , dynamic char*, end char* , sleep , function
    { "netdev"  , "["         , NULL         , "] "      , 1     , update_netdev  },
    { "netmsg"  , "["         , NULL         , "] "      , 0     , update_netmsg  },
    { "stat"    , "["         , NULL         , "] "      , 1     , update_stat    },
//  { "loadavg" , "["         , NULL         , "] "      , 10    , update_loadavg },
    { "ram"     , "["         , NULL         , "] "      , 5     , update_ram     },
//  { "sound"   , "[audio: "  , NULL         , "] "      , 1     , update_sound   },
//  { "battery" , "[bat: "    , NULL         , "] "      , 60    , update_battery },
    { "time"    , "["         , NULL         , "] "      , 30    , update_time    },
};
