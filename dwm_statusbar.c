/* dwm_statusbar
 * author: d4ryus - https://github.com/d4ryus/
 * inspired by several ideas from dwm.suckless.org
 * vim:ts=4:sw=4:aifoldmethod=syntax:
 */

#include "dwm_statusbar.h"

void error(char*);
void *update_time(void*);
void *update_battery(void*);
void *update_ram(void*);
void *update_sound(void*);
void *update_loadavg(void*);
void *update_netdev(void*);
void *update_stat(void*);
void *update_status(void*);
int pthread_setname_np(pthread_t, char*);

void
error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

void
*update_time(void * val)
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

void
*update_battery(void * val)
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
            fscanf(fd_status, "%s", energy_status);
            fclose(fd_status);
        }

        battery_percentage = (int)((((float)energy_now) / ((float)energy_full) * 100.0) + 0.5);
        sprintf(displayed_battery, "%s %d%s", energy_status, battery_percentage, "%");
        sleep(battery_sleep);
    }
}

void
*update_ram(void * val)
{
    usleep(rand() % 100000);
    int  ram[5];
    char buffer[1024];
    FILE *fp;
    while(1)
    {
        fp = fopen(RAM, "r");
        if(fp == NULL)
        {
            sprintf(displayed_ram, "ram error");
            sleep(ram_sleep);
            continue;
        }

        int i = 0;
        for(;fgets(buffer, 1024, fp);i++)
        {
            if(i==5) break;
            sscanf(buffer, "%*s %d %*s", &ram[i]);
        }
        fclose(fp);

        ram[2] = ram[0] - (ram[1] + ram[3] + ram[4]); /* ram used */

        sprintf(displayed_ram, "%d mb", ram[2] / 1024);

        sleep(ram_sleep);
    }
}

void
*update_sound(void * val)
{
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
            sprintf(displayed_sound, "error");
            sleep(sound_sleep);
            continue;
        }

        if (snd_mixer_attach(h_mixer, "default") < 0)
        {
            sprintf(displayed_sound, "error");
            sleep(sound_sleep);
            continue;
        }

        if (snd_mixer_selem_register(h_mixer, NULL, NULL) < 0)
        {
            sprintf(displayed_sound, "error");
            sleep(sound_sleep);
            continue;
        }

        if (snd_mixer_load(h_mixer) < 0)
        {
            sprintf(displayed_sound, "error");
            sleep(sound_sleep);
            continue;
        }

        snd_mixer_selem_id_alloca(&sid);
        snd_mixer_selem_id_set_index(sid, 0);
        snd_mixer_selem_id_set_name(sid, "Master");

        if ((elem = snd_mixer_find_selem(h_mixer, sid)) == NULL)
        {
            sprintf(displayed_sound, "error");
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
            sprintf(displayed_sound, "%s%3.0f%%",
                                   (switch_value == 1) ? "on" : "off", volume);
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
            sprintf(displayed_sound, "%s%3.0f%%", (switch_value == 1) ? "on" : "off", volume);
            fast_refresh_flag = 0;
            sleep(sound_sleep);
        }
    }
}

void
*update_loadavg(void * val)
{
    usleep(rand() % 100000);
    double avg[3];
    while(1)
    {
        if(getloadavg(avg, 3) < 0)
        {
            sprintf(displayed_loadavg, "avg's error");
            sleep(loadavg_sleep);
            continue;
        }
        sprintf(displayed_loadavg, "%.2f %.2f %.2f", avg[0], avg[1], avg[2]);
        sleep(loadavg_sleep);
    }
}

void
*update_netdev(void * val)
{
    usleep(rand() % 100000);
    FILE* fp;

    unsigned int up_b4[3];
    unsigned int down_b4[3];
    unsigned int up = 0;
    unsigned int down = 0;
    unsigned int received;
    unsigned int send;
    int count;
    int empty_count;
    char buffer[1024];
    char interface[12];
    while(1)
    {
        fp = fopen(NETDEV, "r");
        if(fp == NULL)
        {
            sprintf(displayed_netdev, "netdev_error");
            sleep(1);
            continue;
        }
        fgets(buffer, 1024, fp);
        fgets(buffer, 1024, fp);
        count = 0;
        empty_count = 0;
        while(fgets(buffer, 1024, fp))
        {
            sscanf(buffer, "%s %u %*u %*u %*u %*u %*u %*u %*u %u",
                                                  interface, &received, &send);
            up = send - up_b4[count];
            down = received - down_b4[count];
            up_b4[count] = send;
            down_b4[count] = received;
            if((up == 0) || (down == 0))
            {
                count++;
                empty_count++;
                continue;
            }
            sprintf(displayed_netdev, "%s %u/%u kBs",
                                  interface, up/1000, down/1000);
            count++;
        }
        fclose(fp);
        if(empty_count == count)
            sprintf(displayed_netdev, "-/- kBs");

        sleep(1);
    }
}

