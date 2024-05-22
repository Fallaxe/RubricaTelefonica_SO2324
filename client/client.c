#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "client.h"

int main(int argc, char * argv[]) {
    int clientSocket;
    struct sockaddr_in serverAddress;
    int returnCode;
  
    /* buffer bidirezionale per messaggi */
    char buffer[BUFFER_MAX];
  
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
    send(clientSocket, home, strlen(home), 0);
  
    while(1){
        /* attende risposta dal server */
        if((returnCode = recv(clientSocket, buffer, BUFFER_MAX, 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
            buffer[returnCode] = '\0';
            if(strcmp(buffer, "x") == 0) {
                close(clientSocket);
                printf("Disconnesso da %s\n", SERVERADDRESS);
                exit(0);
            } else {
                printf("Server:\n%s\n" ,buffer);
            }
        }
    
        /* manda messaggio al server */
        printf("\nDigita scelta:\t");
        scanf("%s", &buffer[0]);
        send(clientSocket, buffer, sizeof(buffer), 0);
    }
  
    return(0);
}