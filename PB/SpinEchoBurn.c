/**
 * \file SpinEchoBurn.c
 *
 *  Author: Sam Kim
 *
 *  Called from LabVIEW, burns the pulse sequence for Spin Echo
 *
 *  args (25):
 *  window times 1-3,5,7-12
 *  window 4 min time
 *  window 4 max time
 *  window channels 1-5,7-12
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
	double window_time[12];
	double min_tau, max_tau;
	int tau;
	int window_channel[12];
	int window_channel3;

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
	//Window times 1-3, in ns
    for(i=0; i<3; i++) {
        window_time[i] = atof(argv[i+1]) * 1e9;
    }
    //Window 5
    window_time[4] = atof(argv[4]) * 1e9;
    //Window times 7-12
    for(i=6; i<12; i++) {
        window_time[i] = atof(argv[i-1]) * 1e9;
    }
    //Window channels 1-5
	for(i=0; i<5; i++) {
        if(i!=3 && i!=5) {
            window_channel[i] = atoi(argv[i+13]);
    
            if (window_time[i] > 5*2) {
                window_channel[i] = ON | window_channel[i];
            }
        }
    }
    //Window channels 7-12
    for(i=6; i<12; i++) {
        window_channel[i] = atoi(argv[i+13-1]);  
        if (window_time[i] > 5*2) {
            window_channel[i] = ON | window_channel[i];
        }
    }
    min_tau = atof(argv[11]) * 1e9;
    max_tau = atof(argv[12]) * 1e9;
    window_channel[3] = atoi(argv[16]);
    window_channel[5] = window_channel[3];
    num_scans = atoi(argv[24]);
    num_times = atoi(argv[25]);
	
	// Tell the driver what clock frequency the board has (in MHz)
	pb_core_clock(CLOCK);

	pb_start_programming(PULSE_PROGRAM);
	
	// Loop through all the time axis points
	scan_loop = pb_inst(0x0, LOOP, num_scans, 10 * ns);
	// For each time axis point
    for(i=0; i<num_times; i++) {
        pb_inst(window_channel[0], CONTINUE, 0, window_time[0] * ns);
        pb_inst(window_channel[1], CONTINUE, 0, window_time[1] * ns);
        pb_inst(window_channel[2], CONTINUE, 0, window_time[2] * ns);
        
        //Calculate tau in each loop
        tau = (int) ((max_tau - min_tau)/(num_times-1)*i + min_tau);
        // adjusts the channel (ON) for tau based on how long it is
        if (tau > 5*2) {
            window_channel3 = ON | window_channel[3];
        }
        else {
            window_channel3 = window_channel[3];
        }
        pb_inst(window_channel3, CONTINUE, 0, tau * ns);    // tau
        
        
        pb_inst(window_channel[4], CONTINUE, 0, window_time[4] * ns);   //pi pulse
        pb_inst(window_channel3, CONTINUE, 0, tau * ns);    //tau
        
        //Windows 7-12
        for(j=6;j<12;j++) {
            pb_inst(window_channel[j], CONTINUE, 0, window_time[j] * ns);
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
