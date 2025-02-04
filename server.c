/******************************************************************************
* myServer.c
* 
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/
#include "networks.h"
#include "safeUtil.h"
#include "communicate.h"
#include "pollLib.h"
#include "server.h"
#include "dict.h"
#include "makePDU.h"

int main(int argc, char *argv[])
{
	int mainServerSocket = 0;   //socket descriptor for the server socket
	int clientSocket = 0;   //socket descriptor for the client socket
	int portNumber = 0;
	
	portNumber = checkArgs(argc, argv);

	//make the table
	table = dctCreate();
	//printf("DEBUG - TABLE SIZE WHEN CREATED: %d\n", table->size);
	
	//create the server socket
	mainServerSocket = tcpServerSetup(portNumber);
	serverControl(mainServerSocket);

	while(1) {
		// wait for client to connect
		clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);

		recvFromClient(clientSocket);
	
		/* close the sockets */
		close(clientSocket);
	}

	dctDestroy(table);
	close(mainServerSocket);

	return 0;
}

//will check if the handle is in the table
//if it is, then it will return the invalid flag
//if it isnt, it will add it into the table, and return the valid flag
uint8_t validateHandle(Dict *table, char* handle, int clientSocket){

	printf("LOOKING FOR KEY: %s\n", handle);
	int uniqueOrNah = searchByKey(table, handle);
	printf("IS IT IN THE TABLE?: %d\n", uniqueOrNah);

	if(uniqueOrNah == -1){
		//key is not found, so it is unique, send back the flag
		dctInsert(table, handle, clientSocket);

		//checking if I can get values out of my table
		int num = searchByKey(table, handle);
		char* han = searchByValue(table, clientSocket);
		printf("HERE: the socketnum just added: %d\n", num);
		printf("HERE: the handle just added: %s\n", han);


		printf("DEBUG - TABLE SIZE AFTER INSERTED: %d\n", table->size);
		return VALID_FLAG;
	}
	return INVALID_FLAG;
}

void forwardBPDU(int socketNum, uint8_t* OGBuffer, int OGbufferLen){

	//get all of the keys in the table rn and forward the OG buffer to all of them
	char** allKeys = dctkeys(table);

	for (int i = 0; i < table->size; i++){

		// find who is sending it and skip them
        int clientSocket = searchByKey(table, allKeys[i]);

        // Skip sending the message back to the sender
        if (socketNum == clientSocket) {
            continue;
        }

		int sent = sendPDU(clientSocket, OGBuffer, OGbufferLen);

    	if (sent < 0){
            perror("forwarding %B message failed\n");
            exit(-1);
        }

		printf("Broadcast sent to '%s' (Socket: %d), Bytes sent: %d\n", allKeys[i], clientSocket, sent);
	}
}


void forwardLPDU(int socketNum){

	//how many handles there currently are
	//put that into network order 32 bit
	uint32_t numTotalHandles = table->size;
	printf("HOW MANY CLIENTS IN TABLE WHEN FORWARDING??: %d\n", numTotalHandles);
	uint32_t numTotalHandlesNET = htonl(numTotalHandles);

	//make a buffer big enough to send that
	uint8_t totalClients[5]; //flag + 4 byte number

	//make the response buffer [flag 11][number of handles total]
	int sendLen = makeServerLPDU(numTotalHandlesNET, totalClients);
	int sent = sendPDU(socketNum, totalClients, sendLen);

    if (sent < 0){
            perror("forwarding %L message failed\n");
            exit(-1);
        }

	printf("Message successfully sent! Bytes sent: %d\n", sent);

	//now have to do the second send...right? and make new packets?? 

	char** allKeys = dctkeys(table);

	for (int i = 0; i < table->size; i++){

		//make the buffer big enough for 1 byte flag, 1 byte length, then handle
		uint8_t handleNameBuf[HANDLE_MAX + 2];

		int sendLists = makeListNamesPDU(allKeys[i], handleNameBuf);

		int sent = sendPDU(socketNum, handleNameBuf, sendLists);

    	if (sent < 0){
            perror("forwarding %L message failed\n");
            exit(-1);
        }

		printf("Handle '%s' sent successfully! Bytes sent: %d\n", allKeys[i], sent);
	}
 
	//send the last flag because the list was done sending
    uint8_t doneFlag = LIST_DONE;
    sent = sendPDU(socketNum, &doneFlag, 1); //just send that one flag over no need to make any pdu

    if (sent < 0) {
        perror("list done flag not send\n");
        exit(-1);
    }

    printf("Flag 13 (completion) sent successfully! Bytes sent: %d\n", sent);

	 return;
}



