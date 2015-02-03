/**
 * \file XY4Burn.c
 *
 *  Author: Sam Kim
 *
 *  Called from LabVIEW, burns the pulse sequence for XY-4
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
 *  16  Window 5a channels (X)
 *  17  Window 5c channels (Y)
 *  18  Window 8 channels
 *  19  Window 9 channels
 *  20  Window 10 channels
 *  21  Window 11 channels
 *  22  Window 12 channels
 *  23  Number of scans
 *  24  Number of pulse sequence reptitions
 *  25  Number of delay times per scan
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
	int start, scan_loop, pulse_loop, status;
	int num_scans, num_sequences, num_delay_times;
       //num_delay_times - the number of times to pulse the MW
	   //num_reps - number of repetitions of pulse sequence
	   //num_scans - number of runs for min->max
	int numBoards;
	double window_time[13];    //12+1 to make it 1-indexed
	double min_tau, max_tau, tau;
	int window_channel[13];    //12+1 to make it 1-indexed
	int window_channel4, window_channelX, window_channelY;

	//Uncommenting the line below will generate a debug log in your current
	//directory that can help debug any problems that you may be experiencing   
	//pb_set_debug(1); 
	
	if (argc != 26) {
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
    //Window 4 min/max time
    min_tau = atof(argv[4]) * 1e9;
    max_tau = atof(argv[5]) * 1e9;
    //Window 5 time
    window_time[5] = atof(argv[6]) * 1e9;
    //Window 6=4, 7=3
    //Window times 8-12
    for(i=8; i<13; i++) {
        window_time[i] = atof(argv[i-1]) * 1e9;
    }
    
    //Window channels 1-4
    for(i=1; i<5; i++) {
        window_channel[i] = atoi(argv[i+11]);
        if(i != 4 && window_time[i] > 5*2) {
            window_channel[i] = ON | window_channel[i];
        }
    }
	window_channelX = ON | atoi(argv[16]);
	window_channelY = ON | atoi(argv[17]);
    //Window channels 8-12
    for(i=8; i<13; i++) {
        window_channel[i] = atoi(argv[i+10]);
        if(window_time[i] > 5*2) {
            window_channel[i] = ON | window_channel[i];
        }
    }
    num_scans = atoi(argv[23]);
    num_sequences = atoi(argv[24]);
    num_delay_times = atoi(argv[25]);
	
	window_time[7] = window_time[3];
	window_channel[7] = window_channel[3];
	window_channel[6] = window_channel[4];
		
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
        pulse_loop = pb_inst(window_channel4, LOOP, num_sequences, tau * ns);
        //Pulse sequences X-T-Y-T-X-T-Y
		pb_inst(window_channelX, CONTINUE, 0, window_time[5] * ns);
		pb_inst(window_channel4, LOOP, num_sequences, 2*tau * ns);
		pb_inst(window_channelY, CONTINUE, 0, window_time[5] * ns);
		pb_inst(window_channel4, LOOP, num_sequences, 2*tau * ns);
		pb_inst(window_channelX, CONTINUE, 0, window_time[5] * ns);
		pb_inst(window_channel4, LOOP, num_sequences, 2*tau * ns);
		pb_inst(window_channelY, CONTINUE, 0, window_time[5] * ns);
		//Window 6=Window 4
		pb_inst(window_channel4, END_LOOP, pulse_loop, tau * ns);
		
		//Windows 7-12 - counting photons
		for(j=7; j<13; j++) {
			pb_inst(window_channel[j], CONTINUE, 0, window_time[j]*ns);
		}
    }
	pb_inst(0x0, END_LOOP, scan_loop, 10*ns);
    pb_inst(0x0, STOP, 0, 10*ns);
	
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
