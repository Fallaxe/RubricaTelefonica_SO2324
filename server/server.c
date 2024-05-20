/*
    -> ascolto connessioni
    -> messaggio di benvenuto
    -> la gestione delle varie connessioni è gestita da una fork()
    -> comunicazione bidirezionale?
*/

#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

void sendMenu(int connectSocket)
{
    send(connectSocket,benvenuto,strlen(benvenuto),0);
    send(connectSocket,scelte,strlen(scelte),0);
}

int choiseHandler(char *choise)
{
    switch(*choise)
    {
    case 'v':
        printf("%s",visita);
        break;
    case 'l':
        printf("login admin");
        break;
    case 'x':
        printf("esci");
        break;
    case 'm':
        /* test sul flag se sei admin;
        */
        break;
    // case 5:
    //     break;
    // case 6:
    //     break;
    default:
        break;
    }
}



void main(int argc, char const *argv[])
{
    int serverSocket, connectSocket, returnCode;
    socklen_t clientAddressLen;
    struct sockaddr_in serverAddress, clientAddress;
    char buffer[BUFFER_MAX];
    char *clientIP;

    if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("errore nella creazione del socket!");
        exit(serverSocket);
    }   

    serverAddress.sin_family = AF_INET;         // IPv4
    serverAddress.sin_port = htons(SERVERPORT); // su quale porta apriamo
    serverAddress.sin_addr.s_addr = inet_addr(SERVERADDRESS);// indirizzo server

    // bind del socket
    if((returnCode = bind(serverSocket,(struct sockaddr*) &serverAddress, sizeof(serverAddress)))< 0){
        perror("errore nel binding");
        exit(42);
    }

    // listen
    if((returnCode = listen(serverSocket,MAX_CLIENT)) < 0){
        perror("errore nel listening");
        exit(-1);
    }

    printf("server online CTRL+C per terminare\n");

    clientAddressLen = sizeof(clientAddress);

    while (1)
    {
        if((connectSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen))<0){
            perror("errore in accept");
            close(serverSocket);
            exit(-1);
        }
        if(fork() == 0){
            clientIP = inet_ntoa(clientAddress.sin_addr);
            printf("client connected! @ %s : %d\n",clientIP, connectSocket);

            sendMenu(connectSocket);
        }   
        else 
            close(connectSocket); 
    }
    

}
