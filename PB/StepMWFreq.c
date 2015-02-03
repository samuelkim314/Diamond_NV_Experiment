/**
 * \file ESRFreq.c
 *
 *  Author: Sam Kim
 *
 *  Called from LabVIEW, steps MW frequency specified number of times
 * Used to reset MW frequency
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
	int start, freq_loop, status;
	int num_freqs;
	int numBoards;
	double window_time;
	int window_channel;

	//Uncommenting the line below will generate a debug log in your current
	//directory that can help debug any problems that you may be experiencing   
	//pb_set_debug(1); 
	
	if (argc != 4) {
       printf("Wrong number of arguments");
       system("pause");
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
	
	window_time = atof(argv[1]) * 1e9; //convert to ns
	window_channel = atoi(argv[2]);

    if (window_time > 5*2) {
        window_channel = ON | window_channel;
    }
	
    num_freqs = atoi(argv[3]);
	
	// Tell the driver what clock frequency the board has (in MHz)
	pb_core_clock(CLOCK);

	pb_start_programming(PULSE_PROGRAM);
	
	//Loop
	freq_loop = pb_inst(window_channel, LOOP, num_freqs, window_time * ns);
	//Window 2
	pb_inst(0x0, CONTINUE, 0, 100 * ns);
	//Window 4
	pb_inst(0x0, END_LOOP, freq_loop, 100 * ns);

	pb_stop_programming();
	
	pb_reset();
	pb_start();
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
