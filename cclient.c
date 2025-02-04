/******************************************************************************
* myClient.c
*
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

#include "networks.h"
#include "safeUtil.h"
#include "communicate.h"
#include "pollLib.h"
#include "cclient.h"
#include "makePDU.h"

int printPromptFlag = 1;
int handlesInList = 0;

int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	
	checkArgs(argc, argv);

	// only null terminates if shorter than so still need to do it after incase handle is 100 char
	strncpy(clientHandle, argv[1], HANDLE_MAX - 1);
	clientHandle[HANDLE_MAX - 1] = '\0';

	// set up the TCP Client socket
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);
	
	sendHandle(socketNum);

	clientControl(socketNum);
	
	close(socketNum);
	return 0;
}

// gets the buffer from readfromstdin, parses it properly, stores stuff into array of pointers
char** parseLine(uint8_t *buffer){

	static char* chunks[MAX_CHUNKS];
	int i = 0;
    char *firstChunk = strtok((char*)buffer, " ");

	if (firstChunk == NULL) {
            printf("Error: Missing Command.\n");
			printf("Known commands are: %%M, %%C, %%B, %%L\n");
            return NULL;
	}

	chunks[i] = firstChunk;
	i++;

	if (strcasecmp(firstChunk, "%M") == 0) { 
       char *handle = strtok(NULL, " "); 

		if (handle == NULL) {
            printf("Error: Missing destination handle.\n");
            printf("Usage: %%M <destination handle> <text>\n");
            return NULL;
        }

	   chunks[i] = handle;  
	   i++;

       char *message = strtok(NULL, "\n"); 

		// if there is no message treat it like its empty
	   if (message == NULL) {
            message = "";
        }


	   chunks[i] = message;
	   i++;
    }

	else if (strcasecmp(firstChunk, "%C") == 0) { 
       char *number = strtok(NULL, " ");

		if (number == NULL) {
        	printf("Error: Missing number of destination handles.\n");
        	printf("Usage: %%C <num-handles> <destination-handle> <destination-handle> <text>\n");
        	return NULL;
    	}


	   chunks[i] = number;
	   i++;

	   int numHandles = atoi(number);

	   if (numHandles < 2 || numHandles > 9){
		printf("Error: Too many or not enough destination handles.\nPlease enter between 2-9 destinations.\n");
        printf("Usage: %%C <num-handles> <destination-handle> <destination-handle> <text>\n");
        return NULL;
	   }

	   int j = 0;
	   for (j = 0; j < numHandles; j++){
		char* curHandle = strtok(NULL, " ");

		// Check if destination handle exists
        if (curHandle == NULL) {
            printf("Error: Missing destination handles.\n");
            printf("Usage: %%C <num-handles> <destination-handle> <destination-handle> <text>\n");
            return NULL;
        }

		chunks[i] = curHandle;
		i++;
	   }

       char *message = strtok(NULL, "\n"); 

		//check for if the message is empty
		if (message == NULL) {
        	message = "";
    	}

	   chunks[i] = message;
	   i++;
    }

	else if (strcasecmp(firstChunk, "%B") == 0) { 
       char *message = strtok(NULL, "\n"); 

	   //check for if the message is empty
		if (message == NULL) {
        	message = "";
    	}

	   chunks[i] = message;  
	   i++;
    }

	else if (strcasecmp(firstChunk, "%L") == 0) { 

		printPromptFlag = 0; 
		//first chunk already in the chunks array so do nothing.
	   return chunks;
    }

	else {
        //if its neither of these commands
        printf("Error: Unknown command '%s'.\n", firstChunk);
        printf("Known commands are: %%M, %%C, %%B, %%L\n");
        return NULL;
    }

	return chunks;
}

//keep for error checking at the end
void printChunks(char **chunks) {
    int i = 0;
    printf("Parsed Tokens:\n");
    while (chunks[i] != NULL) {
        printf("Token %d: %s\n", i, chunks[i]);
        i++;
    }
    printf("End of tokens.\n");
}

void sendToServer(int socketNum)
{
	uint8_t sendBuf[MAXBUF];   //data buffer
	int sendLen = 0;        //amount of data to send
	int sent = 0;            //actual amount of data sent/* get the data and send it   */
	
	
	sendLen = readFromStdin(sendBuf);

	//testing the parsing
	char** chunkArray = parseLine(sendBuf);
	printChunks(chunkArray);

	if (chunkArray[0] == NULL) {
    	printf("Invalid command.\n");
    	return;
	}

	else if(strcasecmp(chunkArray[0], "%M") == 0){

		//%M, dest, message
		char* message = chunkArray[2];
		int messageLength = strlen(message);
		int chunksToSend = 0;

		if (messageLength > 199){
			chunksToSend = (messageLength / 199);
			if (messageLength % 199 != 0){
				chunksToSend += 1;
			}
		}
		else {
			chunksToSend = 1;
		}

		//have the pointer start pointing at the start of the message
		

		for (int i = 0; i < chunksToSend; i++){

			char newChunk[200];

			//move the pointer forward by 200 every iteration
			int twoHunnidOffset = i * 199;
			int howMuchToCopy = 0;
			//calculate how much to copy that time
			if(messageLength - twoHunnidOffset >= 199){
				howMuchToCopy = 199;
			}
			else {
				howMuchToCopy = messageLength - twoHunnidOffset;
			}
			
			//store that in an new chunk
			//make that what is in the chunk array
			memcpy(newChunk, message + twoHunnidOffset, howMuchToCopy);
			//go to the end and null terminate it
			newChunk[howMuchToCopy] = '\0';
			chunkArray[2] = newChunk;

			uint8_t MBuf[MAX_M_PDU_LEN];
			int MLen = makeMPDU(chunkArray, MBuf);

			int Msent = sendPDU(socketNum, MBuf, MLen);

			if (Msent < 0)
			{
				perror("sendM call");
				exit(-1);
			}

		printf("Sent %d bytes for %%M message (chunk %d).\n", Msent, i + 1);

		}

	}

	else if(strcasecmp(chunkArray[0], "%C") == 0){

		int numDestHandles = atoi(chunkArray[1]);
    char* message = chunkArray[numDestHandles + 2];
    int messageLength = strlen(message);
    int chunksToSend = 0;

    if (messageLength > 199) {
        chunksToSend = (messageLength / 199);
        if (messageLength % 199 != 0) {
            chunksToSend += 1;
        }
    } else {
        chunksToSend = 1;
    }

    for (int i = 0; i < chunksToSend; i++) {

        char newChunk[200];
        int twoHunnidOffset = i * 199;
        int howMuchToCopy = 0;

        if (messageLength - twoHunnidOffset >= 199) {
            howMuchToCopy = 199;
        } else {
            howMuchToCopy = messageLength - twoHunnidOffset;
        }

        memcpy(newChunk, message + twoHunnidOffset, howMuchToCopy);
        newChunk[howMuchToCopy] = '\0';

        // Update the message part in chunkArray
        chunkArray[numDestHandles + 2] = newChunk;

        uint8_t CBuf[MAXBUF];
        int CLen = makeCPDU(chunkArray, CBuf);

        int Csent = sendPDU(socketNum, CBuf, CLen);

        if (Csent < 0) {
            perror("sendC call");
            exit(-1);
        }

        printf("Sent %d bytes for %%C message (chunk %d).\n", Csent, i + 1);
    }
}

	else if(strcasecmp(chunkArray[0], "%L") == 0){
		//only one byte just the flag
		uint8_t LBuf[1];

		int LLen = makeIntroLPDU(chunkArray, LBuf);
		
		int LSent = sendPDU(socketNum, LBuf, LLen);

		if (LSent < 0)
		{
			perror("sendL call");
			exit(-1);
		}

		printf("Sent %d bytes for %%C message.\n", LSent);

	}

	else if(strcasecmp(chunkArray[0], "%B") == 0){
		
		char* message = chunkArray[1];
    int messageLength = strlen(message);
    int chunksToSend = 0;

    if (messageLength > 199) {
        chunksToSend = (messageLength / 199);
        if (messageLength % 199 != 0) {
            chunksToSend += 1;
        }
    } else {
        chunksToSend = 1;
    }

    for (int i = 0; i < chunksToSend; i++) {

        char newChunk[200];
        int twoHunnidOffset = i * 199;
        int howMuchToCopy = 0;

        if (messageLength - twoHunnidOffset >= 199) {
            howMuchToCopy = 199;
        } else {
            howMuchToCopy = messageLength - twoHunnidOffset;
        }

        memcpy(newChunk, message + twoHunnidOffset, howMuchToCopy);
        newChunk[howMuchToCopy] = '\0';

        // Update the message part in chunkArray
        chunkArray[1] = newChunk;

        uint8_t BBuf[MAX_TEXT + 1];
        int BLen = makeBRequestPDU(chunkArray, BBuf);

        int BSent = sendPDU(socketNum, BBuf, BLen);

        if (BSent < 0) {
            perror("sendB call");
            exit(-1);
        }

        printf("Sent %d bytes for %%B message (chunk %d).\n", BSent, i + 1);
    	}
	}

	else{
		printf("read: %s string len: %d (including null)\n", sendBuf, sendLen);
	
		sent = sendPDU(socketNum, sendBuf, sendLen);

		if (sent < 0)
		{
			perror("send call");
			exit(-1);
		}

		printf("Amount of data sent is: %d\n", sent);
	}
}	


