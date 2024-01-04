/*This program controls GPIO LEDS on ZCU102 board*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUMLED 8
#define MAXSTRLEN 50

char led_path_trigger[][MAXSTRLEN+1] = {
	"/sys/class/leds/gpio-led-DS37/trigger",
	"/sys/class/leds/gpio-led-DS38/trigger",
	"/sys/class/leds/gpio-led-DS39/trigger",
	"/sys/class/leds/gpio-led-DS40/trigger",
	"/sys/class/leds/gpio-led-DS41/trigger",
	"/sys/class/leds/gpio-led-DS42/trigger",
	"/sys/class/leds/gpio-led-DS43/trigger",
	"/sys/class/leds/gpio-led-DS44/trigger"
};

char led_path_brightness[][MAXSTRLEN+1] = {
        "/sys/class/leds/gpio-led-DS37/brightness",
        "/sys/class/leds/gpio-led-DS38/brightness",
        "/sys/class/leds/gpio-led-DS39/brightness",
        "/sys/class/leds/gpio-led-DS40/brightness",
        "/sys/class/leds/gpio-led-DS41/brightness",
        "/sys/class/leds/gpio-led-DS42/brightness",
        "/sys/class/leds/gpio-led-DS43/brightness",
        "/sys/class/leds/gpio-led-DS44/brightness"
};

void remove_trigger(char led_path_trigger[][MAXSTRLEN+1]);
void turn_on(char *led_path_brightness);
void turn_off(char *led_path_brightness);
void control(char led_path_brightness[][MAXSTRLEN+1]);

int main(int argc, char* argv[]){
	remove_trigger(led_path_trigger);	
	/*char *temp_led_path_brightness[NUMLED];

	for (int i = 0; i < NUMLED; i++) {
	    	temp_led_path_brightness[i] = led_path_brightness[i];
	}

	control(temp_led_path_brightness);*/
	control(led_path_brightness);	
	return 0;
}

void remove_trigger(char led_path_trigger[][MAXSTRLEN+1]){
	for(int i = 0; i < NUMLED; i++){
		FILE *trigger_file = fopen(led_path_trigger[i], "w");
		/*if(trigger_file == NULL){
			perror("Error opening trigger file");
			exit(EXIT_FAILURE);
		}*/

		fprintf(trigger_file, "none");
		fclose(trigger_file);
	}
}

void turn_on(char *led_path_brightness){
	for(int i = 0; i < NUMLED; i++){
		FILE *brightness_file = fopen(led_path_brightness, "w");
		/*if(brightness_file == NULL){
			perror("Error opening brightness file");
			exit(EXIT_FAILURE);
		}*/

		fprintf(brightness_file, "1");
		fclose(brightness_file);
	}
}

void turn_off(char *led_path_brightness){
        for(int i = 0; i < NUMLED; i++){
                FILE *brightness_file = fopen(led_path_brightness, "w");
                /*if(brightness_file == NULL){
                        perror("Error opening brightness file");
                        exit(EXIT_FAILURE);
                }*/

                fprintf(brightness_file, "0");
                fclose(brightness_file);
        }
}

void control(char led_path_brightness[][MAXSTRLEN+1]){
	int brightness[NUMLED];
	int next_brightness;

	while(1){
		for(int i = 0; i < NUMLED; i++){
			FILE *_brightness_file = fopen(led_path_brightness[i], "r");
			fscanf(_brightness_file, "%d", &next_brightness);
			brightness[i] = next_brightness;
			fclose(_brightness_file);
		}
		for(int i = 0; i < NUMLED; i++){
			if(brightness[i] == 0){
				turn_on(led_path_brightness[i]);
			}
			else{
				turn_off(led_path_brightness[i]);
			}
			sleep(2);
		}
	}
}
