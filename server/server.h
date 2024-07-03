/*
    7109803 Miranda Bellezza
    7112588 Daniele Fallaci
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <ctype.h>

#include "server_utils.h"

//fonte: https://github.com/DaveGamble/cJSON
#include "../vendor/cjson/cJSON.h"
// #include "../vendor/cjson/cJSON_Utils.h" // il file è compreso con la libreria ma non è stato utilizzato nel seguente progetto.

#define SERVERPORT 12345
#define SERVERADDRESS "127.0.0.1"

#define BUFFER_MAX 1024
#define MAX_CLIENT 5
#define RECORDS_INPAGE 4
#define RECORDS_MAX 100

#define FILE_USERS "settings.txt"
#define FILE_DB "data.json"
#define CONVERTION_SHA256_MAX (EVP_MAX_MD_SIZE *2 + 1) //usato per la conversione unsigned char to char nei txt

/*
* I tipi utilizzati sono 
* MSG per l'invio dei messaggi e la identificazione degli utenti loggati,
* operationOnLit per la definizione di operazioni da eseguire sulla ricerca,
* credenziali usato per immagazzinare le credenziali di accesso al server.
*/
typedef struct MSG {
    int isAdmin;
    char message[BUFFER_MAX];
} MSG;

typedef int (*operationOnList)(cJSON *found, cJSON* list, int connectSocket, char *clientIP, MSG buffer);

typedef struct credenziali
{
    char user[25];
    char password[25];
} t_credenziali;

char *divisore = "---------------------------------------------\n";
char *menuHeader = "|              Rubrica Telefonica           |\n";
char *contattoHeader = "|                  Contatto                 |\n";
char *cercaHeader = "|               Cerca contatti              |\n";
char *scelte = "\t\tv - visita\n\t\ts - ricerca\n";
char *scelteLogin = "\t\tl - login admin\n";
char *scelteadmin= "\t\ta - aggiungi contatto\n\t\tm - modifica\n\t\tr - rimuovi contatto\n";
char *sceltaUscita = "\t\tx - esci\n\nCosa vuoi fare?\n";
int serverSocket;
char * resetArg = "-r"; // FLAG di utilizzo del reset user e password dell'admin

sem_t **semPtr; // utilizzo di un puntatore per la gestione delle interruzioni sulle mutue esclusioni
int inCriticalSection; // pid processo in sezione critica
int ppidServerInit=1; //dichiarata solo per identificare il padre
t_credenziali cred;