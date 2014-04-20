/* dwm_statusbar
 * author: d4ryus - https://github.com/d4ryus/
 * inspired by several ideas from dwm.suckless.org
 * also thanks to Yu-Jie Lin, most of the alsa code is from him.
 * vim:ts=4:sw=4:ai:
 */
#include "dwm_statusbar.h"

void error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

void *update_time(void * val)
{
    usleep(rand() % 100000);
    struct tm*  timeinfo;
    time_t      rawtime;
    while(1)
    {
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strncpy(displayed_time, asctime(timeinfo), 16);
        sleep(time_sleep);
    }
}

void *update_battery(void * val)
{
    usleep(rand() % 100000);
    FILE* fd_now;
    FILE* fd_full;
    FILE* fd_status;
    int  energy_now;
    int  energy_full;
    char energy_status[12];
    int  battery_percentage;
    while(1)
    {
        fd_now    = fopen(BATTERY_NOW, "r");
        if(fd_now == NULL)
        {
            sprintf(displayed_battery, "on AC");
            sleep(battery_sleep);
            continue;
        } else {
            fscanf(fd_now, "%d", &energy_now);
            fclose(fd_now);
        }

        fd_full   = fopen(BATTERY_FULL, "r");
        if(fd_full == NULL)
        {
            sprintf(displayed_battery, "on AC");
            sleep(battery_sleep);
            continue;
        } else {
            fscanf(fd_full, "%d", &energy_full);
            fclose(fd_full);
        }

        fd_status = fopen(BATTERY_STATUS, "r");
        if(fd_status == NULL)
        {
            sprintf(displayed_battery, "on AC");
            sleep(battery_sleep);
            continue;
        } else {
            fscanf(fd_status, "%s", &energy_status);
            fclose(fd_status);
        }

        battery_percentage = (int)((((float)energy_now) / ((float)energy_full) * 100.0) + 0.5);
        sprintf(displayed_battery, "%s %d%s", energy_status, battery_percentage, "%");
        sleep(battery_sleep);
    }
}

void *update_ram(void * val)
{
    usleep(rand() % 100000);
    int  ram_total;
    int  ram_available;
    int  ram_free;
    int  ram_used;
    char buffer[64];
    FILE *fd;
    while(1)
    {
        fd = fopen(RAM, "r");
        if(fd == NULL)
        {
            sprintf(displayed_ram, " coudln't read file :( ");
            sleep(ram_sleep);
            continue;
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

void *update_ip(void * val)
{
    usleep(rand() % 100000);
    char         wifi_ip[17];
    char         cable_ip[17];
    int wifi_connected  = 0; // just boolean flag
    int cable_connected = 0; // just boolean flag
    int          fd;
    struct ifreq ifr_wifi;
    struct ifreq ifr_cable;
    while(1)
    {
        //Type of address to retrieve - IPv4 IP address
        ifr_wifi.ifr_addr.sa_family  = AF_INET;
        ifr_cable.ifr_addr.sa_family = AF_INET;

        //Copy the interface name in the ifreq structure
        strncpy(ifr_wifi.ifr_name,  WIFI_INTERFACE,  IFNAMSIZ-1);
        strncpy(ifr_cable.ifr_name, CABLE_INTERFACE, IFNAMSIZ-1);

        fd = socket(AF_INET, SOCK_DGRAM, 0);
        ioctl(fd, SIOCGIFADDR, &ifr_wifi);
        close(fd);
        fd = socket(AF_INET, SOCK_DGRAM, 0);
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
                ( w1 == 118 && w2 == 97 && w3 == 105 && w4 == 108 ) )
        {
            wifi_connected = 0;
        } else {
            wifi_connected = 1;
        }

        if( c1 == 0 && c2 == 0 && c3 == 0 && c4 == 0 )
        {
            cable_connected = 0;
        } else {
            cable_connected = 1;
        }


        if        ( wifi_connected &&  cable_connected )
        {

            sprintf(displayed_ip_wifi, "%s: %s", WIFI_INTERFACE, wifi_ip);
            sprintf(displayed_ip_cable, " %s: %s", CABLE_INTERFACE, cable_ip);

        } else if ( wifi_connected && !cable_connected )
        {

            sprintf(displayed_ip_wifi, "%s: %s", WIFI_INTERFACE, wifi_ip);
            displayed_ip_cable[0] = '\0'; // no cable assigned

        } else if (!wifi_connected &&  cable_connected )
        {

            displayed_ip_wifi[0] = '\0'; // no wifi assigned
            sprintf(displayed_ip_cable, "%s: %s", CABLE_INTERFACE, cable_ip);

        } else if (!wifi_connected && !cable_connected )
        {

            strncpy(displayed_ip_wifi, "not connected", 13);
            displayed_ip_cable[0] = '\0'; // no cable assigned

        }
        sleep(ip_sleep);
    }
}

void *update_sound(void * val)
{
    int  err;
    int  switch_value;
    int  switch_value_backup = -1;
    int  count = 0;
    long vol;
    long vol_min;
    long vol_max;
    float volume;
    float volume_backup = -1.0;
    snd_mixer_t          *h_mixer;
    snd_mixer_selem_id_t *sid;
    snd_mixer_elem_t     *elem;
    while(1)
    {
        if (snd_mixer_open(&h_mixer, 1) < 0)
        {
            sprintf(displayed_sound, "error on snd_mixer_open");
            sleep(sound_sleep);
            continue;
        }

        if (snd_mixer_attach(h_mixer, "default") < 0)
        {
            sprintf(displayed_sound, "error on snd_mixer_attach");
            sleep(sound_sleep);
            continue;
        }

        if (snd_mixer_selem_register(h_mixer, NULL, NULL) < 0)
        {
            sprintf(displayed_sound, "error on snd_mixer_selem_register");
            sleep(sound_sleep);
            continue;
        }

        if (snd_mixer_load(h_mixer) < 0)
        {
            sprintf(displayed_sound, "error on snd_mixer_load");
            sleep(sound_sleep);
            continue;
        }

        snd_mixer_selem_id_alloca(&sid);
        snd_mixer_selem_id_set_index(sid, 0);
        snd_mixer_selem_id_set_name(sid, "Master");

        if ((elem = snd_mixer_find_selem(h_mixer, sid)) == NULL)
        {
            sprintf(displayed_sound, "error on snd_mixer_find_selem");
            sleep(sound_sleep);
            continue;
        }

        snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &vol);
        snd_mixer_selem_get_playback_volume_range(elem, &vol_min, &vol_max);
        snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &switch_value);

        snd_mixer_close(h_mixer);

        volume = 100.0 * vol / vol_max;

        if((volume != volume_backup) || (switch_value != switch_value_backup))
        {
            count = 10;
            fast_refresh_flag = 1;
            sprintf(displayed_sound, "%s%3.0f%%", (switch_value == 1) ? "on" : "off", volume);
        } else {
            count--;
        }

        volume_backup = volume;
        switch_value_backup = switch_value;

        if (count > 0)
        {
            usleep(fast_refresh);
            continue;
        } else {
            fast_refresh_flag = 0;
            sleep(sound_sleep);
        }
    }
}

