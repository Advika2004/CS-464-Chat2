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


// structs
typedef struct PercentMMessage {
    uint8_t handle[101];
    uint8_t message[200];
}PercentMMessage;

// macros
#define MAX_MESSAGE_LENGTH 1400
#define MAXBUF 1024
#define DEBUG_FLAG 1
#define MAX_CHUNKS 12
#define HANDLE_MAX 100
#define M_FLAG 5
#define L_FLAG 10
#define B_FLAG 4
#define C_FLAG 6
#define DNE_FLAG 7

// global 
char clientHandle[HANDLE_MAX];