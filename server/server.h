/*
    il server una volta avviato si mette in ascolto di connessioni;
    per ogni connessione fa un fork per gestire le richieste

    se si logga un admin si dovra gesire la possibilita' di aggiungere contatti
    sennò si gestisce un visitatore normale;

*/

char *benvenuto = "Benvenuto client";
char *scelte = "1)visita \n2)modifica\n3)esci";

void printMenu(); //manda il menu' al client
int choiseHandler(); //gestisce la richiesta restituendo l'intero corrispondete
void printContent(); //manda i contatti presenti sul server

/*solo admin*/
void addContact(); //aggiunge un nuovo contatto (aggiungo qui la richiesta di admin-mode?)