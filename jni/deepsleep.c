/*
 * deepsleep forces the Tolino device to deep sleep when the screen is turned "off".
 * 
 * Version:  0.3
 * Date: 2014-09-13
 * Authors: Martin Zwicknagl <martin.zwicknagl@kirchbichl.net>
 *          Stefan Rado <tolino@sradonia.net>
 * 
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define VERSION "0.3, 2014-09-13"

#define PROP_BOOT_COMPLETE "sys.boot_completed"

#define FILE_SYS_POWER_STATE               "/sys/power/state"
#define FILE_CPUFREQ_GOVERNOR              "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
#define FILE_ONDEMAND_UPTHRESHOLD          "/sys/devices/system/cpu/cpufreq/ondemand/up_threshold"
#define FILE_ONDEMAND_SAMPLING_DOWN_FACTOR "/sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor"
#define FILE_ONDEMAND_IGNORE_NICE_LOAD     "/sys/devices/system/cpu/cpufreq/ondemand/ignore_nice_load"

#define MAX_PROPVALUE 255
#define MAX_BUFFER 1024

#define POLL_TIME 750000 // in us


void getProp(char* property, char* propValue) {
	FILE *pipe_reader;
	char str[MAX_PROPVALUE];

	sprintf(str, "getprop %s", property);

	if ((pipe_reader = popen(str, "r")) == NULL) {
		printf("Error running getprop\n");
		exit(1);
	}

	fgets(propValue, MAX_PROPVALUE, pipe_reader);
	pclose(pipe_reader);
}

void setProp(char* property, char* propValue) {
	FILE *pipe_reader;
	char str[MAX_BUFFER];

	sprintf(str, "setprop %s %s", property, propValue);
	if ((pipe_reader = popen(str, "r")) == NULL) {
		printf("Error running setprop\n");
		exit(1);
	}

	pclose(pipe_reader);
}

void writeToFile(char *file, char *data) {
	FILE *fp = fopen(file, "w"); // write mode
	if (fp == NULL) {
		printf("Error opening file %s\n", file);
		exit(2);
	}

	printf("Writing \"%s\" to %s\n", data, file);
	fprintf(fp, "%s", data);

	fclose(fp);
}

int getStatus() {
	int status;

	FILE *pipe_reader;
	if ((pipe_reader = popen("dumpsys power", "r")) == NULL) {
		printf("Error running dumpsys\n");
		exit(3);
	}

	char str[MAX_BUFFER];
	while (!feof(pipe_reader)) {
		fgets(str, MAX_BUFFER, pipe_reader);

		if (strstr(str, "mPowerState=") != NULL) {
			if (strstr(str, "mPowerState=0") != NULL) {
				status = 0;
			} else {
				status = 1;
			}
			break;
		}
	}

	pclose(pipe_reader);

	return status;
}


int main(int argc, char* argv[]) {
	char propValue[MAX_PROPVALUE];
	int oldStatus, status;

	printf("DeepSleep for Tolino - version " VERSION "\n");
	printf("by Martin Zwicknagl, put to GPL\n\n");

	// Wait for boot to complete
	getProp(PROP_BOOT_COMPLETE, propValue);
	if (strncmp(propValue, "1", 1) != 0) {
		printf("Waiting for boot to complete...\n");
		do {
			sleep(1);
			getProp(PROP_BOOT_COMPLETE, propValue);
		}
		while (strncmp(propValue, "1", 1) != 0);
	}

	// Set CPU governor
	printf("Setting CPU governor...\n");
	writeToFile(FILE_CPUFREQ_GOVERNOR, "ondemand");
	writeToFile(FILE_ONDEMAND_UPTHRESHOLD, "66");
	writeToFile(FILE_ONDEMAND_SAMPLING_DOWN_FACTOR, "2");
	writeToFile(FILE_ONDEMAND_IGNORE_NICE_LOAD, "0");

	// Power save loop
	printf("Starting power save loop...\n");
	status = getStatus();
	while (1) {
		oldStatus = status;
		status = getStatus();

		if (status != oldStatus) {
			printf("Status change: %u -> %u\n", oldStatus, status);
			if (status) {
				// sleep -> awake
				writeToFile(FILE_SYS_POWER_STATE, "on");
			} else {
				// awake -> sleep
				writeToFile(FILE_SYS_POWER_STATE, "mem");
			}
		}

		usleep(POLL_TIME);
	}

	return 0;
}