void receiveHandleConf(int socketNum){
	// just the one byte flag 
	uint8_t responseBuf[3];

	int recvLen = recvPDU(socketNum, responseBuf, 3);

    if (recvLen <= 0) {
        perror("confirmation flag receive call failed\n");
        exit(-1);
    }

    uint8_t responseFlag = responseBuf[0];  // get the response flag

    if (responseFlag == 2) {
        printf("Handle accepted.\n");
    } 
    else if (responseFlag == 3) {
        printf("Handle already in use: %s\n", clientHandle);
        close(socketNum);
        exit(1);
    } 
    else {
        printf("Unknown response. Terminating.\n");
        close(socketNum);
        exit(1);
    }
}

void sendHandle(int socketNum)
{
	uint8_t *sendHandleBuf = makeIntroPDU(); //data buffer
	int sendLen = 0;        //amount of data to send
	int sent = 0;            //actual amount of data sent/* get the data and send it

	sendLen = 2 + strlen(clientHandle); //total packet is 1 byte for flag, 1 for length of handle, then length of handle
	
	sent = sendPDU(socketNum, sendHandleBuf, sendLen);

	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	printf("Amount of data sent is: %d\n", sent);
	printf("PDU Contents: [Flag: %d] [Length: %d] [Handle: %s]\n", sendHandleBuf[0], sendHandleBuf[1], clientHandle);

	receiveHandleConf(socketNum);

}

int readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	int inputLen = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	//printf("Enter data: ");
	fflush(stdout);
	while (inputLen < (MAXBUF - 1) && aChar != '\n')
	{
		aChar = getchar();
		if (aChar != '\n')
		{
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0'; 
	inputLen++;
	
	// error checks for if the message given is larger than max amount
	//if that goes over, print error message, ignore all input until \n, ignore command, don't send anything
	if (inputLen - 1 > MAX_MESSAGE_LENGTH){
		printf("Error: Input length exceeds maximum input message length %d\n", MAX_MESSAGE_LENGTH);

		// go back through the entire thing and clear it
		if (aChar != '\n') {
        while ((aChar = getchar()) != '\n' && aChar != EOF) {
            // Discard extra characters
        	}
   		}
		return 0;
	}  

	return inputLen;
}

void checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 4)
	{
		printf("Wrong Inputs.\nPlease Provide: ./cclient <handle> <server_ip> <server_port>\n");
		exit(1);
	}

	// check if given handle is too long
    if (strlen(argv[1]) > HANDLE_MAX - 1) { 
        printf(" Invalid handle, handle longer than 100 characters: %s\n", argv[1]);
        exit(1);
    }

}

void clientControl(int serverSocket){

	// first want to add the server socket and the stdin to the poll set
	setupPollSet();
	addToPollSet(serverSocket);
	addToPollSet(STDIN_FILENO);

	while(1){

		if (printPromptFlag == 1) {
			printf("$: ");
			fflush(stdout);
		}
	
		// the current socket is what the pollcall will return 
		int currentSocket = pollCall(-1);
		if (currentSocket == serverSocket){
			processMsgFromServer(currentSocket);
		}
		else if (currentSocket == STDIN_FILENO) {
			processStdin(serverSocket);
		}
	}
}


