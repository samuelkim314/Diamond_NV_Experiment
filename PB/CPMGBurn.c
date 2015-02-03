/**
 * \file CPMGBurn.c
 *
 *  Author: Sam Kim
 *
 *  Called from LabVIEW, burns the pulse sequence for CPMG
 *
 *  Arg Description
 *  1   Window 1 time
 *  2   Window 2 time
 *  3   Window 3 time
 *  4   Window 4 min time
 *  5   Window 4 max time
 *  6   Window 5 time
 *  7   Window 8 time
 *  8   Window 9 time
 *  9   Window 10 time
 *  10  Window 11 time
 *  11  Window 12 time
 *  12  Window 1 channels
 *  13  Window 2 channels
 *  14  Window 3 channels
 *  15  Window 4 channels
 *  16  Window 5 channels
 *  17  Window 8 channels
 *  18  Window 9 channels
 *  19  Window 10 channels
 *  20  Window 11 channels
 *  21  Window 12 channels
 *  22  Number of scans
 *  23  Number of pulse sequence reptitions
 *  24  Number of delay times per scan
 */

#include <stdio.h>
#include <stdlib.h>

#define PBESRPRO
#define CLOCK 500.0
#include "spinapi.h"

int detect_boards();
int select_board(int numBoards);

int main(int argc, char *argv[])
{
	int start, scan_loop, pulse_loop, status, error;
	int num_scans, num_pulses, num_delay_times;
       //num_delay_times - the number of times to pulse the MW
	   //num_reps - number of repetitions of pulse sequence
	   //num_scans - number of runs for min->max
	int numBoards;
	double window_time[13];    //12+1 to make it 1-indexed
	double min_tau, max_tau, tau;  //this is actually tau/2 (depending on notation)
	int window_channel[13];    //12+1 to make it 1-indexed
	int window_channel4;

	//Uncommenting the line below will generate a debug log in your current
	//directory that can help debug any problems that you may be experiencing   
	//pb_set_debug(1); 
	
	if (argc != 25) {
       printf("Wrong number of arguments");
       return -1;
    }

	/*If there is more than one board in the system, have the user specify. */
	if ((numBoards = detect_boards()) > 1) {
		select_board(numBoards);
	}

	if (pb_init() != 0) {
		printf("Error initializing board: %s\n", pb_get_error());
		system("pause");
		return -1;
	}
	
	int i,j;
	
	//Window times 1-3
	for(i=1; i<4; i++) {
        window_time[i] = atof(argv[i]) * 1e9;
    }
    //Min/max of window 4
    min_tau = atof(argv[4]) * 1e9;
    max_tau = atof(argv[5]) * 1e9;
    //Window time 5 (pi pulse)
    window_time[5] = atof(argv[6]) * 1e9;
    //Window time 6 = 4, window time 7=3
    //Window times 8-12
    for(i=8; i<13; i++) {
        window_time[i] = atof(argv[i-1]) * 1e9;
    }
    
    //Window channels 1-5
    for(i=1; i<6; i++) {
        window_channel[i] = atoi(argv[i+11]);
        if(i != 4 && window_time[i] > 5*2) {
            window_channel[i] = ON | window_channel[i];
        }
    }
    //Window channels 8-12
    for(i=8; i<13; i++) {
        window_channel[i] = atoi(argv[i+9]);
        if(window_time[i] > 5*2) {
            window_channel[i] = ON | window_channel[i];
        }
    }
    num_scans = atoi(argv[22]);
    num_pulses = atoi(argv[23]);
    num_delay_times = atoi(argv[24]);
	
	window_time[7] = window_time[3];
	window_channel[7] = window_channel[3];
	window_channel[6] = window_channel[4];
	
	/*for(i=1; i<13; i++) {
        printf("%f %d\n", window_time[i], window_channel[i]);
    }*/
		
	// Tell the driver what clock frequency the board has (in MHz)
	pb_core_clock(CLOCK);

	pb_start_programming(PULSE_PROGRAM);
	
	scan_loop = pb_inst(0x0, LOOP, num_scans, 10*ns);
	for(i=0; i<num_delay_times; i++) {
        pb_inst(window_channel[1], CONTINUE, 0, window_time[1] * ns);
        pb_inst(window_channel[2], CONTINUE, 0, window_time[2] * ns);
        pb_inst(window_channel[3], CONTINUE, 0, window_time[3] * ns);
            
        //Calculating tau
		tau = (max_tau - min_tau)/(num_delay_times-1) * i + min_tau;
        if (tau > 5*2) {
            window_channel4 = ON | window_channel[4];
        }
        else {
            window_channel4 = window_channel[4];
        }
        pulse_loop = pb_inst(window_channel4, LOOP, num_pulses, tau * ns);
        
		pb_inst(window_channel[5], CONTINUE, 0, window_time[5] * ns);
		//Window 6=Window 4
		pb_inst(window_channel4, END_LOOP, pulse_loop, tau * ns);
		//Windows 7-12
		for(j=7; j<13; j++) {
			pb_inst(window_channel[j], CONTINUE, 0, window_time[j]*ns);
		}
    }
	pb_inst(0x0, END_LOOP, scan_loop, 10*ns);
    pb_inst(0x0, STOP, 0, 10*ns);
	
	if (error < 0) {
        return -1;
    }
	
	pb_stop_programming();

	return 0;
}

int detect_boards()
{
	int numBoards;

	numBoards = pb_count_boards();	/*Count the number of boards */

	if (numBoards <= 0) {
		printf
		    ("No Boards were detected in your system. Verify that the board "
		     "is firmly secured in the PCI slot.\n\n");
		system("PAUSE");
		exit(-1);
	}

	return numBoards;
}

int select_board(int numBoards)
{
	int choice;

	do {
		printf
		    ("Found %d boards in your system. Which board should be used? "
		     "(0-%d): ", numBoards, numBoards - 1);
		fflush(stdin);
		scanf("%d", &choice);

		if (choice < 0 || choice >= numBoards) {
			printf("Invalid Board Number (%d).\n", choice);
		}
	} while (choice < 0 || choice >= numBoards);

	pb_select_board(choice);
	printf("Board %d selected.\n", choice);

	return choice;
}
