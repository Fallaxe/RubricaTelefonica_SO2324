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
#include <cjson/cJSON.h> //installabile via package manager
#include <cjson/cJSON_Utils.h>

#define SERVERPORT 12345
#define SERVERADDRESS "127.0.0.1"

#define BUFFER_MAX 1024
#define MAX_CLIENT 5
#define FILE_USERS "password.txt"

typedef struct MSG {
    int isAdmin;
    char message[BUFFER_MAX];
} MSG;

typedef struct credenziali
{
    char user[25];
    char password[25];
} t_credenziali;

t_credenziali cred;
char *benvenuto = "Benvenuto client\n";
char *scelte = "v - visita \nl - login admin\n";
char *scelteadmin= "m - modifica\n";
char *sceltaUscita = "x - esci\nCosa vuoi fare?\t";
char *scelteAdmin = "m - modifica";
char *visita = "inserisci il nome del contatto ricercato: ";


void sendMenu(); //manda il menu' al client
int choiseHandler(); //gestisce la richiesta restituendo l'intero corrispondete
void printContent(); //manda i contatti presenti sul server
void login();
int verifica(t_credenziali cred);

/*solo admin*/
void addContact(); //aggiunge un nuovo contatto (aggiungo qui la richiesta di admin-mode?)