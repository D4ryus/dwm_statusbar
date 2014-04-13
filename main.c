/* dwm_statusbar
 * author: d4ryus - https://github.com/d4ryus/
 * inspired by several ideas from dwm.suckless.org
 * vim:ts=4:sw=4:ai:
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>

static const char* BATTERY_STATUS = "/sys/class/power_supply/BAT0/status";
static const char* BATTERY_FULL   = "/sys/class/power_supply/BAT0/energy_full";
static const char* BATTERY_NOW    = "/sys/class/power_supply/BAT0/energy_now";
static const char* RAM            = "/proc/meminfo";

static struct tm *timeinfo;
static Display *display;
static time_t rawtime;

static char displayed_text[256];

static int  battery_percentage;
static int  energy_full;
static int  energy_now;
static char energy_status[12];

static char TIME[17];

static int ram_total;
static int ram_used;
static int ram_free;

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
    FILE *fd;
    fd = fopen(BATTERY_FULL, "r");
    if(fd == NULL) {
        error("Error opening energy_full");
    }
    fscanf(fd, "%d", &energy_full);
    fclose(fd);
}

void update_time() {
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strncpy(TIME, asctime(timeinfo), 16);
    //sleep(60);
}

void update_battery() {
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
    //sleep(60);
}

void update_ram() {
    char buffer[64];
}

/*
 * will set the XRootwindow with given string
 */
void set_status(char *str) {
    XStoreName(display, DefaultRootWindow(display), str);
    XSync(display, False);
}

int main () {

    init();

    update_time();
    update_battery();

    sprintf(displayed_text, "[bat: %s %d%] [%s]", energy_status, battery_percentage, TIME);

    //printf("[bat: %s %d%] [%s]", energy_status, battery_percentage, TIME);
    set_status(displayed_text);


    XCloseDisplay(display);
}
