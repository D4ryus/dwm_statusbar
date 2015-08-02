/* dwm_statusbar
 * author: d4ryus - https://github.com/d4ryus/
 * inspired by several ideas from dwm.suckless.org
 * file: config.h
 * vim:ts=8:sw=8:noet:
 */

// config values, change them so that they fit ur system. if u change these to
// wrong values the statusbar will not work properly, or not at all
static const char *BATTERY_STATUS = "/sys/class/power_supply/BAT0/status";
static const char *BATTERY_FULL   = "/sys/class/power_supply/BAT0/energy_full";
static const char *BATTERY_NOW    = "/sys/class/power_supply/BAT0/energy_now";
static const char *RAM            = "/proc/meminfo";
static const char *NETDEV         = "/proc/net/dev";
static const char *STAT           = "/proc/stat";
static const int CPU_CORES        = 4;
static const int NETDEVCOUNT      = 3;
static const char *PORT           = "8383";
static const uint8_t REFRESH      = 1;
static const uint16_t MSG_LENGTH  = 256;

// array with all values, just outcommend stuff u do not want to display
static struct Info infos[] = {
// name     , first char *, dynamic char *, end char *, sleep , function
  {"netdev" , "["         , NULL          , "] "      , 1     , update_netdev },
//{"netmsg" , "["         , NULL          , "] "      , 0     , update_netmsg },
  {"stat"   , "["         , NULL          , "] "      , 1     , update_stat   },
//{"loadavg", "["         , NULL          , "] "      , 10    , update_loadavg},
  {"ram"    , "["         , NULL          , "] "      , 5     , update_ram    },
  {"sound"  , "[audio: "  , NULL          , "] "      , 1     , update_sound  },
  {"battery", "[bat: "    , NULL          , "] "      , 60    , update_battery},
  {"time"   , "["         , NULL          , "] "      , 30    , update_time   },
};
