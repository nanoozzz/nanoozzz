#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <inttypes.h>

#define GPIO_BASE_ADDRESS 0xA0010000  // Replace with your actual AXI GPIO base address
#define GPIO_DATA_OFFSET  0x0

int main() {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("Error opening /dev/mem");
        exit(EXIT_FAILURE);
    }

    uint32_t* gpio_base = (uint32_t*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE_ADDRESS);
    if (gpio_base == MAP_FAILED) {
        perror("Error mapping memory");
        close(fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read the GPIO data register
        uint32_t gpio_data = gpio_base[GPIO_DATA_OFFSET / sizeof(uint32_t)];

        // Extract button states
        uint8_t button_states = (gpio_data & 0x1F);  // Assuming 5 buttons (5 bits)

        // Print the binary representation
        printf("Binary representation: %05" PRIu8 "\n", button_states);
    }

    // Unmap memory and close file descriptor
    munmap(gpio_base, 4096);
    close(fd);

    return 0;
}

