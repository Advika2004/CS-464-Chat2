#include "makePDU.h"
#include "cclient.h"


//will take in what is given to the command line arguments
uint8_t* makeIntroPDU(){

    uint8_t static buffer[INTRO_PDU_LEN];
    uint8_t flag = 1;
    memcpy(buffer, &flag, 1);
    uint8_t lengthOfHandle = strlen(clientHandle);
    memcpy(buffer + 1, &lengthOfHandle, 1);
    memcpy(buffer + 2, clientHandle, lengthOfHandle);

    return buffer;
}


int makeMPDU(char **chunks, uint8_t *buffer){

    //uint8_t static buffer[MAX_M_PDU_LEN];
    uint16_t whereWeAtInBuf = 0;

    //1 byte for type of message
    uint8_t flag = M_FLAG;
    memcpy(buffer + whereWeAtInBuf, &flag, 1);
    whereWeAtInBuf += 1;

    // 1 byte for the sender handle length
    uint8_t senderHandleLen = strlen(clientHandle);
    memcpy(buffer + whereWeAtInBuf, &senderHandleLen, 1);
    whereWeAtInBuf += 1;

    //100 bytes for the handle
    memcpy(buffer + whereWeAtInBuf, clientHandle, senderHandleLen);
    whereWeAtInBuf += senderHandleLen;

    //1 byte destination handle
    uint8_t numDestHandles = 1;
    memcpy(buffer + whereWeAtInBuf, &numDestHandles, 1);
    whereWeAtInBuf += 1;

    //1 byte for the destination handle length
    uint8_t destHandleLen = strlen(chunks[1]); 
    memcpy(buffer + whereWeAtInBuf, &destHandleLen, 1);
    whereWeAtInBuf += 1;

    //100 bytes for the destination handle
    memcpy(buffer + whereWeAtInBuf, chunks[1], destHandleLen);
    whereWeAtInBuf += destHandleLen;

    // the actual message
    char *message = chunks[2];
    uint16_t messageLen = strlen(message) + 1;
    memcpy(buffer + whereWeAtInBuf, message, messageLen);
    whereWeAtInBuf += messageLen;

    printPDU(buffer, whereWeAtInBuf);
    printf("Total PDU Length: %d bytes\n", whereWeAtInBuf);


    return whereWeAtInBuf;
}

void printPDU(uint8_t *pdu, int length) {
    printf("PDU Contents (Hex): ");
    for (int i = 0; i < length; i++) {
        printf("%02X ", pdu[i]);  // Print each byte in hexadecimal
    }
    printf("\n");

    printf("PDU Contents (ASCII): ");
    for (int i = 0; i < length; i++) {
        if (pdu[i] >= 32 && pdu[i] <= 126) {
            printf("%c", pdu[i]);  // Printable characters
        } else {
            printf(".");          // Non-printable characters as dots
        }
    }
    printf("\n");
}


// uint8_t* makeCPDU(chunks){

// }

// uint8_t* makeBPDU(chunks){

// }

// uint8_t* makeLPDU(chunks){

// }