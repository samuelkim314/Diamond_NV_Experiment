/**
 * \file HoldChannel.c
 *
 *  Author: Sam Kim
 *
 *  Called from LabVIEW, holds the channel
 *
 *  Used for holding Green laser (and MW) while optimizing
 *  Max of 8.59 s
 *  
 *  args:
        window_time (ms)
        window_channel
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
	int status;
	int numBoards;
	double window_time;
	int window_channel;
	int error, loop, num_runs;
	int max_delay = 500;
	int max_runs = 1e6;

	//Uncommenting the line below will generate a debug log in your current
	//directory that can help debug any problems that you may be experiencing   
	//pb_set_debug(1); 
	
	if (argc != 3) {
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
	
	window_time = atof(argv[1]) * 1e6; //convert from ms to ns
	window_channel = atoi(argv[2]);
	if (window_time > 5*2) {
        window_channel = ON | window_channel;
    }
	
	// Tell the driver what clock frequency the board has (in MHz)
	pb_core_clock(CLOCK);

	pb_start_programming(PULSE_PROGRAM);
    
    if (window_time > max_delay * max_runs) {
        num_runs = (int) (window_time/max_delay/max_runs);
        loop = pb_inst(window_channel, LOOP, num_runs, 10 * ns);
        pb_inst(window_channel, LONG_DELAY, max_runs, max_delay * ns);
        pb_inst(window_channel, END_LOOP, loop, 10 * ns);
        window_time -= num_runs * max_delay * max_runs;
    }
    
    if (window_time < max_delay) {
        pb_inst(window_channel, CONTINUE, 0, window_time * ns);
    }
    else {
        num_runs = (int) (window_time/max_delay + 1);
        pb_inst(window_channel, LONG_DELAY, num_runs, max_delay * ns);
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
