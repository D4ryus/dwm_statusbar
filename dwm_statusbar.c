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

int print_only_flag = 0;

void
error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

void*
update_netmsg(Info* st)
{
    usleep(rand() % 100000);

    int    my_socket;
    int    port;
    int    connected_socket;
    int    client_length;
    char   buffer[MSG_LENGTH];
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    char*  new_text;
    char*  old_text;
    int    size;

    my_socket = socket(AF_INET, SOCK_STREAM, 0);

    if( my_socket < 0 )
        error("could not create my_socket");

    bzero((char *) &server_addr, sizeof(server_addr));
    port = atoi(PORT);

    server_addr.sin_port        = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_family      = AF_INET;

    if( bind(my_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 )
        error("could not bind");

    listen(my_socket, 5);
    client_length = sizeof(client_addr);
    connected_socket = accept(my_socket, (struct sockaddr *) &client_addr,
            (socklen_t *) &client_length);
    while(connected_socket)
    {
        bzero(buffer,MSG_LENGTH);
        if( read(connected_socket, buffer, MSG_LENGTH - 1) < 0)
            error("could not read");

        if(buffer[0] == '0')
        {
            old_text = st->text;
            st->text = NULL;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
        }
        else
        {
            size = strlen(buffer) + 1;
            new_text = malloc(sizeof(char) * size);
            bzero(new_text, size);
            strncpy(new_text, buffer, size);
            old_text = st->text;
            st->text = new_text;
            if(old_text != NULL)
            {
                free(old_text);
                old_text = NULL;
            }
        }
        if( write(connected_socket, "OK", 2) < 0)
            error("could not write");

        connected_socket = accept( my_socket, (struct sockaddr *) &client_addr,
                (socklen_t *) &client_length);
    }
    close(connected_socket);
    close(my_socket);
    error("error on accept");
    return 0;
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
    char  buff[4];
    char* new_text;
    char* old_text;
    int   size;

    int i;
    int ii;
    for (i = 0; i < CPU_CORES +1; i++)
    {
        load[i] = 0.0;
        for (ii = 0; ii < 8; ii++)
            cpu[i][ii] = 0;
    }

    while(1)
    {
        fp = fopen(STAT, "r");
        if(fp == NULL)
        {
            size = 11;
            new_text = malloc(sizeof(char) * size);
            bzero(new_text, size);
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
        //int i;
        for(i = 0; i < CPU_CORES + 1; i++)
        {
            bzero(buffer, 1024);
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

        size = 5 + (CPU_CORES * 3);
        new_text = malloc(sizeof(char) * size);
        bzero(new_text, size);
        for (i = 0; i < CPU_CORES+1; i++)
        {
            bzero(buff, 4);
            sprintf(buff, "%3.0f", load[i]);
            strncat(new_text, buff, 3);
            if(i == 0)
                strncat(new_text, "|", 1);
        }
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

    unsigned int up_b4[NETDEVCOUNT];
    unsigned int down_b4[NETDEVCOUNT];
    unsigned int up         = 0;
    unsigned int down       = 0;
    unsigned int received   = 0;
    unsigned int send       = 0;
    int count;
    int empty_count;
    int bsize = 512;
    char buffer[bsize];
    int isize = 12;
    char interface[isize];
    char* new_text;
    char* old_text;
    int size;

    int i;
    for (i = 0; i < NETDEVCOUNT; i++)
    {
        up_b4[i]   = 0;
        down_b4[i] = 0;
    }

    while(1)
    {
        fp = fopen(NETDEV, "r");
        if(fp == NULL)
        {
            size = 13;
            new_text = malloc(sizeof(char) * size);
            bzero(new_text, size);
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
        fgets(buffer, bsize, fp);
        fgets(buffer, bsize, fp);
        count = 0;
        empty_count = 0;
        bzero(buffer, bsize);
        bzero(interface, isize);
        while(fgets(buffer, bsize, fp))
        {
            bzero(interface, isize);
            sscanf(buffer, "%11s %u %*u %*u %*u %*u %*u %*u %*u %u",
                                                 interface, &received, &send);
            bzero(buffer, bsize);
            up   = send     - up_b4[count];
            down = received - down_b4[count];
            up_b4[count]   = send;
            down_b4[count] = received;
            if((up == 0) && (down == 0))
            {
                count++;
                empty_count++;
                continue;
            }
            size = 32;
            new_text = malloc(sizeof(char) * size);
            bzero(new_text, size);
            sprintf(new_text, "%s %u/%u kBs", interface, up/1024, down/1024);
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
            size = 8;
            new_text = malloc(sizeof(char) * size);
            bzero(new_text, size);
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
    double avg[3] = {0.0, 0.0, 0.0};
    char* new_text;
    char* old_text;
    int size;
    while(1)
    {
        if(getloadavg(avg, 3) < 0)
        {
            size = 12;
            new_text = malloc(sizeof(char) * size);
            bzero(new_text, size);
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
        size = 15;
        new_text = malloc(sizeof(char) * size);
        bzero(new_text, size);
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
            bzero(new_text, size);
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
        bzero(new_text, size);
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
    long  vol;
    long  vol_min;
    long  vol_max;
    float volume;
    int   switch_value;
    snd_mixer_t          *h_mixer;
    snd_mixer_selem_id_t *sid;
    snd_mixer_elem_t     *elem;
    char* new_text;
    char* old_text;
    int   size;

    while(1)
    {
        if (    ( snd_mixer_open          (&h_mixer, 1         ) < 0 )
             || ( snd_mixer_attach        ( h_mixer, "default" ) < 0 )
             || ( snd_mixer_selem_register( h_mixer, NULL, NULL) < 0 )
             || ( snd_mixer_load          ( h_mixer            ) < 0 ) )
        {
            size = 16;
            new_text = malloc(sizeof(char) * size);
            bzero(new_text, size);
            strncpy(new_text, "snd_mixer error", size);
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
            bzero(new_text, size);
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

        snd_mixer_selem_get_playback_volume      (elem, SND_MIXER_SCHN_FRONT_LEFT, &vol);
        snd_mixer_selem_get_playback_volume_range(elem, &vol_min, &vol_max);
        snd_mixer_selem_get_playback_switch      (elem, SND_MIXER_SCHN_FRONT_LEFT, &switch_value);

        snd_mixer_close(h_mixer);

        volume = 100.0 * vol / vol_max;

        size = 8;
        new_text = malloc(sizeof(char) * size);
        bzero(new_text, size);
        sprintf(new_text, "%s %.0f%%", (switch_value == 1) ? "on" : "off", volume);
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
            bzero(new_text, size);
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
            bzero(new_text, size);
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
            bzero(new_text, size);
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
    int    size = 17;
    char*  new_text;
    char*  old_text;
    while(1)
    {
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        new_text = malloc(sizeof(char) * (size));
        bzero(new_text, size);
        strncpy(new_text, asctime(timeinfo), size-1);
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
    if(!print_only_flag)
    {
        if (!(display = XOpenDisplay(NULL)))
            error("Cannot open display");
        XStoreName(display, DefaultRootWindow(display), "init statusbar...");
        XSync(display, False);
    }

    sleep(2);

    int bsize = 512;
    char displayed_text[bsize];
    int i;
    while(1)
    {
        bzero(displayed_text, bsize);
        for (i = 0; i < sizeof(infos)/sizeof(Info); i++)
        {
            if((infos[i].text != NULL) && (infos[i].text[0] != '\0'))
            {
                if((infos[i].before != NULL) && (infos[i].before[0] != '\0'))
                    strncat(displayed_text, infos[i].before, strlen(infos[i].before));

                strncat(displayed_text, infos[i].text, strlen(infos[i].text));

                if((infos[i].after != NULL)  && (infos[i].after[0] != '\0'))
                    strncat(displayed_text, infos[i].after, strlen(infos[i].after));
            }
        }
        if(!print_only_flag)
        {
            XStoreName(display, DefaultRootWindow(display), displayed_text);
            XSync(display, False);
        }
        else
        {
            printf("%s\n", displayed_text);
        }
        sleep(REFRESH);
    }
    if(!print_only_flag)
        XCloseDisplay(display);
}

int
main(int argc, const char *argv[])
{
    if(argc > 1)
    {
        if(    (argv[1][0] == '-')
            && (argv[1][1] == 't') )
            print_only_flag = 1;
    }
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