void forwardCPDU(char* curHandle, char destHandles[][HANDLE_MAX], int numDest, char* message, uint8_t* OGBuffer, int OGbufferLen) {
    printf("FORWARDING MULTICAST MESSAGE...\n");
    printf("Current Handle: %s\n", curHandle);
    printf("Message: %s\n", message);
    printf("Number of Recipients: %d\n", numDest);

    for (int i = 0; i < numDest; i++) {

        printf("Sending to: %s\n", destHandles[i]);

        // Call forwardMPDU for each recipient
        forwardMPDU(curHandle, destHandles[i], message, OGBuffer, OGbufferLen);
    }
}

void forwardMPDU(char* curHandle, char* destHandle, char* message, uint8_t* OGBuffer, int OGbufferLen){

	//JUST DEBUGGING PRINTING
	printf("FORWARDING MESSAGE...\n");
    printf("Current Handle: %s\n", curHandle);
    printf("Destination Handle: %s\n", destHandle);
    printf("Message: %s\n", message);

    for (int i = 0; i < table->cap; i++) {
        Node *current = table->arr[i];
        while (current != NULL) {
            printf("TABLE ENTRY: Handle: %s, Socket: %d\n", current->key, current->value);
            current = current->next;
        }
    }

	//int messageLength = strlen(message) + 1; //strlen does not include the null terminator
	int destSocketNum = searchByKey(table, destHandle);
	int sent = 0;
	printf("Destination Socket Number (after searchByKey): %d\n", destSocketNum);

	//it is within the table
    if (destSocketNum != -1){
    	printf("Destination found. Sending message to socket: %d\n", destSocketNum);

		int sent = sendPDU(destSocketNum, (uint8_t *)OGBuffer, OGbufferLen);

        if (sent < 0){
              perror("forwarding %M message failed\n");
              exit(-1);
          }

		printf("Message successfully sent! Bytes sent: %d\n", sent);
        return;
	}

	else{
        //want to send back the error flag and the error message to the client
        printf("Destination handle '%s' not found in table. Sending error response back to sender.\n", destHandle);

		int flag = DNE_FLAG;
        int destHandleLen = strlen(destHandle);
        uint8_t errorBuffer[2 + destHandleLen];
    
        //will give me who to send it back to
        int clientSocket = searchByKey(table, curHandle);
        printf("SOcket to send it back to: %d\n", clientSocket);
    
        if (clientSocket == -1) {
                printf("Error: Could not find socket for sender handle '%s'.\n", curHandle);
                return; 
        }
    
        memcpy(errorBuffer, &flag, 1); // 1 byte that holds the flag
        memcpy(errorBuffer + 1, &destHandleLen, 1); //1 byte for how long the handle is
        memcpy(errorBuffer + 2, destHandle, destHandleLen); //then the handle searched for 
    
        printf("FLAG BEING SENT BACK BECAUSE DESTINATION DOES NOT EXIST: %d\n", flag);
            printf("Error Buffer Contents (Hex): ");
            for (int i = 0; i < destHandleLen + 2; i++) {
                printf("%02X ", errorBuffer[i]);
            }
            printf("\n");
    
        sent = sendPDU(clientSocket, errorBuffer, destHandleLen + 2);
    
        if (sent < 0){
              perror("send failed\n");
              exit(-1);
          }
    	printf("Error response successfully sent to sender. Bytes sent: %d\n", sent);
    }
}


