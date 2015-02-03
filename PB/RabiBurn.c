/**
 * \file RabiBurn.c
 *
 *  Author: Sam Kim
 *
 *  Called from LabVIEW, burns the pulse sequence for Rabi
 *
 *  args:
 *  window times 1,2
 *  window 3 min_time
 *  window time 4-8
 *  window 3 max time
 *  window channels 1-8
 *  number of scans (samples before track)
 *  number of times (samples) per scan (time axis points)
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
	int start, scan_loop, status;
	int num_scans, num_times; //num_times - the number of times to pulse the MW
	                           //num_scans - number of runs for min->max
	int numBoards;
	double window_time[9]; //window_time[0,1,3-7] are windows i+1
	                       //window_time[2] is min_time of window 3
	                       //window_time[8] is max_time of window 3
	double min_time, max_time, mw_time;
	int window_channel[8];
	int window_channel3;

	//Uncommenting the line below will generate a debug log in your current
	//directory that can help debug any problems that you may be experiencing   
	//pb_set_debug(1); 
	
	if (argc != 20) {
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
	
	int i;
	for(i=0; i<8; i++) {
        window_time[i] = atof(argv[i+1]) * 1e9; //convert to ns
        window_channel[i] = atoi(argv[i+10]);

        if (window_time[i] > 5*2) {
            window_channel[i] = ON | window_channel[i];
        }
    }
    window_time[8] = atof(argv[9]) * 1e9;   //convert to ns
    min_time = window_time[2];  //already in ns
    max_time = window_time[8];  //already in ns
    num_scans = atoi(argv[18]);
    num_times = atoi(argv[19]);
	
	// Tell the driver what clock frequency the board has (in MHz)
	pb_core_clock(CLOCK);

	pb_start_programming(PULSE_PROGRAM);
	
	scan_loop = pb_inst(0x0, LOOP, num_scans, 50 * ns);
	for(i=0; i<num_times; i++) {
        pb_inst(window_channel[0], CONTINUE, 0, window_time[0] * ns);
        pb_inst(window_channel[1], CONTINUE, 0, window_time[1] * ns);
        
        mw_time = (max_time - min_time)/(num_times-1)*i + min_time;
        if (mw_time > 5*2) {
            window_channel3 = ON | window_channel[2];
        }
        else {
            window_channel3 = window_channel[2];
        }
        pb_inst(window_channel3, CONTINUE, 0, mw_time * ns);
        
        pb_inst(window_channel[3], CONTINUE, 0, window_time[3] * ns);
        pb_inst(window_channel[4], CONTINUE, 0, window_time[4] * ns);
        pb_inst(window_channel[5], CONTINUE, 0, window_time[5] * ns);
        pb_inst(window_channel[6], CONTINUE, 0, window_time[6] * ns);
        pb_inst(window_channel[7], CONTINUE, 0, window_time[7] * ns);
    }
    pb_inst(0x0, END_LOOP, scan_loop, 50*ns);
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
