#include "client.h"

// Gestione dei segnali
static void customSigHandler() {
    printf("\nInterruzione ricevuta: chiusura socket.\n");
    close(clientSocket);
    exit(1);
}
static void sigpipe_handler() {
    printf("Connessione chiusa dal server.\n");
    close(clientSocket);
    exit(0);
}
///////////////////////////////////////////////////////////////////

static int parser(char *argomenti[],int max) {
    int i;

    for (i = 1; i < max-2; i++)
    {
        if(strcmp(argomenti[i], loginArg) == 0){
            strcpy(cred.user,argomenti[i+1]);
            strcpy(cred.password, argomenti[i+2]);
            break;
        }
    }
    if(i == max-2)
        return 0;

    return 1;
}

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
    signal(SIGPIPE, sigpipe_handler);
    signal(SIGINT,customSigHandler);
    
    // apertura socket del client
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

    // controllo argomenti
    if (argc == 1) {
        requestHome(clientSocket, buffer);
    } else if (argc >= 4 && parser(argv,argc) == 1) {
        login(clientSocket, buffer);            
    } else {
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
    
        // manda messaggio al server
        printf(">>\t");
        fgets(buffer.message, BUFFER_MAX, stdin);
        printf("\n");

        if ((strlen(buffer.message) > 0) && (buffer.message[strlen(buffer.message) - 1] == '\n'))
            buffer.message[strlen (buffer.message) - 1] = '\0';
        
        send(clientSocket, &buffer, sizeof(buffer), 0);
    }
  
    return(0);
}