#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>

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
	printf("Checkpoint 1: fd done\n\n");

	//uint32_t *gpios = malloc(100 * sizeof(uint32_t));
       	uint32_t* gpios = mmap(NULL, GPIO_MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	/*print pointer value and the value of what it is pointing at*/
	printf("gpios: %p\n", gpios);
	printf("Checkpoint 2: mmap done\n\n");

	/*print initial value of registers*/
	uint32_t val0, val1, val2, val3, val4, val5, val6;
	val0 = *(uint32_t *)(gpios + GPIO_DATA_OFFSET / sizeof(uint32_t));
	printf("GPIO_DATA_OFFSET: %08x\n", val0);
	val1 = *(uint32_t *)(gpios + GPIO_TRI_OFFSET / sizeof(uint32_t));
	printf("GPIO_TRI_OFFSET: %08x\n", val1);
	val2 = *(uint32_t *)(gpios + GPIO2_DATA_OFFSET / sizeof(uint32_t));
	printf("GPIO2_DATA_OFFSET: %08x\n", val2);
	val3 = *(uint32_t *)(gpios + GPIO2_TRI_OFFSET / sizeof(uint32_t));
	printf("GPIO2_TRI_OFFSET: %08x\n", val3);
	val4 = *(uint32_t *)(gpios + GIER / sizeof(uint32_t));
	printf("GIER: %08x\n", val4);
	val5 = *(uint32_t *)(gpios + IP_IER / sizeof(uint32_t));
	printf("IP_IER: %08x\n", val5);
	val6 = *(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t));	
	printf("IP_ISR: %08x\n", val6);
	printf("\n");

	/*enable all interupts*/
	*(uint32_t *)(gpios + GIER / sizeof(uint32_t)) = 0x80000000;
	val1 = *(uint32_t *)(gpios + GIER / sizeof(uint32_t));
	printf("GIER: %08x\n", val1);
	printf("Checkpoint 2.1\n");

	*(uint32_t *)(gpios + IP_IER / sizeof(uint32_t)) = 0x1;
	val2 = *(uint32_t *)(gpios + IP_IER / sizeof(uint32_t));
        printf("IP_IER: %08x\n", val2);
	printf("Checkpoint 2.2\n");

	*(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)) = 0x0;
	val3 = *(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t));
        printf("IP_ISR: %08x\n", val3);
	printf("Checkpoint 3: enable all interrupts done\n\n");

	/*enable UIO interrupts*/
	int irq = 1;
	write(fd, &irq, sizeof(irq));
	printf("Checkpoint 4: enable UIO interrupt done\n\n");

	/*set GPIO pin as input*/
	/*read from GPIO*/
	/**(uint32_t *)(gpios + GPIO_TRI_OFFSET / sizeof(uint32_t)) = 31;
	val0 = *(uint32_t *)(gpios + GPIO_DATA_OFFSET / sizeof(uint32_t));
	printf("Input is: %x\n", val0);*/
	printf("Checkpoint 5: read from GPIO done\n\n");

	/*print interrupts value*/
	val4 = *((uint32_t *)(gpios + GIER / sizeof(uint32_t)));
	printf("GIER: %08x\n", val4);
	printf("Checkpoint 5.1\n");
	val5 = *((uint32_t *)(gpios + IP_IER / sizeof(uint32_t)));
	printf("IP_IER: %08x\n", val5);
	printf("Checkpoint 5.2\n");
	val6 = *((uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)));
	printf("IP_ISR: %08x\n", val6);
	printf("Checkpoint 6: print interrupts value done\n\n");

		uint32_t count = 0;
		int err;
	while(1){
        	printf("Checkpoint 6.0: enable UIO interrupt done\n\n");
		printf("Checkpoint 6.1: waiting\n");
		/*if (*(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)) == 0x1){
			*(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)) = 0x0;
			val6 = *(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t));
                	printf("IP_ISR: %08x\n", val6);
		}*/
		if (*(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)) == 0x1) {
        		// Handle the interrupt
			*(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)) = 0x1; // Clear the interrupt
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

		//*(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t)) = 0x1;
		val6 = *(uint32_t *)(gpios + IP_ISR / sizeof(uint32_t));
       		printf("IP_ISR: %08x\n", val6);
		usleep(10000);
		count = 0;
	}
	//printf("Addr before unmap: %p\n", gpios);
	munmap(gpios, GPIO_MAP_SIZE);
	//printf("Addr after unmap: %p\n", gpios);
	close(fd);
	printf("Checkpoint 8: munmap done\n\n");
	printf("Checkpoint 9: free done\n\n");
	return 0;
}
