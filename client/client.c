#include "client.h"

int main(int argc, char * argv[]) {
    struct sockaddr_in serverAddress;
    int returnCode;
  
    /* buffer bidirezionale per messaggi */
    MSG buffer;
    buffer.isAdmin = 0;
    signal(SIGPIPE, sigpipe_handler);
    signal(SIGINT,customSigHandler);

    //printf("argc = %d\n", argc);
    
    /* apertura socket del client */
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Errore nella creazione socket()");
        exit(-1);
    }
  
    /* preparazione dell'indirizzo del server */
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVERPORT);
    serverAddress.sin_addr.s_addr = inet_addr(SERVERADDRESS);
  
    /* connessione socket al server */
    if ((connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress))) < 0) {
        perror("Errore nella connessione socket.");
        exit(-1);
    }
  
    printf("Sei connesso a %s\n", SERVERADDRESS);

    /* AVVIO DELLA CONNESSIONE:
    *  1) Se si avvia con parametri si prova un login, altrimenti si entra come visitatore
    */
    if (argc == 1) {
        requestHome(clientSocket, buffer);
    } else if (argc >= 4 && parser(argv,argc) == 1) {
        login(clientSocket, buffer);            
    } else {
        printf("Impossibile fare il login.\nUtilizzare: -a username password\n");
        requestHome(clientSocket, buffer);
    }
    
    while(1){
        /* attende risposta dal server */
        /* aggiunti per capire meglio dei [client/server] al printf, poi si levano magari*/
        if((recv(clientSocket, &buffer, sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {

            if(strcmp(buffer.message, "x") == 0) {
                close(clientSocket);
                printf("Disconnesso da %s\n", SERVERADDRESS);
                exit(0);
            } else {
                printf("\n[server said]>>%s", buffer.message);
            }
        }
    
        /* manda messaggio al server */
        printf("\n[Write Prompt]>>");
        fgets(buffer.message, BUFFER_MAX, stdin);

        if ((strlen(buffer.message) > 0) && (buffer.message[strlen(buffer.message) - 1] == '\n'))
            buffer.message[strlen (buffer.message) - 1] = '\0';
        
        send(clientSocket, &buffer, sizeof(buffer), 0);
    }
  
    return(0);
}

void login(int clientSocket, MSG buffer)
{
    strcpy(buffer.message, "l");
    send(clientSocket, &buffer, sizeof(buffer), 0);

    if(recv(clientSocket,&buffer, sizeof(buffer), 0) < 0) {
        printf("Errore nella ricezione dei dati.\n");
    }
        
    strcpy(buffer.message, cred.user);
    send(clientSocket,&buffer, sizeof(buffer), 0);

    if(recv(clientSocket,&buffer, sizeof(buffer), 0) < 0) {
        printf("Errore nella ricezione dei dati.\n");
    }

    strcpy(buffer.message,cred.password);
    send(clientSocket,&buffer, sizeof(buffer), 0);
}

int parser(char *argomenti[],int max){
    int i;
    for (i = 1; i < max-2; i++)
    {
        if(strcmp(argomenti[i],"-a") == 0){
            strcpy(cred.user,argomenti[i+1]);
            strcpy(cred.password, argomenti[i+2]);
            //printf("user: %s\n", cred.user);
            //printf("pass: %s\n", cred.password);
            break;
        }
    }

    if(i == max-2){
        return 0;
    }

    return 1;
}

void requestHome(int clientSocket, MSG buffer) {
    strcpy(buffer.message, home);
    send(clientSocket, &buffer, sizeof(buffer), 0);
}

void customSigHandler(){
    printf("\ninterruzione ricevuta: chiusura socket.");
    close(clientSocket);
    exit(1);
}
void sigpipe_handler(int signo) {
    printf("Connessione chiusa dal server.\n");
    close(clientSocket);
    exit(0);  // Esci dall'applicazione o gestisci la riconnessione
}