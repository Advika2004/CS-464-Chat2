#include <stdint.h>

#define MAX_M_PDU_LEN 1604
#define INTRO_PDU_LEN 102
#define LIST_REQUEST 10
#define LIST_ACK 11
#define LIST_HANDLE 12


uint8_t* makeIntroPDU();
int makeMPDU(char **chunks, uint8_t *buffer);
void printPDU(uint8_t *pdu, int length);
int makeCPDU(char **chunks, uint8_t *buffer);
int makeIntroLPDU(char **chunks, uint8_t *buffer);
int makeServerLPDU(uint32_t numHandles, uint8_t *buffer);
int makeListNamesPDU(char* handle, uint8_t *buffer);
int makeBRequestPDU(char **chunks, uint8_t *buffer);
