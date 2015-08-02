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
swap_text(struct Info *st, char *new_text)
{
	char *tmp = st->text;
	st->text = new_text;
	if (tmp != NULL) {
		free(tmp);
		tmp = NULL;
	}
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

	usleep((uint)rand() % 100000);

	if ((my_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error("could not create socket");
	}

	bzero((char *) &server_addr, sizeof(server_addr));

	server_addr.sin_port = htons(atoi(PORT));
	server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server_addr.sin_family = AF_INET;

	if (bind(my_socket, (struct sockaddr *) &server_addr,
	    sizeof(server_addr)) < 0) {
		error("could not bind");
	}

	listen(my_socket, 5);
	client_length = sizeof(client_addr);
	connected_socket = accept(my_socket, (struct sockaddr *) &client_addr,
			       (socklen_t *) &client_length);
	while (connected_socket) {
		bzero(buffer, MSG_LENGTH);
		if (read(connected_socket, buffer, MSG_LENGTH - 1) < 0) {
			error("could not read");
		}

		if (buffer[0] == '0') {
			swap_text(st, NULL);
		} else {
			size_t size;
			char *new_text;

			size = strlen(buffer) + 1;
			new_text = malloc(sizeof(char) * size);
			bzero(new_text, size);
			strncpy(new_text, buffer, size);
			swap_text(st, new_text);
		}
		if (write(connected_socket, "OK", 2) < 0) {
			error("could not write");
		}

		connected_socket = accept(my_socket,
				       (struct sockaddr *) &client_addr,
				       (socklen_t *) &client_length);
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
			bzero(new_text, 11);
			strncpy(new_text, "stat_error", 11);
			swap_text(st, new_text);
			sleep(st->sleep);

			continue;
		}

		/* read from file into cpu array */
		for (i = 0; i < CPU_CORES + 1; i++) {
			bzero(buffer, 1024);
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
		bzero(new_text, size);
		for (i = 0; i < CPU_CORES+1; i++) {
			bzero(buff, 4);
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

void *
update_netdev(struct Info *st)
{
	char *new_text;
	uint64_t up_b4[NETDEVCOUNT];
	uint64_t down_b4[NETDEVCOUNT];
	int i;
	FILE *fp;
	int first;
	char interface[12];
	uint64_t up;
	uint64_t down;
	uint64_t received;
	uint64_t send;
	int count;
	int empty_count;
	char *interface_text;

	usleep((uint)rand() % 100000);

	for (i = 0; i < NETDEVCOUNT; i++) {
		up_b4[i] = 0;
		down_b4[i] = 0;
	}

	while (1) {
		char *buffer;

		buffer = malloc(sizeof(char) * 512);
		fp = fopen(NETDEV, "r");
		if (fp == NULL) {
			new_text = malloc(sizeof(char) * 13);
			bzero(new_text, 13);
			strncpy(new_text, "netdev_error", 13);
			swap_text(st, new_text);
			sleep(st->sleep);
			continue;
		}
		fgets(buffer, 512, fp);
		fgets(buffer, 512, fp);
		bzero(buffer, 512);
		first = 1;
		up = 0;
		down = 0;
		received = 0;
		send = 0;
		empty_count = 0;
		interface_text = malloc(sizeof(char) * 32 * (size_t)NETDEVCOUNT);
		bzero(interface_text, 32 * NETDEVCOUNT);
		for (count = 0; count < NETDEVCOUNT; count++) {
			char *tmp;

			fgets(buffer, 512, fp);
			bzero(interface, 12);
			sscanf(buffer,
			    "%11s %lu %*u %*u %*u %*u %*u %*u %*u %lu",
			    interface, &received, &send);
			bzero(buffer, 512);
			up = send - up_b4[count];
			down = received - down_b4[count];
			up_b4[count] = send;
			down_b4[count] = received;
			if ((up == 0) && (down == 0)) {
				empty_count++;
				continue;
			}
			if (first) {
				sprintf(interface_text, "%s %lu/%lu kBs",
				    interface, up >> 10, down >> 10);
				first = 0;
			} else {
				tmp = malloc(sizeof(char) * 32);
				bzero(tmp, 32);
				sprintf(tmp, " | %s %lu/%lu kBs",
				    interface, up >> 10, down >> 10);
				strncat(interface_text, tmp, 32);
				free(tmp);
			}
		}
		fclose(fp);
		free(buffer);
		swap_text(st, interface_text);
		if (empty_count == count) {
			new_text = malloc(sizeof(char) * 8);
			bzero(new_text, 8);
			sprintf(new_text, "-/- kBs");
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
			bzero(new_text, 12);
			strncpy(new_text, "avg's error", 12);
			swap_text(st, new_text);
			sleep(st->sleep);
			continue;
		}
		new_text = malloc(sizeof(char) * 15);
		bzero(new_text, 15);
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
			bzero(new_text, 11);
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
		bzero(new_text, 8);
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
			bzero(new_text, 16);
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
			bzero(new_text, 6);
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
		bzero(new_text, 8);
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
		bzero(new_text, 17);
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

		bzero(displayed_text, 512);
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
