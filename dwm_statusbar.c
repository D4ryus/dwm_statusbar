/* dwm_statusbar
 * author: d4ryus - https://github.com/d4ryus/
 * inspired by several ideas from dwm.suckless.org
 * file: dwm_statusbar.c
 * vim:ts=4:sw=4:ai:foldmethod=syntax:
 */

#include "dwm_statusbar.h"
#include "config.h"

void error(char*);
void *update_status();
int pthread_setname_np(pthread_t, char*);

void
error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

void*
update_stat(Info* st)
{
    usleep(rand() % 100000);

    FILE*        fp;
    char         buffer[1024];
    /* saves up user/nice/system/idle/total/usage/usageb4/totalb4 */
    unsigned int cpu[CPU_CORES +1][8];
    double       load[CPU_CORES +1];
    char* new_text;
    char* old_text;
    int   size;

    while(1)
    {
        fp = fopen(STAT, "r");
        if(fp == NULL)
        {
            size = 11;
            new_text = malloc(sizeof(char) * size);
            strncpy(new_text, "stat_error", size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            sleep(st->sleep);
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

        size = 17;
        new_text = malloc(sizeof(char) * size);
        sprintf(new_text, "%3.0f|%3.0f%3.0f%3.0f%3.0f",
                load[0], load[1], load[2], load[3], load[4]);
        old_text = st->text;
        st->text = new_text;
        if(old_text != NULL)
        {
            free(old_text);
            old_text = NULL;
        }
        sleep(st->sleep);
    }
}

void*
update_netdev(Info* st)
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
    char* new_text;
    char* old_text;
    int size;
    while(1)
    {
        fp = fopen(NETDEV, "r");
        if(fp == NULL)
        {
            size = 13;
            new_text = malloc(sizeof(char) * size);
            strncpy(new_text, "netdev_error", size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            sleep(st->sleep);
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
            up   = send     - up_b4[count];
            down = received - down_b4[count];
            up_b4[count]   = send;
            down_b4[count] = received;
            if((up == 0) || (down == 0))
            {
                count++;
                empty_count++;
                continue;
            }
            size = 24;
            new_text = malloc(sizeof(char) * size);
            sprintf(new_text, "%s %u/%u kBs", interface, up/1000, down/1000);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            count++;
        }
        fclose(fp);
        if(empty_count == count)
        {
            size = 7;
            new_text = malloc(sizeof(char) * size);
            sprintf(new_text, "-/- kBs");
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
        }
        sleep(st->sleep);
    }
}

void*
update_loadavg(Info* st)
{
    usleep(rand() % 100000);
    double avg[3];
    char* new_text;
    char* old_text;
    int size;
    while(1)
    {
        if(getloadavg(avg, 3) < 0)
        {
            size = 12;
            new_text = malloc(sizeof(char) * size);
            strncpy(new_text, "avg's error", size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            sleep(st->sleep);
            continue;
        }
        size = 12;
        new_text = malloc(sizeof(char) * size);
        sprintf(new_text, "%.2f %.2f %.2f", avg[0], avg[1], avg[2]);
        old_text = st->text;
        st->text = new_text;
        if(old_text != NULL)
        {
            free(old_text);
            old_text = NULL;
        }
        sleep(st->sleep);
    }
}

void*
update_ram(Info* st)
{
    usleep(rand() % 100000);
    int  ram[5];
    char buffer[1024];
    FILE *fp;
    char* new_text;
    char* old_text;
    int size;
    while(1)
    {
        fp = fopen(RAM, "r");
        if(fp == NULL)
        {
            size = 11;
            new_text = malloc(sizeof(char) * size);
            strncpy(new_text, "ram error", size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            sleep(st->sleep);
            continue;
        }

        int i;
        for(i = 0; fgets(buffer, 1024, fp); i++)
        {
            if( i == 5 )
                break;
            sscanf(buffer, "%*s %d %*s", &ram[i]);
        }
        fclose(fp);

        ram[2] = ram[0] - (ram[1] + ram[3] + ram[4]); // ram used

        size = 8;
        new_text = malloc(sizeof(char) * size);
        sprintf(new_text, "%d mb", ram[2] / 1024);
        old_text = st->text;
        st->text = new_text;
        if(old_text != NULL)
        {
            free(old_text);
            old_text = NULL;
        }
        sleep(st->sleep);
    }
}

void*
update_sound(Info* st)
{
    long vol;
    long vol_min;
    long vol_max;
    float volume;
    int switch_value;
    snd_mixer_t          *h_mixer;
    snd_mixer_selem_id_t *sid;
    snd_mixer_elem_t     *elem;
    char* new_text;
    char* old_text;
    int   size;
    while(1)
    {
        if (snd_mixer_open(&h_mixer, 1) < 0)
        {
            size = 6;
            new_text = malloc(sizeof(char) * size);
            strncpy(new_text, "error", size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            sleep(st->sleep);
            continue;
        }

        if (snd_mixer_attach(h_mixer, "default") < 0)
        {
            size = 6;
            new_text = malloc(sizeof(char) * size);
            strncpy(new_text, "error", size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            sleep(st->sleep);
            continue;
        }

        if (snd_mixer_selem_register(h_mixer, NULL, NULL) < 0)
        {
            size = 6;
            new_text = malloc(sizeof(char) * size);
            strncpy(new_text, "error", size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            sleep(st->sleep);
            continue;
        }

        if (snd_mixer_load(h_mixer) < 0)
        {
            size = 6;
            new_text = malloc(sizeof(char) * size);
            strncpy(new_text, "error", size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            sleep(st->sleep);
            continue;
        }

        snd_mixer_selem_id_alloca(&sid);
        snd_mixer_selem_id_set_index(sid, 0);
        snd_mixer_selem_id_set_name(sid, "Master");

        if ((elem = snd_mixer_find_selem(h_mixer, sid)) == NULL)
        {
            size = 6;
            new_text = malloc(sizeof(char) * size);
            strncpy(new_text, "error", size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            sleep(st->sleep);
            continue;
        }

        snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &vol);
        snd_mixer_selem_get_playback_volume_range(elem, &vol_min, &vol_max);
        snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &switch_value);

        snd_mixer_close(h_mixer);

        volume = 100.0 * vol / vol_max;

        size = 8;
        new_text = malloc(sizeof(char) * size);
        sprintf(new_text, "%s %3.0f%%", (switch_value == 1) ? "on" : "off", volume);
        old_text = st->text;
        st->text = new_text;
        if(old_text != NULL)
        {
            free(old_text);
            old_text = NULL;
        }
        sleep(st->sleep);
    }
}

void*
update_battery(Info* st)
{
    usleep(rand() % 100000);
    FILE* fd_now;
    FILE* fd_full;
    FILE* fd_status;
    int   energy_now;
    int   energy_full;
    char  energy_status[12];
    int   battery_percentage;
    int   size;
    char* old_text;
    char* new_text;
    while(1)
    {
        fd_now    = fopen(BATTERY_NOW, "r");
        if(fd_now   == NULL)
        {
            size = 6;
            new_text = malloc(sizeof(char)*size);
            strncpy(new_text, "on AC", size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            sleep(st->sleep);
            continue;
        }
        else
        {
            fscanf(fd_now, "%d", &energy_now);
            fclose(fd_now);
        }

        fd_full   = fopen(BATTERY_FULL, "r");
        if(fd_full   == NULL)
        {
            size = 6;
            new_text = malloc(sizeof(char)*size);
            strncpy(new_text, "on AC", size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            sleep(st->sleep);
            continue;
        }
        else
        {
            fscanf(fd_full, "%d", &energy_full);
            fclose(fd_full);
        }

        fd_status = fopen(BATTERY_STATUS, "r");
        if(fd_status == NULL)
        {
            size = 6;
            new_text = malloc(sizeof(char)*size);
            strncpy(new_text, "on AC", size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
            sleep(st->sleep);
            continue;
        }
        else
        {
            fscanf(fd_status, "%s", energy_status);
            fclose(fd_status);
        }

        battery_percentage = (int)((((float)energy_now) / ((float)energy_full) * 100.0) + 0.5);
        new_text = malloc(sizeof(energy_status) + sizeof(char) * 6);
        sprintf(new_text, "%s %d%s", energy_status, battery_percentage, "%");
        old_text = st->text;
        st->text = new_text;
        if(old_text != NULL)
        {
            free(old_text);
            old_text = NULL;
        }
        sleep(st->sleep);
    }
}

void*
update_time(Info* st)
{
    usleep(rand() % 100000);
    struct tm*  timeinfo;
    time_t rawtime;
    int    size = 16;
    char*  new_text;
    char*  old_text;
    while(1)
    {
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        new_text = malloc(sizeof(char) * (size+1));
        strncpy(new_text, asctime(timeinfo), size);
        new_text[size] = '\0';
        old_text = st->text;
        st->text = new_text;
        if(old_text != NULL)
        {
            free(old_text);
            old_text = NULL;
        }
        sleep(st->sleep);
    }
}

void*
update_status()
{
    Display* display;
    if (!(display = XOpenDisplay(NULL)))
        error("Cannot open display");

    XStoreName(display, DefaultRootWindow(display), "init statusbar...");
    XSync(display, False);

    sleep(2);

    char displayed_text[512];
    int i;
    while(1)
    {
        strcpy(displayed_text, "");
        for (i = 0; i < sizeof(infos)/sizeof(Info); i++)
        {
            if(infos[i].text != NULL)
            {
                strncat( displayed_text, infos[i].before, strlen(infos[i].before) );
                strncat( displayed_text, infos[i].text,   strlen(infos[i].text  ) );
                strncat( displayed_text, infos[i].after,  strlen(infos[i].after ) );
            }
        }
        strncat(displayed_text, "", 1);
        XStoreName(display, DefaultRootWindow(display), displayed_text);
        XSync(display, False);
        sleep(REFRESH);
    }
    XCloseDisplay(display);
}

int
main ()
{
    int i;
    for (i = 0; i < sizeof(infos)/sizeof(Info); i++)
    {
        pthread_t thread;
        if( pthread_create(&thread, NULL, (void*)infos[i].fun, ((void*) &infos[i])) != 0)
            error("couldn't create thread\n");
        pthread_setname_np(thread, infos[i].name);
    }
    pthread_t thread;
    if( pthread_create(&thread, NULL, (void*)update_status, NULL) != 0)
        error("couldn't create thread\n");
    pthread_setname_np(thread, "status");

    pthread_join(thread, NULL);

    return 0;
}
