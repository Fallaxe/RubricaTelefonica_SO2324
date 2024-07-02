/*
    il server una volta avviato si mette in ascolto di connessioni;
    per ogni connessione fa un fork per gestire le richieste

    se si logga un admin si dovra gesire la possibilita' di aggiungere contatti
    sennò si gestisce un visitatore normale;

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
#include "../vendor/cjson/cJSON_Utils.h"

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

typedef int (*operationOnList)(cJSON *found, cJSON* list, int connectSocket, MSG buffer);

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

/*
* Metodi usati dal server
* invio di menù, gestore delle richieste, lettura del DB, ricerca e azione su essa.
* utilizzo di login e verifica per il login del server.
*/
// static void sendMenu(); //manda il menu' al client
// static int choiseHandler(int connectSocket, char*clientIP, MSG choise,sem_t *sem); //gestisce la richiesta restituendo l'intero corrispondete
// static MSG readContent(int connectSocket, char *clientIP, MSG buffer); //manda i contatti presenti sul server
// static MSG search(int connectSocket, char * clientIP, MSG buffer,operationOnList op); // funziona in maniera simile ad uno stream java
// static MSG login(int connectSocket, MSG buffer);
// static int verifica(t_credenziali cred);

// /*solo admin*/
// static void addContact(); // aggiunge un nuovo contatto
// static int aggiungiPersona(int connectSocket, char *clientIP, MSG buffer);
// static int removeFromList(cJSON *found, cJSON* list,int connectSocket, MSG buffer);
// static int editFromList(cJSON *found, cJSON* list,int connectSocket, MSG buffer);


// /*altri metodi generici utilizzati per la gestione di segnali e del server*/
// static void customSigHandler();
// static int createSettings(char const *argomenti[],int max);
// static int parser(char const *argomenti[], int max);
// static cJSON * loadDatabase();
// static void saveDatabase(cJSON * list);
// static cJSON *creaPersona(int connectSocket, MSG buffer);
// static MSG  printContent(cJSON * array, int connectSocket,char *clientIP,MSG buffer);