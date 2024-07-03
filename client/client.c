/*
    7109803 Miranda Bellezza
    7112588 Daniele Fallaci
*/

#include "client.h"

// Gestione dei segnali
static void customSigHandler() {
    printf("\nInterruzione ricevuta: chiusura socket.\n");
    close(clientSocket);
    exit(0);
}
static void sigpipe_handler(int signo) {
    printf("Connessione chiusa dal server.\n");
    close(clientSocket);
    exit(0);
}

// Pulizia stream input
static int check_stdin()
{
    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    // tempo di attesa, 0 nel nostro caso
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    return select(1, &rfds, NULL, NULL, &tv);
}

static void clean_stdin() {
    if(check_stdin()) {
        char buffer[BUFFER_MAX];

        while(check_stdin())
            // se ci sono righe piene le legge una alla volta
            fgets(buffer, BUFFER_MAX, stdin);
    }
}

// Gestione argomenti
static int parser(char *argomenti[]) {

    for (char **ptr = argomenti; ptr[2]; ptr++)
    {
        if(strcmp(ptr[0], loginArg) == 0){
            printf("Login richiesto con username: %s, password: %s", ptr[1], ptr[2]);
            strcpy(cred.user, ptr[1]);
            strcpy(cred.password, ptr[2]);
            return 1;
        }
    }
    return 0;
}

// Procedura login
static void login(int clientSocket, MSG buffer) {
    strcpy(buffer.message, loginChar);
    send(clientSocket, &buffer, sizeof(buffer), 0);

    strcpy(buffer.message, "");
    if(recv(clientSocket,&buffer, sizeof(buffer), 0) < 0)
    {
        perror("Errore nella ricezione dei dati.\n");
        exit(-1);
    }

    // invia il nome utente    
    strcpy(buffer.message, cred.user);
    send(clientSocket,&buffer, sizeof(buffer), 0);

    strcpy(buffer.message, "");
    if(recv(clientSocket,&buffer, sizeof(buffer), 0) < 0)
    {
        perror("Errore nella ricezione dei dati.\n");
        exit(-1);
    }

    // invia la password
    strcpy(buffer.message,cred.password);
    send(clientSocket,&buffer, sizeof(buffer), 0);
}

static void requestHome(int clientSocket, MSG buffer) {
    strcpy(buffer.message, homeChar);
    send(clientSocket, &buffer, sizeof(buffer), 0);
}

int main(int argc, char * argv[]) {
    struct sockaddr_in serverAddress;
    int returnCode;
  
    // buffer bidirezionale per messaggi
    MSG buffer;
    buffer.isAdmin = 0;

    // gestione segnali
    signal(SIGPIPE, sigpipe_handler);
    signal(SIGINT,customSigHandler);
    
    // apertura socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Errore nella creazione socket()\n");
        exit(-1);
    }
  
    // preparazione dell'indirizzo del server
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVERPORT);
    serverAddress.sin_addr.s_addr = inet_addr(SERVERADDRESS);
  
    // connessione socket al server
    if ((connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress))) < 0) {
        perror("Errore nella connessione socket.\n");
        exit(-1);
    }
  
    printf("Sei connesso a %s\n", SERVERADDRESS);

    // controllo argomenti per login diretto
    if (argc == 1)
        requestHome(clientSocket, buffer);
    else if (argc >= 4 && parser(argv) == 1)
        login(clientSocket, buffer);            
    else {
        printf("Per il login diretto utilizzare: -a username password\nAccesso come visitatore.\n");
        requestHome(clientSocket, buffer);
    }

    while(1){
        // attende risposta dal server
        strcpy(buffer.message, "");
        if((recv(clientSocket, &buffer, sizeof(buffer), 0)) < 0) {
            perror("Errore nella ricezione dei dati.\n");
            exit(-1);
        } else {
            if(strcmp(buffer.message, "x") == 0) {
                close(clientSocket);
                printf("Disconnesso da %s\n", SERVERADDRESS);
                exit(0);
            }
            else
                printf("\nServer >>\n%s", buffer.message);
        }

        // Controllo e pulizia stream input
        clean_stdin();

        // manda messaggio al server
        printf(">>\t");
        fgets(buffer.message, BUFFER_MAX, stdin);
        printf("\n");

        // se la stringa inviata è maggiore della dimensione permessa viene tagliata
        if (strlen(buffer.message) > INPUT_MAX)
            buffer.message[INPUT_MAX - 1] = '\0';
        else
            buffer.message[strlen(buffer.message) - 1] = '\0';

        send(clientSocket, &buffer, sizeof(buffer), 0);
    }
  
    return(0);
}