void parsePDU(int clientSocket, uint8_t *buffer, int messageLen){
	//read the flag out
	//first byte of the payload will be the flag, then length of handle, then handle
	uint8_t messageTypeFlag = buffer[0];
	int sent = 0;

	//if its a intro message, then check if it exists in the table and send back the corresponding flag
	if(messageTypeFlag == INTRO_FLAG){

		uint8_t handleLen = buffer[1];     //get the handle length and add 1 for the null terminator
    	char handleName[handleLen + 1];

		memcpy(handleName, buffer + 2, handleLen); //just take out the handle
    	handleName[handleLen] = '\0';

		uint8_t flag;
		flag = validateHandle(table, handleName, clientSocket);

		printf("FLAG BEING SENT BACK: %d\n", flag);
		sent = sendPDU(clientSocket, &flag, 1);
		printf("Sent %d bytes (flag: %d) to client.\n", sent, flag);

		if (sent < 0){
        	perror("send failed\n");
        	exit(-1);
    	}
	}

	//[flag][length handle][handle][number destinations][length of target][actual target][message]
	if (messageTypeFlag == M_FLAG) {
    	printf("MESSAGE PACKET RECEIVED\n");

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

    	//pass message to the forwarder
    	forwardMPDU(currHandle, destHandle, messageToPass, buffer, messageLen);
	}

	if(messageTypeFlag == B_FLAG){

		printf("BROADCAST PACKET RECEIVED\n");

		forwardBPDU(clientSocket, buffer, messageLen);

		return;
	}

	if(messageTypeFlag == C_FLAG){
		printf("MULTICAST PACKET RECEIVED\n");

		int curBufSpot = 1;

   		uint8_t handleLen = buffer[curBufSpot];
    	char currHandle[handleLen + 1];  // +1 for null terminator

		curBufSpot++;

    	memcpy(currHandle, buffer + curBufSpot, handleLen);
    	currHandle[handleLen] = '\0'; 
    	curBufSpot += handleLen;

    	// should be the number of handles given 
    	uint8_t numDest = buffer[curBufSpot];
		curBufSpot++;

		//NEW STUFF FOR %C!!
		//hold the handle names
		char destHandles[numDest][HANDLE_MAX];
    	int destSockets[numDest];

		//go through the rest of the buffer, get out each handle, and store it into the array.
		for (int i = 0; i < numDest; i++) {
			uint8_t destLen = buffer[curBufSpot];
			curBufSpot++;

			//get the handle out and add that to the array
    		memcpy(destHandles[i], buffer + curBufSpot, destLen);
    		destHandles[i][destLen] = '\0'; 
    		curBufSpot += destLen;

			//look through the table and add that to here as well
			int socketNumber = searchByKey(table, destHandles[i]);
			destSockets[i] = socketNumber;

			//DEBUG PRINT
			printf("Destination %d: %s (Socket: %d)\n", i + 1, destHandles[i], destSockets[i]);
		}

    	//get the rest of the message
    	char *messageToPass = (char *)(buffer + curBufSpot); 

    	//for testing
    	printf("PARSED Message: %s\n", messageToPass);

    	//pass message to the forwarder
    	forwardCPDU(currHandle, destHandles, numDest, messageToPass, buffer, messageLen);

		return;
	}

	if(messageTypeFlag == L_FLAG){

		printf("LIST PACKET RECEIVED\n");

		forwardLPDU(clientSocket);

		return;
	}
}

void recvFromClient(int clientSocket)
{
	uint8_t dataBuffer[MAXBUF]; 
	int messageLen = 0;
	
	//now get the data from the client_socket
	messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF);
	printf("what recvPDu is returning: %d\n", messageLen);
	if (messageLen < 0) {
		perror("recv call");
	}

	if (messageLen > 0)
	{
		parsePDU(clientSocket, dataBuffer, messageLen);
		
		printf("Message received on socket %d, length: %d Data: %s\n", clientSocket, messageLen, dataBuffer);
		
		//! part b of part 8 - uncomment to test that processMsgFromServer works

		// //send the same PDU back to the client
		// int sentLen = sendPDU(clientSocket, dataBuffer, messageLen);
		// //error check that it was sent properly
		// if (sentLen < 0)
        // {
        //     perror("sendPDU failed");
        //     close(clientSocket);
        //     removeFromPollSet(clientSocket);
        // }
	}
	else if (messageLen == 0)
	{
		dctRemoveValue(table, clientSocket);
		printf("DEBUG - TABLE SIZE AFTER REMOVED: %d\n", table->size);
		close(clientSocket);
		removeFromPollSet(clientSocket);
		printf("Connection closed by other side\n");
	}
}

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}

void serverControl(int serverSocket){

	// first want to add the server socket in
	addToPollSet(serverSocket);

	while(1){
		// the current socket is what the pollcall will return
		int currentSocket = pollCall(-1);

		if (currentSocket == serverSocket){
			addNewSocket(currentSocket);
			continue;
		}
		else {
			processClient(currentSocket);
			continue;
		}
	}
	return;
}

void addNewSocket(int serverSocket){
	int newSocket = tcpAccept(serverSocket, DEBUG_FLAG);
	addToPollSet(newSocket);
	return;
}

void processClient(int socketNumber){
	recvFromClient(socketNumber);
	return;
}
