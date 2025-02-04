#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>

// functions
void sendToServer(int socketNum);
int readFromStdin(uint8_t * buffer);
void checkArgs(int argc, char * argv[]);
void clientControl(int serverSocket);
void processMsgFromServer(int serverSocket);
void processStdin(int serverSocket);
void printChunks(char** array);
char** parseLine(uint8_t *buffer);
void sendHandle(int socketNum);
void receiveHandleConf(int socketNum);
void printChunks(char **chunks);
char** parseLine(uint8_t *buffer);
void sendChunkedMessage(int socketNum, char **chunkArray, int messageIndex, int (*makePDUFunc)(char **, uint8_t *));

// structs
typedef struct PercentMMessage {
    uint8_t handle[101];
    uint8_t message[200];
}PercentMMessage;

// macros
#define MAX_MESSAGE_LENGTH 1400
//is this 199 or 200?
#define MAX_TEXT 199
#define MAXBUF 1024
#define DEBUG_FLAG 1
#define MAX_CHUNKS 12
#define HANDLE_MAX 100
#define M_FLAG 5
#define L_FLAG 10
#define B_FLAG 4
#define C_FLAG 6
#define DNE_FLAG 7
#define LIST_HANDLE 12
#define LIST_DONE 13

// global 
char clientHandle[HANDLE_MAX];
int printPromptFlag;
