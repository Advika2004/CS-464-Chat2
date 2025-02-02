// macros
#define MAX_MESSAGE_LENGTH 1400
#define MAXBUF 1024
#define DEBUG_FLAG 1
#define MAX_CHUNKS 12
#define HANDLE_MAX 100


struct MPDU{
    char senderHandle[HANDLE_MAX];
    char destHandle[HANDLE_MAX];
    char message[MAX_MESSAGE_LENGTH];
} MPDU;