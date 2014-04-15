/* dwm_statusbar
 * author: d4ryus - https://github.com/d4ryus/
 * inspired by several ideas from dwm.suckless.org
 * also thanks to Yu-Jie Lin, most of the alsa code is from him.
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
/* network includes, needed to display ip adress */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

/* will be known by compile time */
const static char* BATTERY_STATUS  = "/sys/class/power_supply/BAT0/status";
const static char* BATTERY_FULL    = "/sys/class/power_supply/BAT0/energy_full";
const static char* BATTERY_NOW     = "/sys/class/power_supply/BAT0/energy_now";
const static char* RAM             = "/proc/meminfo";
const static char* WIFI_INTERFACE  = "wlp3s0";   // max 10 charscters
const static char* CABLE_INTERFACE = "enp0s25";  // max 10 characters

const static char* displayed_begin        = "[";
const static char* displayed_between      = "] [";
const static char* displayed_end          = "]";
const static char* displayed_ip_info      = "";
const static char* displayed_battery_info = "bat: ";
const static char* displayed_ram_info     = "ram: ";
const static char* displayed_time_info    = "";
const static char* displayed_sound_info   = "vol: ";

const static int   update_sleep  = 1;
const static int   ip_sleep      = 1;
const static int   battery_sleep = 60;
const static int   ram_sleep     = 5;
const static int   time_sleep    = 60;
const static int   sound_sleep   = 1;

/* will be set by init() */
static int         energy_full;
static Display*    display;

/* will be updated during execution */
static char        displayed_text[512];

static char        displayed_battery[17];
static char        displayed_ram[12];
static char        displayed_time[17];
static char        displayed_ip_wifi[30];
static char        displayed_ip_cable[30];
static char        displayed_sound[9];

/*
 * prints errormessages, will exit the programm.
 */
void error(char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/* set all values which will not change during updates */
void init() {
    if (!(display = XOpenDisplay(NULL))) {
        error("Cannot open display");
    }
    FILE* fd;
    fd = fopen(BATTERY_FULL, "r");
    if(fd == NULL) {
        error("Error opening energy_full");
    }
    fscanf(fd, "%d", &energy_full);
    fclose(fd);
}

void *update_time(void * val) {
    while(1) {
        struct tm*  timeinfo;
        time_t      rawtime;

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strncpy(displayed_time, asctime(timeinfo), 16);
        sleep(time_sleep);
    }
}

void *update_battery(void * val) {
    while(1) {
        int  battery_percentage;
        int  energy_now;
        char energy_status[12];
        FILE *fd;

        fd = fopen(BATTERY_NOW, "r");
        if(fd == NULL) {
            error("Error opening energy_now");
        }
        fscanf(fd, "%d", &energy_now);
        fclose(fd);

        fd = fopen(BATTERY_STATUS, "r");
        if(fd == NULL) {
            error("Error opening energy_status");
        }
        fscanf(fd, "%s", &energy_status);
        fclose(fd);

        battery_percentage = (int)((((float)energy_now) / ((float)energy_full) * 100.0) + 0.5);

        sprintf(displayed_battery, "%s %d%s", energy_status, battery_percentage, "%");
        sleep(battery_sleep);
    }
}

void *update_ram(void * val) {
    while(1) {
        int  ram_total;
        int  ram_available;
        int  ram_free;
        int  ram_used;
        char buffer[64];
        FILE *fd;

        fd = fopen(RAM, "r");

        if(fd == NULL) {
            error("Error opening meminfo");
        }
        fscanf(fd, "%s %d %s", &buffer, &ram_total,  &buffer);
        fscanf(fd, "%s %d %s", &buffer, &ram_free, &buffer);
        fscanf(fd, "%s %d",    &buffer, &ram_available);
        fclose(fd);

        ram_total /= 1000;
        ram_available /= 1000;
        ram_used = ram_total - ram_available;

        sprintf(displayed_ram, "%d/%d", ram_used, ram_total);
        sleep(ram_sleep);
    }
}

void *update_ip(void * val) {
    while(1) {
        char         wifi_ip[17];
        char         cable_ip[17];
        int wifi_connected  = 0; // just boolean flag
        int cable_connected = 0; // just boolean flag
        int          fd;
        struct ifreq ifr_wifi;
        struct ifreq ifr_cable;

        //Type of address to retrieve - IPv4 IP address
        ifr_wifi.ifr_addr.sa_family  = AF_INET;
        ifr_cable.ifr_addr.sa_family = AF_INET;

        //Copy the interface name in the ifreq structure
        strncpy(ifr_wifi.ifr_name,  WIFI_INTERFACE,  IFNAMSIZ-1);
        strncpy(ifr_cable.ifr_name, CABLE_INTERFACE, IFNAMSIZ-1);

        fd = socket(AF_INET, SOCK_DGRAM, 0);
        ioctl(fd, SIOCGIFADDR, &ifr_wifi);
        ioctl(fd, SIOCGIFADDR, &ifr_cable);
        close(fd);

        // copy ip to global string
        strncpy(wifi_ip,  inet_ntoa(( (struct sockaddr_in *)&ifr_wifi.ifr_addr  )->sin_addr), 16);
        strncpy(cable_ip, inet_ntoa(( (struct sockaddr_in *)&ifr_cable.ifr_addr )->sin_addr), 16);

        int w1, w2, w3, w4;
        int c1, c2, c3, c4;
        sscanf(wifi_ip,  "%d.%d.%d.%d", &w1, &w2, &w3, &w4);
        sscanf(cable_ip, "%d.%d.%d.%d", &c1, &c2, &c3, &c4);

        /* I have an strange bug where when there is no ip assigned to my wifi
         * interface i get ip 118.97.105.108, thats why this section is here.
         * will take a look into it when i have time.
         */
        if( ( w1 == 0   && w2 == 0  && w3 == 0   && w4 == 0   ) ||
            ( w1 == 118 && w2 == 97 && w3 == 105 && w4 == 108 ) ) {
            wifi_connected = 0;
        } else {
            wifi_connected = 1;
        }

        if( c1 == 0 && c2 == 0 && c3 == 0 && c4 == 0 ) {
            cable_connected = 0;
        } else {
            cable_connected = 1;
        }


        if        ( wifi_connected &&  cable_connected ) {

            sprintf(displayed_ip_wifi, "%s: %s", WIFI_INTERFACE, wifi_ip);
            sprintf(displayed_ip_cable, "%s: %s", CABLE_INTERFACE, cable_ip);

        } else if ( wifi_connected && !cable_connected ) {

            sprintf(displayed_ip_wifi, "%s: %s", WIFI_INTERFACE, wifi_ip);
            displayed_ip_cable[0] = '\0'; // no cable assigned

        } else if (!wifi_connected &&  cable_connected ) {

            displayed_ip_wifi[0] = '\0'; // no wifi assigned
            sprintf(displayed_ip_cable, "%s: %s", CABLE_INTERFACE, cable_ip);

        } else if (!wifi_connected && !cable_connected ) {

            strncpy(displayed_ip_wifi, "not connected", 13);
            displayed_ip_cable[0] = '\0'; // no cable assigned

        }
        sleep(ip_sleep);
    }
}

void *update_sound(void * val) {
    while(1) {
        int  err;
        int  switch_value;
        long vol;
        long vol_min;
        long vol_max;
        snd_mixer_t          *h_mixer;
        snd_mixer_selem_id_t *sid;
        snd_mixer_elem_t     *elem ;

        if (snd_mixer_open(&h_mixer, 1) < 0)
            error("Alsa_Mixer open error");

        if (snd_mixer_attach(h_mixer, "default") < 0)
            error("Alsa_Mixer attach error");

        if (snd_mixer_selem_register(h_mixer, NULL, NULL) < 0)
            error("Alsa_Mixer simple element register error");

        if (snd_mixer_load(h_mixer) < 0)
            error("Alsa_Mixer load error");

        snd_mixer_selem_id_alloca(&sid);
        snd_mixer_selem_id_set_index(sid, 0);
        snd_mixer_selem_id_set_name(sid, "Master");

        if ((elem = snd_mixer_find_selem(h_mixer, sid)) == NULL)
            error("Cannot find simple element");

        snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &vol);
        snd_mixer_selem_get_playback_volume_range(elem, &vol_min, &vol_max);
        snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &switch_value);

        snd_mixer_close(h_mixer);

        sprintf(displayed_sound, "%s%3.0f%%", (switch_value == 1) ? "on" : "off", 100.0 * vol / vol_max);
        sleep(sound_sleep);
    }
}

