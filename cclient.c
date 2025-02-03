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
	chunks[i] = firstChunk;
	i++;

	if (strcmp(firstChunk, "%M") == 0) { 
       char *handle = strtok(NULL, " "); 
	   chunks[i] = handle;  
	   i++;
       char *message = strtok(NULL, "\n"); 
	   chunks[i] = message;
	   i++;
    }

	if (strcmp(firstChunk, "%C") == 0) { 
       char *number = strtok(NULL, " ");
	   chunks[i] = number;
	   i++;

	   int numHandles = atoi(number);

	   if (numHandles < 2 || numHandles > 9){
		printf("Too many or too little clients specified, please re-enter between 2-9 other clients\n");
		return NULL;
	   }

	   int j = 0;
	   for (j = 0; j < numHandles; j++){
		char* curHandle = strtok(NULL, " ");
		chunks[i] = curHandle;
		i++;
	   }

       char *message = strtok(NULL, "\n"); 
	   chunks[i] = message;
	   i++;
    }

	if (strcmp(firstChunk, "%B") == 0) { 
       char *message = strtok(NULL, "\n"); 
	   chunks[i] = message;  
	   i++;
    }

	if (strcmp(firstChunk, "%L") == 0) { 
		//first chunk already in the chunks array so do nothing.
	   return chunks;
    }

	chunks[i] = NULL;
	printChunks(chunks);
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

	else if(strcmp(chunkArray[0], "%M") == 0){

		uint8_t MBuf[MAX_M_PDU_LEN];
		int MLen = makeMPDU(chunkArray, MBuf);

		int Msent = sendPDU(socketNum, MBuf, MLen);

		if (Msent < 0)
		{
			perror("sendM call");
			exit(-1);
		}

		printf("Sent %d bytes for %%M message.\n", Msent);
	}

	else if(strcmp(chunkArray[0], "%C") == 0){

		uint8_t CBuf[MAXBUF];
		int CLen = makeCPDU(chunkArray, CBuf);

		int Csent = sendPDU(socketNum, CBuf, CLen);

		if (Csent < 0)
		{
			perror("sendC call");
			exit(-1);
		}

		printf("Sent %d bytes for %%C message.\n", Csent);
	}

	else if(strcmp(chunkArray[0], "%L") == 0){
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
		printf("usage: ./cclient (handle) (server_ip) (server_port)\n");
		exit(1);
	}
}

void clientControl(int serverSocket){

	// first want to add the server socket and the stdin to the poll set
	setupPollSet();
	addToPollSet(serverSocket);
	addToPollSet(STDIN_FILENO);

	int printPromptFlag = 1;

	while(1){

		if (printPromptFlag == 1) {
			printf("$: ");
			fflush(stdout);
			printPromptFlag = 0;
		}
	
		// the current socket is what the pollcall will return 
		int currentSocket = pollCall(-1);
		if (currentSocket == serverSocket){
			processMsgFromServer(currentSocket);
			printPromptFlag = 1;
		}
		else if (currentSocket == STDIN_FILENO) {
			processStdin(serverSocket);
			printPromptFlag = 1;
		}
	}
}


void processMsgFromServer(int serverSocket)
{

	uint8_t buffer[MAXBUF];
	int serverStatus = recvPDU(serverSocket, buffer, MAXBUF);

	if (serverStatus == 0)
    {
        printf("Server has terminated.\n");
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
            char handleName[HANDLE_MAX];    

			// go past the flag and the length (+2)                 
            memcpy(handleName, buffer + 2, handleLength);    
            handleName[handleLength] = '\0';   //null terminate that hoe              

            printf("Client with handle %s does not exist.\n", handleName);
        }

		else if (flag == M_FLAG || flag == C_FLAG){

			printf("Buffer Contents (Hex): ");
    for (int i = 0; i < serverStatus; i++) {
        printf("%02X ", buffer[i]);
    }
    printf("\n");

    // ✅ Directly print the buffer contents (ASCII)
    printf("Buffer Contents (ASCII): ");
    for (int i = 0; i < serverStatus; i++) {
        if (buffer[i] >= 32 && buffer[i] <= 126) {
            printf("%c", buffer[i]);  // Printable characters
        } else {
            printf(".");  // Non-printable characters as '.'
        }
    }
    printf("\n");

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