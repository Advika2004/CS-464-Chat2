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

#include "networks.h"
#include "safeUtil.h"
#include "communicate.h"
#include "pollLib.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1

// the flags
#define INTRO_FLAG 1
#define M_FLAG 5
#define L_FLAG 10
#define B_FLAG 4
#define C_FLAG 6
#define VALID_FLAG 2
#define INVALID_FLAG 3

void recvFromClient(int clientSocket);
int checkArgs(int argc, char *argv[]);
void serverControl(int socketNumber);
void processClient(int socketNumber);
void addNewSocket(int socketNumber);