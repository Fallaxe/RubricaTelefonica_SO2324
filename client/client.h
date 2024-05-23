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

#define SERVERPORT 12345
#define SERVERADDRESS "127.0.0.1"

#define BUFFER_MAX 1024

char *home = "h";
//char *credenziali[2];


typedef struct credenziali
{
    char user[25];
    char password[25];
} t_credenziali;

t_credenziali cred = {.user="", .password=""};


int login(char *user, char *password);
void parser(char *argomenti[], int max);