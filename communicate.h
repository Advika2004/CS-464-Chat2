#include <stdint.h>
#include <sys/socket.h>
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <arpa/inet.h>  
#include <errno.h>


//? function declarations
int sendPDU(int clientSocket, uint8_t * dataBuffer, int lengthOfData);
int recvPDU(int socketNumber, uint8_t * dataBuffer, int bufferSize);

