/**
 * \file ESRFreq.c
 *
 *  Author: Sam Kim
 *
 *  Called from LabVIEW, scans microwave over range of frequencies
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
	int start, scan_loop, freq_loop, status;
	int num_scans, num_freqs;
	int numBoards;
	double window_time[4];
	int window_channel[4];

	//Uncommenting the line below will generate a debug log in your current
	//directory that can help debug any problems that you may be experiencing   
	//pb_set_debug(1); 

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
	for(i=0; i<4; i++) {
        window_channel[i] = ON | 0x8;
    }
    
    window_time[0] = 10e-6;
    window_time[1] = 1e-3;
    window_time[2] = 1e-3;
    window_time[3] = 1e-3;
    
    num_scans = 10;
    num_freqs = 1;
	
	// Tell the driver what clock frequency the board has (in MHz)
	pb_core_clock(CLOCK);

	pb_start_programming(PULSE_PROGRAM);
	
	// Outer loop
	scan_loop = pb_inst (0x0, LOOP, num_scans, 2 * ns);
	//Inner loop
	freq_loop = pb_inst (window_channel[0], LOOP, num_freqs, window_time[0] * ns);
	//Window 2
	pb_inst(window_channel[1], CONTINUE, 0, window_time[1] * ns);
	//Window 3
	pb_inst(window_channel[2], CONTINUE, 0, window_time[2] * ns);
	//Window 4
	pb_inst(window_channel[3], END_LOOP, freq_loop, window_time[3] * ns);
	//End outer loop
	pb_inst(0x0, END_LOOP, scan_loop, 2*ns);
	
	pb_inst(0x0, BRANCH, scan_loop, 2*ns);

	pb_stop_programming();

	// Trigger the pulse program
	pb_reset();
	pb_start();

	//Read the status register
	status = pb_read_status();
	printf("status: %d \n", status);
	printf(pb_status_message());
	printf("\n");
	system("pause");

	pb_stop();
	pb_close();

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
