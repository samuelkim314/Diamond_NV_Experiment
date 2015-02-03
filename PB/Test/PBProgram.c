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
	int start, status;
	int numBoards;

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

	// Tell the driver what clock frequency the board has (in MHz)
	pb_core_clock(CLOCK);

	pb_start_programming(PULSE_PROGRAM);
	
	start = pb_inst (0x0, CONTINUE, 0, 10 * ns);
	pb_inst (ON | 0x8, CONTINUE, 0, 4e6 * ns);
	pb_inst(0x0, BRANCH, start, 1e6 * ns);
	
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
