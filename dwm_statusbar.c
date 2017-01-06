/* dwm_statusbar
 * author: d4ryus - https://github.com/d4ryus/
 * inspired by several ideas from dwm.suckless.org
 * file: dwm_statusbar.c
 * vim:ts=8:sw=8:noet:
 */

#include "dwm_statusbar.h"
#include "config.h"

int print_only_flag = 0;

void
error(char *msg)
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(1);
}

void
quit(int signum)
{
	printf("terminating...\n");
	exit(0);
}

void
swap_text(struct Info *st, char *new_text)
{
	char *tmp = st->text;
	st->text = new_text;
	if (tmp != NULL) {
		free(tmp);
		tmp = NULL;
	}
}

char *
fmt_size(uint64_t size, char fmt_size[7])
{
	char *type;
	uint64_t new_size;
	uint64_t xb; /* 8xb */
	uint64_t tb; /* 8tb */
	uint64_t gb; /* 8gb */
	uint64_t mb; /* 8mb */
	uint64_t kb; /* 8kb */

	new_size = 0;

	xb = (uint64_t)1 << 53;
	tb = (uint64_t)1 << 43;
	gb = (uint64_t)1 << 33;
	mb = (uint64_t)1 << 23;
	kb = (uint64_t)1 << 13;

	if (size > xb) {
		new_size = size >> 50;
		type = "xb";
	} else if (size > tb) {
		new_size = size >> 40;
		type = "tb";
	} else if (size > gb) {
		new_size = size >> 30;
		type = "gb";
	} else if (size > mb) {
		new_size = size >> 20;
		type = "mb";
	} else if (size > kb) {
		new_size = size >> 10;
		type = "kb";
	} else {
		new_size = size;
		type = "b ";
	}

	snprintf(fmt_size, (size_t)7, "%4llu%s",
	    (long long unsigned int)new_size, type);

	return fmt_size;
}

