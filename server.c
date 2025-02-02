/******************************************************************************
* myServer.c
* 
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/
#include "server.h"
#include "dict.h"

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
		printf("DEBUG - TABLE SIZE AFTER INSERTED: %d\n", table->size);
		return VALID_FLAG;
	}
	return INVALID_FLAG;
}

		//TESTING:
		// char* handle = dctGetKey(table, clientSocket);
		// printf("handle name: %s\n", handle);

		// int socket = dctGetValue(table, handleName);
		// printf("socket number: %d\n", socket);

		// printf("DEBUG - TABLE SIZE AFTER INSERTED: %d\n", table->size);


void parsePDU(int clientSocket, uint8_t *buffer, int messageLen){
	//read the flag out
	//first byte of the payload will be the flag, then length of handle, then handle
	uint8_t messageTypeFlag = buffer[0];
	char* handleName = (char*)buffer;
	int sent = 0;
	
	//if its a intro message, then check if it exists in the table and send back the corresponding flag
	if(messageTypeFlag == INTRO_FLAG){

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

	if(messageTypeFlag == M_FLAG){
		return;
	}
	if(messageTypeFlag == B_FLAG){
		return;
	}
	if(messageTypeFlag == C_FLAG){
		return;
	}
	if(messageTypeFlag == L_FLAG){
		return;
	}
}

void recvFromClient(int clientSocket)
{
	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	
	//now get the data from the client_socket
	messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF);
	//printf("what recvPDu is returning: %d\n", messageLen);
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