void processMsgFromServer(int serverSocket)
{

	uint8_t buffer[MAXBUF];
	int serverStatus = recvPDU(serverSocket, buffer, MAXBUF);

	if (serverStatus == 0)
    {
        printf("Server terminated.\n");
        close(serverSocket);
        exit(0);
    }
	else if (serverStatus < 0) {
		perror("recv call");
	}

	else {
		uint8_t flag = 0;
    	flag = buffer[0];  // First byte is the flag

        if (flag == DNE_FLAG) { 
            // the client asked for DNE
            uint8_t handleLength = buffer[1];      

printf("Handle Length Parsed: %d\n", handleLength); 

            char handleName[HANDLE_MAX];    

			// go past the flag and the length (+2)                 
            memcpy(handleName, buffer + 2, handleLength);    
            handleName[handleLength] = '\0';   //null terminate that hoe              

            printf("Client with handle %s does not exist.\n", handleName);
        }

		else if (flag == LIST_ACK){

			printPromptFlag = 0;

			 // print out the start of the list
            uint32_t handleLength;
			//take the next 4 bytes out as one number past the flag
			memcpy(&handleLength, buffer + 1, 4);
			uint32_t hanldeLengthHOST = ntohl(handleLength);

			printPromptFlag = 0;

			printf("Number of clients: %d\n", hanldeLengthHOST);
		}

		else if (flag == LIST_HANDLE){

			printPromptFlag = 0;

		//[1 byte flag][1 byte length][handle]

			uint8_t destLen = buffer[1];

			// no +1 for the null terminator
			char destHandle[destLen + 1];

            memcpy(destHandle, buffer + 2, destLen);
			destHandle[destLen] = '\0';

            printf("\t%s\n", destHandle);
			
		}

		else if (flag == LIST_DONE){
			printPromptFlag = 1;
			return;
		}

		else if (flag == B_FLAG){

			//[]flag][sending length][sender handle][message]

			//take out the sending handle length after the flag
			int curBufSpot = 1;

   			uint8_t handleLen = buffer[curBufSpot];
    		char currHandle[handleLen + 1];  // +1 for null terminator

			curBufSpot++;

			//take out the sending handle
    		memcpy(currHandle, buffer + curBufSpot, handleLen);
    		currHandle[handleLen] = '\0'; 
    		curBufSpot += handleLen;

			//get the rest of the message
    		char *messageToPass = (char *)(buffer + curBufSpot); 

    		//for testing
    		printf("PARSED Sender Handle: %s\n", currHandle);
    		//printf("PARSED Destination Handle: %s\n", destHandle);
    		printf("PARSED Message: %s\n", messageToPass);

    		printf("\n%s: %s\n", currHandle, messageToPass);
    		fflush(stdout);

		}

		else if (flag == M_FLAG || flag == C_FLAG){

			//CAN REFACTOR THIS	
			
			int curBufSpot = 1;

   			uint8_t handleLen = buffer[curBufSpot];
    		char currHandle[handleLen + 1];  // +1 for null terminator

			curBufSpot++;

    		memcpy(currHandle, buffer + curBufSpot, handleLen);
    		currHandle[handleLen] = '\0'; 
    		curBufSpot += handleLen;

			uint8_t numDest = buffer[curBufSpot];

			printf("NUMBER OF DESTINATIONS:: %d\n", numDest);

    		curBufSpot++;

			// get who sent it
            for (int i = 0; i < numDest; i++) {

				// get out the destination
    			uint8_t destLen = buffer[curBufSpot];
				curBufSpot++;
				char destHandle[destLen + 1];

                memcpy(destHandle, buffer + curBufSpot, destLen);
    			destHandle[destLen] = '\0'; 
    			curBufSpot += destLen;

                printf("Message sent to: %s\n", destHandle);
            }

    		//get the rest of the message
    		char *messageToPass = (char *)(buffer + curBufSpot); 

    		//for testing
    		printf("PARSED Sender Handle: %s\n", currHandle);
    		//printf("PARSED Destination Handle: %s\n", destHandle);
    		printf("PARSED Message: %s\n", messageToPass);

    		printf("\n%s: %s\n", currHandle, messageToPass);
    		fflush(stdout);
		}
	}
}


void processStdin(int serverSocket)
{
	sendToServer(serverSocket);
	return;
}




