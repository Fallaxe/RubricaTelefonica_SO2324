/*
    il server una volta avviato si mette in ascolto di connessioni;
    per ogni connessione fa un fork per gestire le richieste

    se si logga un admin si dovra gesire la possibilita' di aggiungere contatti
    sennò si gestisce un visitatore normale;

*/
#include <stdio.h>
#include <stdlib.h>
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
#include "../vendor/cjson/cJSON_Utils.h"

#define SERVERPORT 12345
#define SERVERADDRESS "127.0.0.1"

#define BUFFER_MAX 1024
#define MAX_CLIENT 5
#define FILE_USERS "password.txt"
#define CONVERTION_SHA256_MAX (EVP_MAX_MD_SIZE *2 + 1) //usato per la coonversione unsigned char to char nei txt

typedef struct MSG {
    int isAdmin;
    char message[BUFFER_MAX];
} MSG;

typedef void (*operationOnList)(cJSON *found, cJSON* list, int connectSocket, MSG buffer);

typedef struct credenziali
{
    char user[25];
    char password[25];
} t_credenziali;

t_credenziali cred;
char *divisore =   "---------------------------------------------\n";
char *menuHeader = "|              Rubrica Telefonica           |\n";
char *scelte = "\t\tv - visita\n\t\ts - ricerca\n";
char *scelteLogin = "\t\tl - login admin\n";
char *scelteadmin= "\t\ta - aggiungi contatto\n\t\tm - modifica\n\t\tr - rimuovi contatto\n";
char *sceltaUscita = "\t\tx - esci\n\nCosa vuoi fare?\n";
int serverSocket;

sem_t **semPtr;
int criticalSection;

int ppidServerInit=1; //dichiarata solo per identificare il padre
void sendMenu(); //manda il menu' al client
int choiseHandler(int connectSocket, MSG choise,sem_t *sem); //gestisce la richiesta restituendo l'intero corrispondete
MSG readContent(int connectSocket, MSG buffer); //manda i contatti presenti sul server
MSG  search(int connectSocket, MSG buffer,operationOnList op);
//cJSON* searchAndReturn(int connectSocket, MSG buffer);
MSG login(int connectSocket, MSG buffer);
int verifica(t_credenziali cred);

/*solo admin*/
void addContact(); //aggiunge un nuovo contatto (aggiungo qui la richiesta di admin-mode?)
int aggiungiPersona(int connectSocket, MSG buffer);

void customSigHandler();
int createSettings();
void removeFromList(cJSON *found, cJSON* list,int connectSocket, MSG buffer);
void editFromList(cJSON *found, cJSON* list,int connectSocket, MSG buffer);
MSG  printContent(cJSON * array, int connectSocket,MSG buffer);
static cJSON * loadDatabase();
void saveDatabase(cJSON * list);
cJSON *creaPersona(int connectSocket, MSG buffer);