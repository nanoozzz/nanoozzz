/*Written by Nguyen Bao Nguyen Le | nguyenle.18082003@gmail.com
 *This program polls 2 file descriptors (2 uio devices/buttons)
  to control LEDs in a given order 
  Board used: Zynq UltraScale+ MPSoc ZCU102*/

#include <stdio.h>
#include <poll.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define GPIO_MAP_SIZE     0x10000
#define GPIO_DATA_OFFSET  0x0
#define GPIO_TRI_OFFSET   0x4     // gpio-buttons
#define GPIO2_DATA_OFFSET 0x8
#define GPIO2_TRI_OFFSET  0xc     // gpio-LEDs
#define GIER              0x11c
#define IP_IER            0x128
#define IP_ISR            0x120
void do_control(uint32_t *gpios0, uint32_t *gpios1);
int main(int argc, char *argv[])
{
	int irq0, irq1, fd0, fd1;
	int flag0 = 0, flag1 = 0, pflag0 = 1, pflag1 = 1;
	uint32_t *gpios0;
	uint32_t *gpios1;
	uint32_t val0, val1, val2, val3, val4, val5, val6;
	struct pollfd fds[2];

	fd0 = open("/dev/uio4", O_RDWR);
	if (fd0 < 1) {
		printf("Open file /dev/uio4 error\n");
	}
	gpios0 = mmap(NULL, GPIO_MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd0, 0);
	
	fd1 = open("/dev/uio5", O_RDWR);
	if (fd1 < 1) {
		printf("Open file /dev/uio5 error\n");
	}
	gpios1 = mmap(NULL, GPIO_MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);
	printf("mmap done\n");

	/*enable all interrupts*/
        printf("Enable all interrupts gpios0\n");
        *(uint32_t *)(gpios0 + GIER / sizeof(uint32_t)) = 0x80000000;
	val4 = *(uint32_t *)(gpios0 + GIER / sizeof(uint32_t));
        printf("GIER: %8x\n", val4);
        *(uint32_t *)(gpios0 + IP_IER / sizeof(uint32_t)) = 0x1;
        val5 = *(uint32_t *)(gpios0 + IP_IER / sizeof(uint32_t));
        printf("IP_IER: %8x\n", val5);
        if (*((uint32_t *)(gpios0 + IP_ISR / sizeof(uint32_t))) == 0x1) {
                *((uint32_t *)(gpios0 + IP_ISR / sizeof(uint32_t))) = 0x1;
        }
	val6 = *(uint32_t *)(gpios0 + IP_ISR / sizeof(uint32_t));
        printf("IP_ISR: %8x\n", val6);

	printf("Enable all interrupts gpios1\n");
        *(uint32_t *)(gpios1 + GIER / sizeof(uint32_t)) = 0x80000000;
        val4 = *(uint32_t *)(gpios1 + GIER / sizeof(uint32_t));
        printf("GIER: %8x\n", val4);
        *(uint32_t *)(gpios1 + IP_IER / sizeof(uint32_t)) = 0x1;
        val5 = *(uint32_t *)(gpios1 + IP_IER / sizeof(uint32_t));
        printf("IP_IER: %8x\n", val5);
        if (*((uint32_t *)(gpios1 + IP_ISR / sizeof(uint32_t))) == 0x1) {
                *((uint32_t *)(gpios1 + IP_ISR / sizeof(uint32_t))) = 0x1;
        }
        val6 = *(uint32_t *)(gpios1 + IP_ISR / sizeof(uint32_t));
        printf("IP_ISR: %8x\n", val6);

        /*enable UIO interrupt*/
        irq0 = 1;
	irq1 = 1;
	write(fd0, &irq0, sizeof(int));
        write(fd1, &irq1, sizeof(int));

	/*set up pollfd structure*/
	fds[0].fd = fd0;
	fds[0].events = POLLIN;
	fds[1].fd = fd1;
	fds[1].events = POLLIN;

	while (1) {
		int ret, res0, res1, buf0, buf1;
		
		printf("Toggle the button\n");
		ret = poll(fds, 2, -1);
		if (ret < 0){
			printf("Error polling\n");
		}
		
		if ((fds[0].revents & POLLIN)) {
			write(fds[0].fd, &irq0, sizeof(int));
			res0 = read(fd0, &buf0, sizeof(int));

			if (res0 > 0) {
				printf("Interrupt uio0 caught, res0 is %d\n", res0);
			}
			*((uint32_t *)(gpios0 + IP_ISR / sizeof(uint32_t))) = 0x1; // toggle IP_ISR
			pflag0 = flag0;
			flag0 = !flag0;
			
			if ((flag1 != 0) && (flag0 == 0) && (pflag0 != 1) && !*((uint32_t *)(gpios1 + GPIO2_DATA_OFFSET / sizeof(uint32_t)))){
				*((uint32_t *)(gpios0 + GPIO2_DATA_OFFSET / sizeof(uint32_t))) = 0;
			}
		}

		if ((fds[1].revents & POLLIN)) {
			write(fds[1].fd, &irq1, sizeof(int));
			res1 = read(fd1, &buf1, sizeof(int));
                       	
			if (res1 > 0) {
                               	printf("Interrupt uio1 caught, res1 is %d\n", res1);
			}
			*((uint32_t *)(gpios1 + IP_ISR / sizeof(uint32_t))) = 0x1; // toggle IP_ISR
			pflag1 = flag1;
			flag1 = !flag1;

			if ((((*((uint32_t *)(gpios0 + GPIO_DATA_OFFSET / sizeof(uint32_t)))) == 1) || ((*((uint32_t *)(gpios0 + GPIO_DATA_OFFSET / sizeof(uint32_t)))) == 3)) && (*((uint32_t *)(gpios1 + GPIO_DATA_OFFSET / sizeof(uint32_t)))) == 1) {
				*((uint32_t *)(gpios0 + GPIO2_DATA_OFFSET / sizeof(uint32_t))) = 1;
			}
			else if ((((*((uint32_t *)(gpios0 + GPIO_DATA_OFFSET / sizeof(uint32_t)))) == 2) || ((*((uint32_t *)(gpios0 + GPIO_DATA_OFFSET / sizeof(uint32_t)))) == 3)) && (*((uint32_t *)(gpios1 + GPIO_DATA_OFFSET / sizeof(uint32_t)))) == 2) {
				*((uint32_t *)(gpios0 + GPIO2_DATA_OFFSET / sizeof(uint32_t))) = 2;
			}
			else if (((*((uint32_t *)(gpios0 + GPIO_DATA_OFFSET / sizeof(uint32_t)))) == 3) && (*((uint32_t *)(gpios1 + GPIO_DATA_OFFSET / sizeof(uint32_t)))) == 3) {
				*((uint32_t *)(gpios0 + GPIO2_DATA_OFFSET / sizeof(uint32_t))) = 3;
			}
			else {
				*((uint32_t *)(gpios0 + GPIO2_DATA_OFFSET / sizeof(uint32_t))) = 0;
			}
		}
	}
	munmap(gpios0, GPIO_MAP_SIZE);
        munmap(gpios1, GPIO_MAP_SIZE);
        close(fd1);
        close(fd0);
        return 0;
}
