#include "parse.h"


int parseMPDU(uint8_t buffer, MPDU *mpdu){
    int curBufSpot = 1;

   		uint8_t handleLen = buffer[curBufSpot];
    	char currHandle[handleLen + 1];  // +1 for null terminator

		curBufSpot++;

    	memcpy(currHandle, buffer + curBufSpot, handleLen);
    	currHandle[handleLen] = '\0'; 
    	curBufSpot += handleLen;

    	// check that dest number is 1 (should always be that for M)
    	uint8_t numDest = buffer[curBufSpot];
		curBufSpot++;
    	if (numDest != 1) {
        	printf("Error: Only one destination allowed, given %d\n", numDest);
        	return;
    	}

    	// get out the destination
    	uint8_t destLen = buffer[curBufSpot];
		curBufSpot++;
    	char destHandle[destLen + 1];
    	memcpy(destHandle, buffer + curBufSpot, destLen);
    	destHandle[destLen] = '\0'; 
    	curBufSpot += destLen;

    	//get the rest of the message
    	char *messageToPass = (char *)(buffer + curBufSpot); 

    	//for testing
    	printf("PARSED Sender Handle: %s\n", currHandle);
    	printf("PARSED Destination Handle: %s\n", destHandle);
    	printf("PARSED Message: %s\n", messageToPass);
}