void *
update_netmsg(struct Info *st)
{
	int my_socket;
	int connected_socket;
	char buffer[MSG_LENGTH];
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int client_length;

	size_t size;
	char *new_text;

	usleep((uint)rand() % 100000);

	if ((my_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error("could not create socket");
	}

	memset((char *) &server_addr, '\0', sizeof(server_addr));

	server_addr.sin_port = htons(atoi(PORT));
	server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server_addr.sin_family = AF_INET;

	if (bind(my_socket, (struct sockaddr *) &server_addr,
	    sizeof(server_addr)) < 0) {
		error("could not bind");
	}

	listen(my_socket, 5);
	client_length = sizeof(client_addr);
accept_new:
	connected_socket = accept(my_socket, (struct sockaddr *) &client_addr,
			       (socklen_t *) &client_length);
	while (connected_socket) {
		memset(buffer, '\0', MSG_LENGTH);
		if (read(connected_socket, buffer, MSG_LENGTH - 1) <= 0) {
			swap_text(st, NULL);
			close(connected_socket);
			goto accept_new;
		}

		size = strlen(buffer);

		if (buffer[size - 1] == '\n') {
			buffer[size - 1] = '\0';
		}

		new_text = malloc(sizeof(char) * (size + 1));
		memset(new_text, '\0', size);
		strncpy(new_text, buffer, size);
		swap_text(st, new_text);
		if (write(connected_socket, "OK", 2) < 0) {
			swap_text(st, NULL);
			close(connected_socket);
			goto accept_new;
		}
	}
	close(connected_socket);
	close(my_socket);
	error("error on accept");

	return 0;
}

void *
update_stat(struct Info *st)
{
	FILE *fp;
	/* saves up user/nice/system/idle/total/usage/usageb4/totalb4 */
	unsigned int cpu[CPU_CORES +1][8];
	double load[CPU_CORES +1];
	char *new_text;
	int i;
	int ii;

	usleep((uint)rand() % 100000);

	for (i = 0; i < CPU_CORES +1; i++) {
		load[i] = 0.0;
		for (ii = 0; ii < 8; ii++) {
			cpu[i][ii] = 0;
		}
	}

	while (1) {
		char buffer[1024];
		size_t size;
		char buff[4];

		if ((fp = fopen(STAT, "r")) == NULL) {
			new_text = malloc(sizeof(char) * 11);
			memset(new_text, '\0', 11);
			strncpy(new_text, "stat_error", 11);
			swap_text(st, new_text);
			sleep(st->sleep);

			continue;
		}

		/* read from file into cpu array */
		for (i = 0; i < CPU_CORES + 1; i++) {
			memset(buffer, '\0', 1024);
			fgets(buffer, 1024, fp);
			sscanf(buffer, "%*s %u %u %u %u",
				&cpu[i][0], &cpu[i][1], &cpu[i][2], &cpu[i][3]);
		}
		fclose(fp);

		/* calculate total and used cpu */
		for (i = 0; i < CPU_CORES + 1; i++) {
			cpu[i][4] = cpu[i][0] + cpu[i][1] + cpu[i][2]
				        + cpu[i][3];
			cpu[i][5] = cpu[i][4] - cpu[i][3];
		}

		/* calculate percentage usage */
		for (i = 0; i < CPU_CORES + 1; i++) {
			load[i] = ((double)(cpu[i][5] - cpu[i][6]) + 0.5)
				/ ((double)(cpu[i][4] - cpu[i][7]) + 0.5)
				* 100;
			cpu[i][6] = cpu[i][5];
			cpu[i][7] = cpu[i][4];
		}

		size = 5 + (CPU_CORES * 3);
		new_text = malloc(sizeof(char) * size);
		memset(new_text, '\0', size);
		for (i = 0; i < CPU_CORES+1; i++) {
			memset(buff, '\0', 4);
			sprintf(buff, "%3.0f", load[i]);
			strncat(new_text, buff, 3);
			if (i == 0) {
				strncat(new_text, "|", 1);
			}
		}
		swap_text(st, new_text);
		sleep(st->sleep);
	}
}

/*
 * the displayed interface string will look like this:
 * enp0s25: 1024mb/1024gb\0
 * |       ||   | ||   | `------------------------- string termination (1 char)
 * |       ||   | ||   `-------------------- integrity of downloadsize (2 char)
 * |       ||   | |`--------------------------- current download speed (4 char)
 * |       ||   | `----------------------------------------- seperator (1 char)
 * |       ||   `---------------------------- integrity of upload size (2 char)
 * |       |`------------------------------------ current upload speed (4 char)
 * |       `---------------------------------------------------- space (1 char)
 * `------------------------------- interface name with semicolon (max 13 char)
 *
 * on multiple interfaces text will change to:
 * enp0s25: 1024mb/1024gb | wlp3s0: 1024mb/1024gb\0
 * `--------.-----------´`.´`---------.---------´
 *          |             |           `------------ 2nd interface (max 27 char)
 *          |             `-------------- space pipe and another space (3 char)
 *          `-------------------------------------- 1st interface (max 27 char)
 *
 * so the string size is: NETDEVCOUNT * (27 + 3) + 1, 27 for each interface,
 * 3 for each seperator. 1 for the \0
 */
void *
update_netdev(struct Info *st)
{
	char *new_text;
	uint64_t up_b4[NETDEVCOUNT];
	uint64_t down_b4[NETDEVCOUNT];
	int i;
	FILE *fp;
	int first;
	char interface_name[14];
	uint64_t rec;
	uint64_t snd;
	uint64_t up;
	uint64_t down;
	char fmt_up[7];
	char fmt_down[7];
	int count;
	int empty_count;
	size_t interface_text_length = NETDEVCOUNT * (27 + 3) + 3;

	usleep((uint)rand() % 100000);

	for (i = 0; i < NETDEVCOUNT; i++) {
		up_b4[i] = 0;
		down_b4[i] = 0;
	}

	while (1) {
		char buffer[512];

		fp = fopen(NETDEV, "r");
		if (fp == NULL) {
			new_text = malloc(sizeof(char) * 13);
			memset(new_text, '\0', 13);
			strncpy(new_text, "netdev_error", 13);
			swap_text(st, new_text);
			sleep(st->sleep);
			continue;
		}
		first = 1;
		up = 0;
		down = 0;
		rec = 0;
		snd = 0;
		empty_count = 0;
		new_text = malloc(sizeof(char) * interface_text_length);
		memset(new_text, '\0', interface_text_length);

		// throw away first 2 lines
		fgets(buffer, 512, fp);
		fgets(buffer, 512, fp);
		for (count = 0; count < NETDEVCOUNT; count++) {
			memset(buffer, '\0', 512);
			memset(interface_name, '\0', 13);

			fgets(buffer, 512, fp);
			sscanf(buffer,
			    "%11s %lu %*u %*u %*u %*u %*u %*u %*u %lu",
			    interface_name, &rec, &snd);
			up = snd - up_b4[count];
			down = rec - down_b4[count];
			up_b4[count] = snd;
			down_b4[count] = rec;
			if ((up == 0) && (down == 0)) {
				empty_count++;
				continue;
			}
			if (first) {
				snprintf(new_text, 27, "%s %6s/%6s",
				    interface_name,
				    fmt_size(up, fmt_up),
				    fmt_size(down, fmt_down));
				first = 0;
			} else {
				char tmp[31];

				memset(tmp, '\0', 31);
				snprintf(tmp, 30, " | %s %6s/%6s",
				    interface_name,
				    fmt_size(up, fmt_up),
				    fmt_size(down, fmt_down));
				strncat(new_text, tmp, 31);
			}
		}
		fclose(fp);
		swap_text(st, new_text);
		if (empty_count == count) {
			new_text = malloc(sizeof(char) * 4);
			memset(new_text, '\0', 4);
			strncpy(new_text, "-/-", 4);
			swap_text(st, new_text);
		}
		sleep(st->sleep);
	}
}

void *
update_loadavg(struct Info *st)
{
	double avg[3] = {0.0, 0.0, 0.0};
	char *new_text;

	usleep((uint)rand() % 100000);

	while (1) {
		if (getloadavg(avg, 3) < 0) {
			new_text = malloc(sizeof(char) * 12);
			memset(new_text, '\0', 12);
			strncpy(new_text, "avg's error", 12);
			swap_text(st, new_text);
			sleep(st->sleep);
			continue;
		}
		new_text = malloc(sizeof(char) * 15);
		memset(new_text, '\0', 15);
		sprintf(new_text, "%.2f %.2f %.2f", avg[0], avg[1], avg[2]);
		swap_text(st, new_text);
		sleep(st->sleep);
	}
}

void *
update_ram(struct Info *st)
{
	int ram[5];
	char buffer[1024];
	FILE *fp;
	char *new_text;

	usleep((uint)rand() % 100000);

	while (1) {
		int i;

		if ((fp = fopen(RAM, "r")) == NULL) {
			new_text = malloc(sizeof(char) * 11);
			memset(new_text, '\0', 11);
			strncpy(new_text, "ram error", 11);
			swap_text(st, new_text);
			sleep(st->sleep);
			continue;
		}

		for (i = 0; fgets(buffer, 1024, fp); i++) {
			if (i == 5) {
				break;
			}
			sscanf(buffer, "%*s %d %*s", &ram[i]);
		}
		fclose(fp);

		ram[2] = ram[0] - (ram[1] + ram[3] + ram[4]); // ram used

		new_text = malloc(sizeof(char) * 8);
		memset(new_text, '\0', 8);
		sprintf(new_text, "%d mb", ram[2] >> 10);
		swap_text(st, new_text);
		sleep(st->sleep);
	}
}

void *
update_sound(struct Info *st)
{
	long vol;
	long vol_min;
	long vol_max;
	float volume;
	int switch_value;
	snd_mixer_t *h_mixer;
	snd_mixer_selem_id_t *sid;
	snd_mixer_elem_t *elem;
	char *new_text;

	while (1) {
		if ((snd_mixer_open (&h_mixer, 1 ) < 0)
		    || (snd_mixer_attach ( h_mixer, "default" ) < 0)
		    || (snd_mixer_selem_register( h_mixer, NULL, NULL) < 0)
		    || (snd_mixer_load ( h_mixer ) < 0) )
		{
			new_text = malloc(sizeof(char) * 16);
			memset(new_text, '\0', 16);
			strncpy(new_text, "snd_mixer error", 16);
			swap_text(st, new_text);
			sleep(st->sleep);
			continue;
		}

		snd_mixer_selem_id_alloca(&sid);
		snd_mixer_selem_id_set_index(sid, 0);
		snd_mixer_selem_id_set_name(sid, "Master");

		if ((elem = snd_mixer_find_selem(h_mixer, sid)) == NULL) {
			new_text = malloc(sizeof(char) * 6);
			memset(new_text, '\0', 6);
			strncpy(new_text, "find error", 6);
			swap_text(st, new_text);
			sleep(st->sleep);
			continue;
		}

		snd_mixer_selem_get_playback_volume(elem,
		    SND_MIXER_SCHN_FRONT_LEFT, &vol);
		snd_mixer_selem_get_playback_volume_range(elem, &vol_min,
		    &vol_max);
		snd_mixer_selem_get_playback_switch(elem,
		    SND_MIXER_SCHN_FRONT_LEFT, &switch_value);

		snd_mixer_close(h_mixer);

		volume = 100.0f * (float)vol / (float)vol_max;

		new_text = malloc(sizeof(char) * 8);
		memset(new_text, '\0', 8);
		sprintf(new_text, "%s %.0f%%",
		    (switch_value == 1) ? "on" : "off", volume);
		swap_text(st, new_text);
		sleep(st->sleep);
	}
}

void *
update_battery(struct Info *st)
{
	FILE *fd_now;
	FILE *fd_full;
	FILE *fd_status;
	int energy_now;
	int energy_full;
	char energy_status[12];
	int battery_percentage;
	char *new_text;

	usleep((uint)rand() % 100000);

	while (1) {
		if ((fd_now = fopen(BATTERY_NOW, "r")) == NULL) {
			new_text = malloc(sizeof(char) * 6);
			strncpy(new_text, "on AC", 6);
			swap_text(st, new_text);
			sleep(st->sleep);
			continue;
		} else {
			fscanf(fd_now, "%d", &energy_now);
			fclose(fd_now);
		}

		if ((fd_full = fopen(BATTERY_FULL, "r")) == NULL) {
			new_text = malloc(sizeof(char) * 6);
			strncpy(new_text, "on AC", 6);
			swap_text(st, new_text);
			sleep(st->sleep);
			continue;
		} else {
			fscanf(fd_full, "%d", &energy_full);
			fclose(fd_full);
		}

		if ((fd_status = fopen(BATTERY_STATUS, "r")) == NULL) {
			new_text = malloc(sizeof(char) * 6);
			strncpy(new_text, "on AC", 6);
			swap_text(st, new_text);
			sleep(st->sleep);
			continue;
		} else {
			fscanf(fd_status, "%s", energy_status);
			fclose(fd_status);
		}

		battery_percentage = (int) ( ( ((float)energy_now)
					     / ((float)energy_full)
					     * 100.0)
					   + 0.5);
		new_text = malloc(sizeof(energy_status) + sizeof(char) * 6);
		sprintf(new_text, "%s %d%s", energy_status,
		    battery_percentage, "%");
		swap_text(st, new_text);
		sleep(st->sleep);
	}
}

void *
update_time(struct Info *st)
{
	struct tm *timeinfo;
	time_t rawtime;
	char *new_text;

	usleep((uint)rand() % 100000);

	while (1) {
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		new_text = malloc(sizeof(char) * 17);
		memset(new_text, '\0', 17);
		strncpy(new_text, asctime(timeinfo), 16);
		swap_text(st, new_text);
		sleep(st->sleep);
	}
}

void *
update_status()
{
	Display *display;
	char displayed_text[512];

	display = NULL;

	if (!print_only_flag) {
		if (!(display = XOpenDisplay(NULL))) {
			error("Cannot open display");
		}
		XStoreName(display, DefaultRootWindow(display),
		    "init statusbar...");
		XSync(display, False);
	}
	sleep(2);

	while (1) {
		size_t i;

		memset(displayed_text, '\0', 512);
		for (i = 0; i < sizeof(infos) / sizeof(struct Info); i++) {
			if ((infos[i].text == NULL)
			    || (infos[i].text[0] == '\0')) {
				continue;
			}

			if ((infos[i].before != NULL)
			    && (infos[i].before[0] != '\0')) {
				strncat(displayed_text, infos[i].before,
				    strlen(infos[i].before));
			}

			strncat(displayed_text, infos[i].text,
			    strlen(infos[i].text));

			if ((infos[i].after != NULL)
			    && (infos[i].after[0] != '\0')) {
				strncat(displayed_text, infos[i].after,
				    strlen(infos[i].after));
			}
		}
		if (!print_only_flag) {
			XStoreName(display, DefaultRootWindow(display),
			    displayed_text);
			XSync(display, False);
		} else {
			printf("%s\n", displayed_text);
			fflush(stdout);
		}
		sleep(REFRESH);
	}
	if (!print_only_flag) {
		XCloseDisplay(display);
	}
}

int
main(int argc, const char *argv[])
{
	size_t i;
	pthread_t thread;
	struct sigaction action;

	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = quit;
	sigaction(SIGTERM, &action, NULL);

	if (argc > 1) {
		if ((argv[1][0] == '-') && (argv[1][1] == 't') ) {
			print_only_flag = 1;
		}
	}

	for (i = 0; i < sizeof(infos)/sizeof(struct Info); i++) {
		if (pthread_create(&thread, NULL, (void *)infos[i].fun,
		        ((void *) &infos[i])) != 0)
		{
			error("couldn't create thread\n");
		}
		pthread_setname_np(thread, infos[i].name);
	}
	if (pthread_create(&thread, NULL, (void *)update_status, NULL) != 0) {
		error("couldn't create thread\n");
	}
	pthread_setname_np(thread, "status");

	pthread_join(thread, NULL);

	return 0;
}
