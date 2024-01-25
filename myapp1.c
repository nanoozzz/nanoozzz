#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/epoll.h>

#define GPIO_MAP_SIZE 0x10000
#define GPIO_DATA_OFFSET 0x0
#define GPIO_TRI_OFFSET 0x4
#define GPIO2_DATA_OFFSET 0x8
#define GPIO2_TRI_OFFSET 0xc
#define GIER 0x11c
#define IP_IER 0x128
#define IP_ISR 0x120

int main(int argc, char *argv[]) 
{
	int fd = open("/dev/uio4", O_RDWR);
	if (fd < 1) {
		perror(argv[0]);
		return -1;
	}
	//printf("Checkpoint 1: fd done\n\n");

	//uint32_t *gpios = malloc(100 * sizeof(uint32_t));
       	uint32_t* gpios = mmap(NULL, GPIO_MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	/*print pointer value and the value of what it is pointing at*/
	//printf("gpios: %p\n", gpios);
	//printf("Checkpoint 2: mmap done\n\n");

	/*print initial value of registers*/
	uint32_t val0, val1, val2, val3, val4, val5, val6;
	val0 = *(uint32_t *)(gpios + GPIO_DATA_OFFSET / sizeof(uint32_t));
	//printf("GPIO_DATA_OFFSET: %08x\n", val0);
	val1 = *(uint32_t *)(gpios + GPIO_TRI_OFFSET / sizeof(uint32_t));
	//printf("GPIO_TRI_OFFSET: %08x\n", val1);
	val2 = *(uint32_t *)(gpios + GPIO2_DATA_OFFSET / sizeof(uint32_t));
	//printf("GPIO2_DATA_OFFSET: %08x\n", val2);
	val3 = *(uint32_t *)(gpios + GPIO2_TRI_OFFSET / sizeof(uint32_t));
	//printf("GPIO2_TRI_OFFSET: %08x\n", val3);
	val4 = *(uint32_t *)(gpios + GIER / sizeof(uint32_t));
	//printf("initial GIER: %08x\n", val4);
	val5 = *(uint32_t *)(gpios + IP_IER / sizeof(uint32_t));
	//printf("initial IP_IER: %08x\n", val5);
	val6 = *(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t));	
	printf("initial IP_ISR: %08x\n", val6);
	printf("\n");

	/*enable all interupts*/
	*(uint32_t *)(gpios + GIER / sizeof(uint32_t)) = 0x0;
	val1 = *(uint32_t *)(gpios + GIER / sizeof(uint32_t));
	//printf("enabled GIER: %08x\n", val1);
	//printf("Checkpoint 2.1\n");
	*(uint32_t *)(gpios + GIER / sizeof(uint32_t)) = 0x80000000;

	*(uint32_t *)(gpios + IP_IER / sizeof(uint32_t)) = 0x0;
	val2 = *(uint32_t *)(gpios + IP_IER / sizeof(uint32_t));
        //printf("enabled IP_IER: %08x\n", val2);
	//printf("Checkpoint 2.2\n");
	*(uint32_t *)(gpios + IP_IER / sizeof(uint32_t)) = 0x1; //re-enable interrupt

	*(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)) = (*(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t))) ? 0x1 : 0x0;
	val3 = *(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t));
        printf("enabled IP_ISR: %08x\n", val3);
	//printf("Checkpoint 3: enable all interrupts done\n\n");

	/*enable UIO interrupts*/
	int irq = 0;
	write(fd, &irq, sizeof(irq));
	irq = 1;
	//printf("Checkpoint 4: enable UIO interrupt done\n\n");

	/*set GPIO pin as input*/
	/*read from GPIO*/
	/**(uint32_t *)(gpios + GPIO_TRI_OFFSET / sizeof(uint32_t)) = 31;
	val0 = *(uint32_t *)(gpios + GPIO_DATA_OFFSET / sizeof(uint32_t));
	printf("Input is: %x\n", val0);*/
	//printf("Checkpoint 5: read from GPIO done\n\n");

	/*print interrupts value*/
	val4 = *((uint32_t *)(gpios + GIER / sizeof(uint32_t)));
	//printf("before interrupt GIER: %08x\n", val4);
	//printf("Checkpoint 5.1\n");
	val5 = *((uint32_t *)(gpios + IP_IER / sizeof(uint32_t)));
	//printf("before interrupt IP_IER: %08x\n", val5);
	//printf("Checkpoint 5.2\n");
	val6 = *((uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)));
	printf("before interrupt IP_ISR: %08x\n", val6);
	printf("Checkpoint 6: print interrupts value done\n\n");

	//epoll
	int epoll_fd = epoll_create1(0);
	printf("epoll_fd is %d\n", epoll_fd);
	if (epoll_fd == -1) {
        	perror("Error creating epoll file descriptor");
        	exit(EXIT_FAILURE);
    	}

    	struct epoll_event ev;
    	struct epoll_event events[1];

    	for (int i = 0; i < 1; i++) {
        	ev.events = EPOLLIN | EPOLLET;
        	ev.data.fd = fd;

        	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
			perror("Error adding file descriptor to epoll");
            		exit(EXIT_FAILURE);
        	}
    	}
	while(1){
		printf("start\n");
        write(fd, &irq, sizeof(irq));
		*((uint32_t *)(gpios + IP_ISR / sizeof(uint32_t))) = 0x0; //
		val6 = *((uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)));
                printf("ISR start is %x\n", val6);
	
		int num_events = epoll_wait(epoll_fd, events, 1, -1);
        	printf("num event is %d\n", num_events);
		printf("waiting to enter loop\n");
		if (num_events == -1) {
            		perror("Error waiting for events");
            		exit(EXIT_FAILURE);
        	}
		if (num_events > 0){
			for (int i = 0; i < num_events; i++) {
            			if (events[i].events & EPOLLIN) {
					/*if (lseek(events[i].data.fd, 0, SEEK_SET) == -1) {
                    				perror("Error during lseek");
                    				exit(EXIT_FAILURE);
                			}
                			if (read(events[i].data.fd, ((uint32_t *)(gpios + GPIO_DATA_OFFSET / sizeof(uint32_t))), sizeof(uint32_t)) == -1) {
                    				perror("Error during read");
                    				exit(EXIT_FAILURE);
                			}*/
					*(uint32_t *)(gpios + GPIO2_DATA_OFFSET / sizeof(uint32_t)) = *(uint32_t *)(gpios + GPIO_DATA_OFFSET / sizeof(uint32_t));
				}
			}
		}
		val6 = *((uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)));
		printf("ISR before toggled is %x\n", val6);

		*((uint32_t *)(gpios + IP_ISR / sizeof(uint32_t))) = 0x1;
		printf("toggled and done\n");
		
		val6 = *((uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)));
		printf("ISR after toggled is %x\n", val6);
	}






	uint32_t count = 0;
	int err;
	/*while(1){
        	//printf("Checkpoint 6.0: enable UIO interrupt done\n\n");
		printf("Checkpoint 6.1: waiting\n");

		if (*(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t))) {
        		// Handle the interrupt
			printf("before reset IP_ISR: %08x\n", val6);
			*(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)) = 0x1; // Clear the interrupt
		        val6 = *((uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)));
                        printf("after reset IP_ISR: %08x\n", val6);
		}

		err = read(fd, &count, sizeof(uint32_t));
		*(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)) = 0x1;
		printf("err is: %d\n", err);
		printf("Checkpoint 7: gotcha!\n");
		
		if (err > 0){
			*(uint32_t *)(gpios + GPIO2_DATA_OFFSET / sizeof(uint32_t)) = *(uint32_t *)(gpios + GPIO_DATA_OFFSET / sizeof(uint32_t));
		}
		val0 = *(uint32_t *)(gpios + GPIO2_DATA_OFFSET / sizeof(uint32_t));
		printf("Button is %x\n", val0);
		val2 = *(uint32_t *)(gpios + GPIO_DATA_OFFSET / sizeof(uint32_t));
		printf("Led is %x\n", val2);

		val6 = *(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t));
       		printf("after interrupt IP_ISR: %08x\n", val6);
		//usleep(10000);
		//count = 0;
	}*/
	munmap(gpios, GPIO_MAP_SIZE);
	close(fd);
	printf("Checkpoint 8: munmap done\n\n");
	printf("Checkpoint 9: free done\n\n");
	return 0;
}
