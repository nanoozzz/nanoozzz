/*Written by Nguyen Bao Nguyen Le | nguyenle.18082003@gmail.com
  This program controls GPIO LEDs using inputs from GPIO button 
  (polling method) via GPIO subsystem.
  Board used: Zynq UltraScale+ MPSoC ZCU102*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSTRLEN 50
#define NUMGPIO 10
#define STARTBUT 495
#define STARTLED 500
#define NUMBUT 5
#define NUMLED 5

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

void do_export(char gpio_export[]);
void do_set_output(char gpio_direction[][MAXSTRLEN+1]);
void do_set_interrupt(char gpio_interrupt[][MAXSTRLEN+1]);
void do_turn_on(char gpio_value[]);
void do_turn_off(char gpio_value[]);
void do_control(char gpio_value[][MAXSTRLEN+1]);

void do_export(char gpio_export[]){
	for (int i = STARTBUT; i < STARTBUT + NUMGPIO; i++) {
		FILE *export_file = fopen(gpio_export, "w");
		fprintf(export_file, "%d", i);
		fclose(export_file);
	}
}

void do_set_output(char gpio_direction[][MAXSTRLEN+1]){
	for (int i = STARTLED; i < STARTLED + NUMLED; i++) {
		FILE *direction_file = fopen(gpio_direction[i], "w");
		fprintf(direction_file, "%s", "out");
		fclose(direction_file);
	}
}

void do_set_interrupt(char gpio_interrupt[][MAXSTRLEN+1]){
	for (int i = 0; i < NUMBUT; i++){
		FILE *interrupt_file = fopen(gpio_interrupt[i], "w");
		fprintf(interrupt_file, "s", "rising");
		fclose(interrupt_file);
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
	int sw[NUMBUT], led[NUMLED];
	int next_value;
	while(1){
		for (int i = 0; i < NUMBUT; i++) {
			FILE *value_file = fopen(gpio_value[i + STARTBUT], "r");
			fscanf(value_file, "%d", &next_value);
			sw[i] = next_value;
			fclose(value_file);
		}
		for (int i = 0; i < NUMLED; i++) {
			if (sw[i]) {
			 	do_turn_on(gpio_value[STARTLED + i]);
			}
			else {
				do_turn_off(gpio_value[STARTLED + i]);
			}
		}
	}		
}

int main(int argc, char *argv[])
{
        do_export(gpio_export);
        do_set_output(gpio_direction);
        do_set_interrupt(gpio_interrupt);
        do_control(gpio_value);
        return 0;
}

