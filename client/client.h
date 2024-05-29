/*
    i client possono essere admin oppure "visitatori" normali:
    al momento del collegamento si useranno gli argomenti:
    -n nome
    -p password
    esempio $ client -n admin -p passoword

    se la password è errata la connessione è rifiutata
    se si esegue il programma senza argomenti il tipo di connessione è da visitatore

    un visitatore può:
    -> visualizzare la lista dei contatti
    un admin può fare tutto ciò che fa un visitatore ma
    può anche modificare la rubrica (e aggiungere anche altri admin?)
    prova
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define SERVERPORT 12345
#define SERVERADDRESS "127.0.0.1"

#define BUFFER_MAX 1024

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
char *home = "h";

void login(int socket, MSG buffer);
int parser(char *argomenti[], int max);
void requestHome(int socket, MSG buffer);