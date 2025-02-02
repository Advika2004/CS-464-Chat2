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

void sendToServer(int socketNum)
{
	uint8_t sendBuf[MAXBUF];   //data buffer
	int sendLen = 0;        //amount of data to send
	int sent = 0;            //actual amount of data sent/* get the data and send it   */
	
	sendLen = readFromStdin(sendBuf);
	printf("read: %s string len: %d (including null)\n", sendBuf, sendLen);
	
	sent = sendPDU(socketNum, sendBuf, sendLen);

	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	printf("Amount of data sent is: %d\n", sent);
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
			printf("Enter data: ");
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
		printf("\nMessage received on socket %d, length: %d Data: %s\n", serverSocket, serverStatus, buffer);
	}
}


void processStdin(int serverSocket)
{
	sendToServer(serverSocket);
	return;
}