void *update_loadavg(void * val)
{
    usleep(rand() % 100000);
    float  loadavg_1min;
    float  loadavg_5min;
    float  loadavg_10min;
    FILE *fd;
    while(1)
    {
        fd = fopen(LOADAVG, "r");
        if(fd == NULL)
        {
            sprintf(displayed_loadavg, " coudln't read file :( ");
            sleep(loadavg_sleep);
            continue;
        }
        fscanf(fd, "%f %f %f", &loadavg_1min, &loadavg_5min,  &loadavg_10min);
        fclose(fd);

        sprintf(displayed_loadavg, "%.2f %.2f %.2f", loadavg_1min,
                                                     loadavg_5min,
                                                     loadavg_10min);
        sleep(loadavg_sleep);
    }
}

void *update_status(void * val)
{
    Display* display;
    if (!(display = XOpenDisplay(NULL)))
        error("Cannot open display");

    while(1)
    {
        sprintf(displayed_text,
                "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                displayed_begin,
                displayed_ip_info,
                displayed_ip_wifi,
                displayed_ip_cable,
                displayed_between,
                displayed_loadavg_info,
                displayed_loadavg,
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

        if( fast_refresh_flag == 1 )
        {
            usleep(fast_refresh);
            continue;
        } else {
            sleep(update_sleep);
        }
    }
    XCloseDisplay(display);
}

int main ()
{

    pthread_t time_thread;
    pthread_t battery_thread;
    pthread_t ram_thread;
    pthread_t ip_thread;
    pthread_t sound_thread;
    pthread_t status_thread;
    pthread_t loadavg_thread;

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

    if( pthread_create( &loadavg_thread,  NULL, &update_loadavg,  NULL) != 0)
        error("konnte loadavg_thread nicht erzeugen\n");

    /* assigning a name to each thread, not needed but usefull for debugging */
    pthread_setname_np(time_thread,    "update_time");
    pthread_setname_np(battery_thread, "update_battery");
    pthread_setname_np(ram_thread,     "update_ram");
    pthread_setname_np(ip_thread,      "update_ip");
    pthread_setname_np(sound_thread,   "update_sound");
    pthread_setname_np(status_thread,  "update_status");
    pthread_setname_np(loadavg_thread, "update_loadavg");

    pthread_join( time_thread,    NULL);
    pthread_join( battery_thread, NULL);
    pthread_join( ram_thread,     NULL);
    pthread_join( ip_thread,      NULL);
    pthread_join( sound_thread,   NULL);
    pthread_join( status_thread,  NULL);
    pthread_join( loadavg_thread, NULL);

    return 0;
}
