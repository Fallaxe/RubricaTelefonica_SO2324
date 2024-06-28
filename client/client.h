#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define SERVERPORT 12345
#define SERVERADDRESS "127.0.0.1"

#define BUFFER_MAX 1024

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
char *homeChar = "h";
char *loginChar = "l";
char *loginArg = "-a";

static void login(int socket, MSG buffer);
static int parser(char *argomenti[], int max);
static void requestHome(int socket, MSG buffer);

//gestione segnali
static void customSigHandler(); // interruzione: ctrl+C
static void sigpipe_handler(int signo); // server chiude la connessione