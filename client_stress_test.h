#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>

#include "communicate.h"
#include "pollLib.h"
#include "networks.h"

#define MAX_HANDLE_LEN 100
#define MAX_CLIENTS 10000 // Number of clients to simulate
#define SERVER_PORT "12345" // Replace with your server's port
#define SERVER_IP "localhost" // Replace with your server's IP

#define MAXBUF 200
#define DEBUG_FLAG 1

// Function prototypes
void register_handle(int socketNum, const char *handle);
void send_initial_packet(int socketNum, const char *handle);
int wait_for_server_response(int socketNum);
