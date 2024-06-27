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
#include "../vendor/cjson/cJSON.h" //installabile via package manager
#include "../vendor/cjson/cJSON_Utils.h"

#define SERVERPORT 12345
#define SERVERADDRESS "127.0.0.1"

#define BUFFER_MAX 1024
#define MAX_CLIENT 5
#define FILE_USERS "password.txt"

typedef struct MSG {
    int isAdmin;
    char message[BUFFER_MAX];
} MSG;

typedef void (*operationOnList)(cJSON *found, cJSON* list, int connectSocket, MSG buffer);
// typedef struct person{
//     char name[25];
//     int age;
//     char email[25];
// }t_person;

typedef struct credenziali
{
    char user[25];
    char password[25];
} t_credenziali;

char *contactBase =
"{\
    \n\"name\": \n{\
        \n\"question\": \"Inserire il nome del contatto: \n\", \
        \n\"type\": \"string\", \
        \n\"dimension\": 12 \
    \n}, \
    \n\"surname\": \n{\
        \n\"question\": \"Inserire il cognome del contatto: \n\", \
        \n\"type\": \"string\", \
        \n\"dimension\": 12 \
    \n}, \
    \n\"age\": \n{\
        \n\"question\": \"Inserire l'età del contatto: \n\", \
        \n\"type\": \"int\", \
        \n\"max\": 99, \
        \n\"min\": 1\
    \n}, \
    \n\"email\": \n{\
        \n\"question\": \"Inserire l'email del contatto: \n\", \
        \n\"type\": \"string\", \
        \n\"dimension\": 30 \
    \n}, \
    \n\"phone\": \n{\
        \n\"question\": \"Inserire il numero di telefono del contatto: \n\", \
        \n\"type\": \"string\", \
        \n\"dimension\": 16, \
        \n\"useOnly\": \"+ 0123456789\" \
    \n} \
\n}";

t_credenziali cred;
char *benvenuto = "Benvenuto client\n";
char *scelte = "v - visita\ns - ricerca\nl - login admin\n";
char *scelteadmin= "a - aggiungi contatto\nm - modifica\nr - rimuovi contatto\n";
char *sceltaUscita = "x - esci\nCosa vuoi fare?\t";
char *visita = "lista di tutti i contatti: \n";

int ppidServerInit=1; //dichiarata solo per identificare il padre
void sendMenu(); //manda il menu' al client
int choiseHandler(int connectSocket, MSG choise,sem_t *sem); //gestisce la richiesta restituendo l'intero corrispondete
void readContent(int connectSocket, MSG buffer); //manda i contatti presenti sul server
MSG  search(int connectSocket, MSG buffer,operationOnList op);
//cJSON* searchAndReturn(int connectSocket, MSG buffer);
void login(int connectSocket, MSG buffer);
int verifica(t_credenziali cred);

/*solo admin*/
void addContact(); //aggiunge un nuovo contatto (aggiungo qui la richiesta di admin-mode?)
int aggiungiPersona(int connectSocket, MSG buffer);

void customSigHandler();
int createSettings();
void clean_stdin();
void removeFromList(cJSON *found, cJSON* list,int connectSocket, MSG buffer);
void editFromList(cJSON *found, cJSON* list,int connectSocket, MSG buffer);
MSG  printContent(cJSON * array, int connectSocket,MSG buffer);
static cJSON * loadDatabase();
void saveDatabase(cJSON * list);
cJSON* creaPersona(int connectSocket);