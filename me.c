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
#define GIER 0x11c
#define IP_IER 0x128
#define IP_ISR 0x120

int main(int argc, char *argv[]) 
{
	int fd = open("/dev/uio0", O_RDWR);
	if (fd < 1) {
		perror(argv[0]);
		return -1;
	}
	printf("Checkpoint 1: fd done\n\n");

	uint32_t *buttons = malloc(100 * sizeof(uint32_t));
	buttons = mmap(NULL, GPIO_MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	/*print pointer value and the value of what it is pointing at*/
	printf("buttons: %p\n", buttons);
	printf("Checkpoint 2: mmap done\n\n");

	/*print initial value of registers*/
	printf("GIER: %08x\n",*(uint32_t *)(buttons + GIER / sizeof(uint32_t)));
	printf("IP_IER: %08x\n",*(uint32_t *)(buttons + IP_IER / sizeof(uint32_t)));
	printf("IP_ISR: %08x\n",*(uint32_t *)(buttons + IP_ISR / sizeof(uint32_t)));

	/*enable all interupts*/
	uint32_t val0, val1, val2, val3;
	*(uint32_t *)(buttons + GIER / sizeof(uint32_t)) = 0x80000000;
	val1 = *(uint32_t *)(buttons + GIER / sizeof(uint32_t));
	printf("GIER: %08x\n", val1);
	printf("Checkpoint 2.1\n");

	*(uint32_t *)(buttons + IP_IER / sizeof(uint32_t)) = 0x1;
	val2 = *(uint32_t *)(buttons + IP_IER / sizeof(uint32_t));
        printf("IP_IER: %08x\n", val2);
	printf("Checkpoint 2.2\n");

	/**(uint32_t *)(buttons + IP_ISR / sizeof(uint32_t)) = 0x1;
	val3 = *(uint32_t *)(buttons + IP_ISR / sizeof(uint32_t));
        printf("IP_ISR: %08x\n", val3);*/
	printf("Checkpoint 3: enable all interrupts done\n\n");

	/*enable UIO interrupts*/
	int irq = 1;
	write(fd, &irq, sizeof(irq));
	printf("Checkpoint 4: enable UIO interrupt done\n\n");

	/*set GPIO pin as input*/
	/*read from GPIO*/
	*(uint32_t *)(buttons + GPIO_TRI_OFFSET / sizeof(uint32_t)) = 31;
	val0 = *(uint32_t *)(buttons + GPIO_DATA_OFFSET / sizeof(uint32_t));
	printf("Input is: %x\n", val0);
	printf("Checkpoint 5: read from GPIO done\n\n");

	/*print interrupts value*/
	val1 = *((uint32_t *)(buttons + GIER / sizeof(uint32_t)));
	printf("GIER: %08x\n", val1);
	printf("Checkpoint 5.1\n");
	val2 = *((uint32_t *)(buttons + IP_IER / sizeof(uint32_t)));
	printf("IP_IER: %08x\n", val2);
	printf("Checkpoint 5.2\n");
	val3 = *((uint32_t *)(buttons + IP_ISR / sizeof(uint32_t)));
	printf("IP_ISR: %08x\n", val3);
	printf("Checkpoint 6: print interrupts value done\n\n");

	uint32_t count = 0;
	int err;
	err = read(fd, &count, 4);
	printf("err is: %d\n", err);
	printf("Checkpoint 7: gotcha!\n");
	*(uint32_t *)(buttons + IP_ISR / sizeof(uint32_t)) = 0x1;
	val3 = *(uint32_t *)(buttons + IP_ISR / sizeof(uint32_t));
        printf("IP_ISR: %08x\n", val3);
	val0 = *(uint32_t *)(buttons + GPIO_DATA_OFFSET / sizeof(uint32_t));
	printf("Input is: %x\n", val0);
	printf("Addr before unmap: %p\n", buttons);
	munmap(buttons, GPIO_MAP_SIZE);
	printf("Addr after unmap: %p\n", buttons);
	close(fd);
	printf("Checkpoint 8: munmap done\n\n");
	printf("Checkpoint 9: free done\n\n");
	return 0;
}
