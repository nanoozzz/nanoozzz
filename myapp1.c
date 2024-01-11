/*
* Copyright (C) 2013 - 2016  Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in this
* Software without prior written authorization from Xilinx.
*
*/

/*this program controls GPIO LEDs using inputs from GPIO button*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/epoll.h>

#define MAXSTRLEN 50
#define NUMGPIO 10
#define STARTBUT 495
#define STARTLED 500
#define NUMBUT 5
#define NUMLED 5

volatile sig_atomic_t stop_flag = 0;

char gpio_value[][MAXSTRLEN+1] = {
	"/sys/class/gpio/gpio495/value",
	"/sys/class/gpio/gpio496/value",
	"/sys/class/gpio/gpio497/value",
	"/sys/class/gpio/gpio498/value",
	"/sys/class/gpio/gpio499/value",
	"/sys/class/gpio/gpio500/value",
	"/sys/class/gpio/gpio501/value",
	"/sys/class/gpio/gpio502/value",
	"/sys/class/gpio/gpio503/value",
	"/sys/class/gpio/gpio504/value"
};

char gpio_direction[][MAXSTRLEN+1] = {
	"/sys/class/gpio/gpio500/direction",
	"/sys/class/gpio/gpio501/direction",
	"/sys/class/gpio/gpio502/direction",
	"/sys/class/gpio/gpio503/direction",
	"/sys/class/gpio/gpio504/direction"
};

char gpio_export[MAXSTRLEN+1] = {
	"/sys/class/gpio/export"
};

char gpio_unexport[MAXSTRLEN+1] = {
	"/sys/class/gpio/unexport"
};

char gpio_interrupt[][MAXSTRLEN+1] = {
	"/sys/class/gpio/gpio495/edge",
	"/sys/class/gpio/gpio496/edge",
	"/sys/class/gpio/gpio497/edge",
	"/sys/class/gpio/gpio498/edge",
	"/sys/class/gpio/gpio499/edge"
};

void sigint_handler(int signum);
void do_export(char gpio_export[]);
void do_set_output(char gpio_direction[][MAXSTRLEN+1]);
void do_set_interrupt_trigger(char gpio_interrupt[][MAXSTRLEN+1]);
void do_turn_on(char gpio_value[]);
void do_turn_off(char gpio_value[]);
void do_control(char gpio_value[][MAXSTRLEN+1]);

void sigint_handler(int signum) {
    stop_flag = 1;
}

void do_export(char gpio_export[]){
	for (int i = STARTBUT; i < STARTBUT + NUMGPIO; i++) {
		FILE *export_file = fopen(gpio_export, "w");
		fprintf(export_file, "%d", i);
		fclose(export_file);
	}
}

void do_set_output(char gpio_direction[][MAXSTRLEN+1]){
	for (int i = 0; i < NUMLED; i++) {
		FILE *direction_file = fopen(gpio_direction[i], "w");
		fprintf(direction_file, "%s", "out");
		fclose(direction_file);
	}
}

void do_set_interrupt_trigger(char gpio_interrupt[][MAXSTRLEN+1]){
	for (int i = 0; i < NUMBUT; i++){
		FILE *interrupt_file = fopen(gpio_interrupt[i], "w");
		fprintf(interrupt_file, "%s", "rising");
		fclose(interrupt_file);
	}
}

void do_turn_on(char gpio_value[]){
        FILE *brightness_file = fopen(gpio_value, "w");

        if(brightness_file == NULL){
                perror("Error opening brightness file");
                exit(EXIT_FAILURE);
        }

        fprintf(brightness_file, "1");
        fclose(brightness_file);
}

void do_turn_off(char gpio_value[]){
        FILE *brightness_file = fopen(gpio_value, "w");

        if(brightness_file == NULL){
                perror("Error opening brightness file");
		exit(EXIT_FAILURE);
        }

        fprintf(brightness_file, "0");
        fclose(brightness_file);
}

void do_control(char gpio_value[][MAXSTRLEN+1]){
	int epoll_fd = epoll_create1(0);
	printf("epoll_fd is %d\n", epoll_fd);
    if (epoll_fd == -1)
    {
        perror("Error creating epoll file descriptor");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev;
    struct epoll_event events[NUMBUT];

    for (int i = 0; i < NUMBUT; i++)
    {
        int fd = open(gpio_value[i], O_RDONLY | O_NONBLOCK);
        if (fd == -1)
        {
            perror("Error opening GPIO file");
            exit(EXIT_FAILURE);
        }

        ev.events = EPOLLIN;
        ev.data.fd = fd;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
        {
            perror("Error adding file descriptor to epoll");
            exit(EXIT_FAILURE);
        }
    }

    while (1)
    {
        int num_events = epoll_wait(epoll_fd, events, NUMBUT, -1);
        printf("num event is %d\n", num_events);
	printf("waiting to enter loop\n");
	if (num_events == -1)
        {
            perror("Error waiting for events");
            exit(EXIT_FAILURE);
        }
	if (num_events > 0){
		printf("got in loop\n");
        for (int i = 0; i < num_events; i++)
        {
            if (events[i].events & EPOLLIN)
            {
                char tmp;
                if (lseek(events[i].data.fd, 0, SEEK_SET) == -1)
                {
                    perror("Error during lseek");
                    exit(EXIT_FAILURE);
                }
                if (read(events[i].data.fd, &tmp, 1) == -1)
                {
                    perror("Error during read");
                    exit(EXIT_FAILURE);
                }

                if (tmp == '1')
                {
                    do_turn_on(gpio_value[NUMLED + i]);
                }
                else
                {
                    do_turn_off(gpio_value[NUMLED + i]);
                }
            }
        }

        // Introduce a short sleep to reduce CPU usage
        usleep(10000); // Sleep for 10 milliseconds (adjust as needed)
    }
    }

    close(epoll_fd);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, sigint_handler);
        do_export(gpio_export);
        do_set_output(gpio_direction);
        do_set_interrupt_trigger(gpio_interrupt);
        do_control(gpio_value);
        return 0;
}
