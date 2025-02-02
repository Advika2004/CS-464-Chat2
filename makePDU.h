#include <stdint.h>

#define MAX_M_PDU_LEN 1604
#define INTRO_PDU_LEN 102


uint8_t* makeIntroPDU();
int makeMPDU(char **chunks, uint8_t *buffer);
void printPDU(uint8_t *pdu, int length);
