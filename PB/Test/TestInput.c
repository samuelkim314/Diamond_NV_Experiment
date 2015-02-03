/**
 * \file TestInput.c
 *
 *  Author: Sam Kim
 *
 *  Called from LabVIEW, scans microwave over range of frequencies
 */

#include <stdio.h>
#include <stdlib.h>
#include "spinapi.h"

int main(int argc, char *argv[])
{
	int num_scans, num_freqs;
	double window_time[4];
	int window_channel[4];

	//Uncommenting the line below will generate a debug log in your current
	//directory that can help debug any problems that you may be experiencing   
	//pb_set_debug(1); 
	
	if (argc != 11) {
       printf("Wrong number of arguments");
       return -1;
    }

	
	int i;
	for(i=0; i<4; i++) {
        window_time[i] = atof(argv[i+1]);
        window_channel[i] = atof(argv[i+5]);

        /*
        if (window_time[i] > 5*2e-9) {
            window_channel[i] = ON | window_channel[i];
        }*/
    }
    num_scans = atoi(argv[9]);
    num_freqs = atoi(argv[10]);
	
    for(i=0;i<11;i++) {
        printf("%s ",argv[i]);
    }
    printf("\n");
    for(i=0;i<4;i++) {
        printf("%f ",window_time[i]);
    }
    printf("\n");
    
    for(i=0;i<6;i++) {
        printf("%d ",atoi(argv[i+5]));
    }
    for(i=0;i<6;i++) {
        printf("%d ",ON | atoi(argv[i+5]));
    }
    
    printf("\n");

	system ("pause");

	return 0;
}
