#include "communicate.h"




int sendPDU(int clientSocket, uint8_t * dataBuffer, int lengthOfData){

    // entire length for the buffer in host order
    uint16_t PDUHeaderNETWORK = htons(lengthOfData + 2);

    // make the buffer big enough
    uint8_t buffer[lengthOfData + 2];

    // memcpy the first two bytes
    memcpy(buffer, &PDUHeaderNETWORK, 2); 

    // memcpy the actual data
    memcpy(buffer + 2, dataBuffer, lengthOfData); 

    int result;

    result = send(clientSocket, buffer, lengthOfData + 2, 0);

    if (result < 0){
        perror("send failed\n");
        exit(-1);
    }

    return result;
}

int recvPDU(int socketNumber, uint8_t * dataBuffer, int bufferSize){

    int result1;

    result1 = recv(socketNumber, dataBuffer, 2, MSG_WAITALL);

    //printf("RESULT1 %d\n", result1);

    if (result1 <= 0){
        perror("Connection was closed.\n");
        return result1;
    }

    int result2;

    // need to get what is inside the dataBuffer? how do I get the value in there - 2?
    int PDULength = ntohs(*(uint16_t*)dataBuffer);
    int PDUDataLength = ntohs(*(uint16_t *)dataBuffer) - 2;

    if (PDULength > bufferSize){
        perror("Buffer not big enough\n");
        exit(-1);
    }

    result2 = recv(socketNumber, dataBuffer, PDUDataLength, MSG_WAITALL);

    if (result2 < 0){

        if (errno == ECONNRESET)
        {
            result2 = 0;
        }
        else
        {
            perror("recv call");
            result2 = -1;
        }
    }

    return result2;
}