void
*update_stat(void * val)
{
    usleep(rand() % 100000);

    FILE*        fp;
    char         buffer[1024];
    /* saves up user/nice/system/idle/total/usage/usageb4/totalb4 */
    unsigned int cpu[CPU_CORES +1][8];
    double       load[CPU_CORES +1];

    while(1)
    {
        fp = fopen(STAT, "r");
        if(fp == NULL)
        {
            sprintf(displayed_stat, "stat_error");
            sleep(stat_sleep);
            continue;
        }

        /* read from file into cpu array */
        int i;
        for(i = 0; i < CPU_CORES + 1; i++)
        {
            fgets(buffer, 1024, fp);
            sscanf(buffer, "%*s %u %u %u %u",
                               &cpu[i][0], &cpu[i][1], &cpu[i][2], &cpu[i][3]);
        }
        fclose(fp);

        /* calculate total and used cpu */
        for(i = 0; i < CPU_CORES + 1; i++)
        {
            cpu[i][4] = cpu[i][0] + cpu[i][1] + cpu[i][2] + cpu[i][3];
            cpu[i][5] = cpu[i][4] - cpu[i][3];
        }

        /* calculate percentage usage */
        for(i = 0; i < CPU_CORES + 1; i++)
        {
            load[i] = ((double)( cpu[i][5] - cpu[i][6] )+0.5) /
                      ((double)( cpu[i][4] - cpu[i][7] )+0.5) * 100;
            cpu[i][6] = cpu[i][5];
            cpu[i][7] = cpu[i][4];
        }

        sprintf(displayed_stat, "%3.0f|%3.0f%3.0f%3.0f%3.0f",
                   load[0], load[1], load[2], load[3], load[4]);

        sleep(stat_sleep);
    }
}

void
*update_status(void * val)
{
    Display* display;
    if (!(display = XOpenDisplay(NULL)))
        error("Cannot open display");

    while(1)
    {
        sprintf(displayed_text,
                "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                displayed_begin,
                displayed_netdev_info,  displayed_netdev,  displayed_between,
                displayed_stat_info,    displayed_stat,    displayed_between,
                displayed_loadavg_info, displayed_loadavg, displayed_between,
                displayed_ram_info,     displayed_ram,     displayed_between,
                displayed_sound_info,   displayed_sound,   displayed_between,
                displayed_battery_info, displayed_battery, displayed_between,
                displayed_time_info,    displayed_time,
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

int
main ()
{
    pthread_t time_thread;
    pthread_t battery_thread;
    pthread_t ram_thread;
    pthread_t sound_thread;
    pthread_t status_thread;
    pthread_t loadavg_thread;
    pthread_t netdev_thread;
    pthread_t stat_thread;

    if( pthread_create( &time_thread,    NULL, &update_time,     NULL) != 0)
        error("couldn't create time_thread\n");

    if( pthread_create( &battery_thread, NULL, &update_battery,  NULL) != 0)
        error("couldn't create batter_thread\n");

    if( pthread_create( &ram_thread,     NULL, &update_ram,      NULL) != 0)
        error("couldn't create ram_threadnichtn");

    if( pthread_create( &sound_thread,   NULL, &update_sound,    NULL) != 0)
        error("couldn't create sound_thread\n");

    if( pthread_create( &status_thread,  NULL, &update_status,   NULL) != 0)
        error("couldn't create status_thread\n");

    if( pthread_create( &loadavg_thread, NULL, &update_loadavg,  NULL) != 0)
        error("couldn't create loadavg_thread\n");

    if( pthread_create( &netdev_thread,  NULL, &update_netdev,   NULL) != 0)
        error("couldn't create netdev_thread\n");

    if( pthread_create( &stat_thread,    NULL, &update_stat,     NULL) != 0)
        error("couldn't create stat_thread\n");

    /* assigning a name to each thread, not needed but usefull for debugging */
    pthread_setname_np(time_thread,    "update_time");
    pthread_setname_np(battery_thread, "update_battery");
    pthread_setname_np(ram_thread,     "update_ram");
    pthread_setname_np(sound_thread,   "update_sound");
    pthread_setname_np(status_thread,  "update_status");
    pthread_setname_np(loadavg_thread, "update_loadavg");
    pthread_setname_np(netdev_thread,  "update_netdev");
    pthread_setname_np(stat_thread,    "update_stat");

    pthread_join(time_thread,    NULL);
    pthread_join(battery_thread, NULL);
    pthread_join(ram_thread,     NULL);
    pthread_join(sound_thread,   NULL);
    pthread_join(status_thread,  NULL);
    pthread_join(loadavg_thread, NULL);
    pthread_join(netdev_thread,  NULL);
    pthread_join(stat_thread,    NULL);

    return 0;
}
