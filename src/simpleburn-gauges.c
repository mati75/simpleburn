#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


void cdrecordgauge(int trackscount, int trackssizes[]) {
	int totalsize; //MB
	int *writtensizes;
	int percent, lastdisplayedpercent;
	int inputchar;
	char inputline[80];
	int inputnextcharpos;
	int inputtrack, inputwritten, inputtracksize;
	int i;
	char echocommand[9];
	
	totalsize = 0;
	for (i=0; i<trackscount; i++)
		totalsize += trackssizes[i];
	
	writtensizes = (int *) malloc((trackscount) * sizeof(int));
	writtensizes[0] = 0;
	for (i=1; i<trackscount; i++)
		writtensizes[i] = writtensizes[i - 1] + trackssizes[i - 1];
	
	while (inputchar = fgetc(stdin)) { if (inputchar == 13) break; } //waiting loop
	
	inputnextcharpos=0;
	inputline[inputnextcharpos] = '\0';
	inputchar = fgetc(stdin);
	lastdisplayedpercent = -1;
	while (inputchar != EOF) {
		if (inputchar == 13 || inputchar == 10) { //return to line begin
			if (sscanf(inputline, "Track %2d: %4d of %4d MB written", &inputtrack, &inputwritten, &inputtracksize)) {
				percent = ((inputwritten * 100) / totalsize) +(writtensizes[inputtrack - 1] * 100) / totalsize;
				if (percent != lastdisplayedpercent) {
					sprintf(echocommand, "echo %d", percent);
					system(echocommand);
					lastdisplayedpercent = percent;
				}
			}
			inputnextcharpos=0;
		} else {
			inputline[inputnextcharpos] = (char) inputchar;
			inputnextcharpos++;
		}
	inputline[inputnextcharpos] = '\0';
	inputchar = fgetc(stdin);
	}
	
	free(writtensizes);
}


void cdda2wavgauge(int previouspercent, int trackpercent, char *fifoname) {
	bool starting, endoftrack;
	char inputline[80];
	FILE *fifofile;
	int percent, lastdisplayedpercent;
	char echocommand[9];
	char c;
	char inputchars[4];
	int length;
	
	fifofile = fopen(fifoname, "r");
	
	starting = false;
	while (! starting) {
		fgets(inputline, 80, fifofile);
		inputline[13] = '\0';
		if (strncmp(inputline, "percent_done:", 13) == 0) {
			starting = true;
		}
	}
	
	lastdisplayedpercent = -1;
	endoftrack = false;
	inputchars[0] = '\0';
	c = fgetc(fifofile);
	while ((c != EOF) && (! endoftrack)) {
		switch(c) {
		case 37: //'%'
			percent = (atoi(inputchars) * trackpercent) / 100 + previouspercent;
			if (percent !=  lastdisplayedpercent && percent <= 100) {
				sprintf(echocommand, "echo %d", percent);
				system(echocommand);
				lastdisplayedpercent = percent;
			}
			inputchars[0] = '\0';
			break;
		case 32:
			break;
		case 13:
			break;
		default:
			if ((c >= 48) && (c <= 57)) { //[0-9]
				length = strlen(inputchars);
				inputchars[length] = c;
				inputchars[length+1] = '\0';
			} else {
				endoftrack = true;
			}
		}
		c = fgetc(fifofile);
	}
	
	fclose(fifofile);
}


int main(int argc, char *argv[]) {
	int i;
	int *args;
	
	if (argc < 2) { //basic check to prevent a user running this program
		printf("usage: intended for SimpleBurn scripts\n");
		return -1;
	}
	
	args = (int *) malloc(argc * sizeof(int));
	
	if (strncmp(argv[1], "cdda2wav", 8) == 0) {
		cdda2wavgauge(atoi(argv[2]), atoi(argv[3]), argv[4]);
	} else {
		if (strncmp(argv[1], "cdrecord", 8) == 0) {
			for (i=2; i<argc; i++)
				args[i-2] = atoi(argv[i]);
			cdrecordgauge(argc - 2, args);
		}
	}
	
	free(args);
}

