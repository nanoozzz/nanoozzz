#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>

#define GPIO_MAP_SIZE 0x10000
#define GPIO_DATA_OFFSET 0x0
#define GPIO_TRI_OFFSET 0x4
#define GIER 0x11c
#define IS_IER 0x128
#define IS_ISR 0x120

int main(int argc, char *argv[]) 
{
	int fd = open("/dev/uio4", O_RDWR);
	if (fd < 1) {
		perror(argv[0]);
		return -1;
	}
	printf("Checkpoint 1: fd done\n");

	void *buttons;
	buttons = mmap(NULL, GPIO_MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	/*print pointer value and the value of what it is pointing at*/
	printf("buttons: %p,", buttons);
	printf(" pointing to the memory of content %x\n", *buttons);
	printf("Checkpoint 2: mmap done\n");

	/*print initial value of registers*/
	printf("GIER: %08x\n",*((uint32_t *)(buttons + GIER)));
	printf("IP_IER: %08x\n",*((uint32_t *)(buttons + IP_IER)));
	printf("IP_ISR: %08x\n",*((uint32_t *)(buttons + IP_ISR)));
	
	/*enable all interupts*/
	*(uint32_t *)(buttons + GIER) = 0x80000000;
	*(uint32_t *)(buttons + IS_IER) = 0x1;
	*(uint32_t *)(buttons + IS_ISR) = 0x1;
	printf("GIER: %08x\n", *((uint32_t *)(buttons + GIER)));
        printf("IP_IER: %08x\n", *((uint32_t *)(buttons + IP_IER)));
        printf("IP_ISR: %08x\n", *((uint32_t *)(buttons + IP_ISR)));
	printf("Checkpoint 3: enable all interrupts done\n");

	/*enable UIO interrupts*/
	int irq = 1;
	write(fd, &irq, sizeof(irq));
	printf("Checkpoint 4: enable UIO interrupt done\n");

	/*read from GPIO*/
	*(uint32_t *)(buttons + GPIO_DATA_OFFSET) = 31;
	printf("Checkpoint 5: read from GPIO done\n");

	/*print interrupts value*/
	printf("GIER: %08x\n", *((uint32_t *)(buttons + GIER)));
	printf("IP_IER: %08x\n", *((uint32_t *)(buttons + IP_IER)));
	printf("IP_ISR: %08x\n", *((uint32_t *)(buttons + IP_ISR)));
	printf("Checkpoint 6: print interrupts value done\n");

	munmap(buttons, GPIO_MAP_SIZE);
	printf("Checkpoint 7: munmap done\n");

	return 0;
}

