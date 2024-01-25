/*Written by Nguyen Bao Nguyen Le | nguyenle.18082003@gmail.com
  This program controls GPIO LEDs and buttons via UIO using 
  epoll medthod
  Board used: Zynq UltraScale+ MPSoc ZCU102*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/epoll.h>
 
#define GPIO_MAP_SIZE     0x10000
#define GPIO_DATA_OFFSET  0x0
#define GPIO_TRI_OFFSET   0x4     // gpio-buttons
#define GPIO2_DATA_OFFSET 0x8
#define GPIO2_TRI_OFFSET  0xc     // gpio-LEDs
#define GIER              0x11c
#define IP_IER            0x128
#define IP_ISR            0x120

int main(int argc, char *argv[])
{
	int fd, irq, epoll_fd; 
	uint32_t *gpios;
	uint32_t val0, val1, val2, val3, val4, val5, val6;
	struct epoll_event ev;
	struct epoll_event events[1];

	/*open file descriptor*/
	fd = open("/dev/uio4", O_RDWR);
	if (fd < 1) {
		printf("Error opening device file\n");
		exit(EXIT_FAILURE);
	}

	/*memory mapping*/
	gpios = mmap(NULL, GPIO_MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	/*print initial value of registers*/
	printf("Initial value of registers are:\n");
	val0 = *(uint32_t *)(gpios + GPIO_DATA_OFFSET / sizeof(uint32_t));
	printf("GPIO_DATA_OFFSET: %8x\n", val0);
	val1 = *(uint32_t *)(gpios + GPIO_TRI_OFFSET / sizeof(uint32_t));
	printf("GPIO_TRI_OFFSET: %8x\n", val1);
	val2 = *(uint32_t *)(gpios + GPIO2_DATA_OFFSET / sizeof(uint32_t));
	printf("GPIO2_DATA_OFFSET: %8x\n", val2);
	val3 = *(uint32_t *)(gpios + GPIO2_TRI_OFFSET / sizeof(uint32_t));
	printf("GPIO_TRI_OFFSET: %8x\n", val3);
	val4 = *(uint32_t *)(gpios + GIER / sizeof(uint32_t));
	printf("GIER: %8x\n", val4);
	val5 = *(uint32_t *)(gpios + IP_IER /sizeof(uint32_t));
	printf("IP_IER: %8x\n", val5);
	val6 = *(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t));
	printf("IP_ISR: %8x\n", val6);

	/*enable all interrupts*/
	printf("Enable all interrupts\n");
	*(uint32_t *)(gpios + GIER / sizeof(uint32_t)) = 0x80000000;
	val4 = *(uint32_t *)(gpios + GPIO_DATA_OFFSET / sizeof(uint32_t));
	printf("GIER: %8x\n", val4);
	*(uint32_t *)(gpios + IP_IER / sizeof(uint32_t)) = 0x1;
	val5 = *(uint32_t *)(gpios + IP_IER / sizeof(uint32_t));
	printf("IP_IER: %8x\n", val5);

	/*enable UIO interrupt*/
	irq = 1;
	write(fd, &irq, sizeof(int));

	/*use epoll to handle interrupts*/
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		printf("Error creating file descriptor\n");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN | EPOLLET; // trigger was set to EDGE RISING in device-tree
	ev.data.fd = fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		printf("Error adding file descriptor to epoll\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		int num_events;

		write(fd, &irq, sizeof(int)); // re-enable UIO interrupt
		printf("Press the buttons\n");
		num_events = epoll_wait(epoll_fd, events, 1, -1); // waiting for interrupt
		if (num_events == -1) {
			printf("Error waiting for events\n");
			exit(EXIT_FAILURE);
		}

		if (num_events > 0 && (events[0].events & EPOLLIN)) {
			*(uint32_t *)(gpios + GPIO2_DATA_OFFSET / sizeof(uint32_t)) = *(uint32_t *)(gpios + GPIO_DATA_OFFSET / sizeof(uint32_t));
		}

		*((uint32_t *)(gpios + IP_ISR / sizeof(uint32_t))) = 0x1; // toggle IP_ISR
		printf("Done\n\n");
	}
	munmap(gpios, GPIO_MAP_SIZE);
	close(fd);
	return 0;
}
