/**
 * \file SpectrumBurn.c
 *  Author: Sam Kim
 *  Called from LabVIEW, burns the Exc Res Spectrum pulse sequence
 *
 *  args:
        argv[1-6]:  window times 1-6
        argv[7-12]: window channels 1-6
        argv[13]: number of repetitions
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
	int num_scans;
	int numBoards;
	double window_time[6];
	int window_channel[6];

	//Uncommenting the line below will generate a debug log in your current
	//directory that can help debug any problems that you may be experiencing   
	//pb_set_debug(1); 
	
	if (argc != 14) {
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
	for(i=0; i<6; i++) {
        window_time[i] = atof(argv[i+1]) * 1e9; //convert to ns
        window_channel[i] = atoi(argv[i+7]);

        if (window_time[i] > 5*2) {
            window_channel[i] = ON | window_channel[i];
        }
    }
    num_scans = atoi(argv[13]);
	
	// Tell the driver what clock frequency the board has (in MHz)
	pb_core_clock(CLOCK);

	pb_start_programming(PULSE_PROGRAM);
	
	// Repetition loop, window 1
	scan_loop = pb_inst(window_channel[0], LOOP, num_scans, window_time[0] * ns);
	//Window 2-5
	for(i=1; i<5; i++) {
        pb_inst(window_channel[i], CONTINUE, 0, window_time[i] * ns);
    }
	//End loop, window 6
	pb_inst(window_channel[5], END_LOOP, scan_loop, window_time[5] * ns);
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
