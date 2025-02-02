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


// uint8_t* makeMPDU(chunks){

//     uint8_t buffer[MAX_M_PDU_LEN];

//     //1 byte for type of message
//     uint8_t flag = 5;
//     memcpy(buffer, flag, 1);

//     //1 byte length of handle
//     lengthOfHandle = chunks[1];
//     memcpy(buffer, lengthOfHandle, 1);



// }

// uint8_t* makeCPDU(chunks){

// }

// uint8_t* makeBPDU(chunks){

// }

// uint8_t* makeLPDU(chunks){

// }