/*
    7109803 Miranda Bellezza
    7112588 Daniele Fallaci
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#define SERVERPORT 12345
#define SERVERADDRESS "127.0.0.1"

#define BUFFER_MAX 1024
#define INPUT_MAX 35

// messaggio che client e server si scambiano
typedef struct MSG {
    int isAdmin;
    char message[BUFFER_MAX];
} MSG;

// credenziali per il login
typedef struct credenziali
{
    char user[25];
    char password[25];
} t_credenziali;

t_credenziali cred;
int clientSocket;
char *homeChar = "h";  // utilizzato per la richiesta di home al server
char *loginChar = "l"; // utilizzato per la richiesta di login al server
char *loginArg = "-a"; // FLAG di utilizzo per il login diretto