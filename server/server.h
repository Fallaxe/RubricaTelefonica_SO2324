/*
    il server una volta avviato si mette in ascolto di connessioni;
    per ogni connessione fa un fork per gestire le richieste

    se si logga un admin si dovra gesire la possibilita' di aggiungere contatti
    sennò si gestisce un visitatore normale;

*/

#define SERVERPORT 12345
#define SERVERADDRESS "127.0.0.1"

#define BUFFER_MAX 32
#define MAX_CLIENT 5

char *benvenuto = "Benvenuto client";
char *scelte = "v - visita \nl - login admin\nx - esci";
char *scelteAdmin = "m - modifica";
char *visita = "inserisci il nome del contatto ricercato: ";


void sendMenu(); //manda il menu' al client
int choiseHandler(char*); //gestisce la richiesta restituendo l'intero corrispondete
void printContent(); //manda i contatti presenti sul server

/*solo admin*/
void addContact(); //aggiunge un nuovo contatto (aggiungo qui la richiesta di admin-mode?)