/*
 * will set the XRootwindow with given string
 */
void *update_status(void * val) {
    while(1) {
        sprintf(displayed_text,
            "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
            displayed_begin,
            displayed_ip_info,
            displayed_ip_wifi,
            displayed_ip_cable,
            displayed_between,
            displayed_ram_info,
            displayed_ram,
            displayed_between,
            displayed_sound_info,
            displayed_sound,
            displayed_between,
            displayed_battery_info,
            displayed_battery,
            displayed_between,
            displayed_time_info,
            displayed_time,
            displayed_end
            );
        XStoreName(display, DefaultRootWindow(display), displayed_text);
        XSync(display, False);
        sleep(update_sleep);
    }
}

int main () {

    init();

    pthread_t time_thread;
    pthread_t battery_thread;
    pthread_t ram_thread;
    pthread_t ip_thread;
    pthread_t sound_thread;
    pthread_t status_thread;

    if( pthread_create( &time_thread,    NULL, &update_time,    NULL) != 0)
        error("konnte time_thread nicht erzeugen\n");

    if( pthread_create( &battery_thread, NULL, &update_battery, NULL) != 0)
        error("konnte batter_thread nicht erzeugen\n");

    if( pthread_create( &ram_thread,     NULL, &update_ram,     NULL) != 0)
        error("konnte ram_threadnicht erzeugen\n");

    if( pthread_create( &ip_thread,      NULL, &update_ip,      NULL) != 0)
        error("konnte ip_thread nicht erzeugen\n");

    if( pthread_create( &sound_thread,   NULL, &update_sound,   NULL) != 0)
        error("konnte sound_thread nicht erzeugen\n");

    if( pthread_create( &status_thread,  NULL, &update_status,  NULL) != 0)
        error("konnte status_thread nicht erzeugen\n");

    pthread_join( time_thread,    NULL);
    pthread_join( battery_thread, NULL);
    pthread_join( ram_thread,     NULL);
    pthread_join( ip_thread,      NULL);
    pthread_join( sound_thread,   NULL);
    pthread_join( status_thread,  NULL);

    XCloseDisplay(display);
    return 0;
}
