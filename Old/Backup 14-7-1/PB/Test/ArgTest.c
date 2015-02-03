/**
 * \file ArgTest.c
 *
 *  Author: Sam Kim
 *
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int start, scan_loop, freq_loop, status;
	int numBoards;
	double window_time[4];
	
	if (argc != 5) {
       printf("Wrong number of arguments");
       return -1;
    }
	
	int i;
	for(i=0; i<4; i++) {
        window_time[i] = atof(argv[i+1]);
    }
    
    for(i=0; i<4; i++) {
        printf("%1.9lf\n", window_time[i]);
    }

	system ("pause");
	return 0;
}
