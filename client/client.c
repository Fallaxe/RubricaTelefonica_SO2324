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

    printf("argc = %d\n", argc);
    
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
    *  1) viene mandato h se non si passa parametri, senno si manda -l
    */
    if(argc >= 4){
        parser(argv,argc);
        
        send(clientSocket, "l", strlen("l"), 0);

        if((returnCode = recv(clientSocket, buffer, BUFFER_MAX, 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        }
        
        strcpy(buffer, cred.user);
        strcat(buffer," ");
        strcat(buffer,cred.password);

        send(clientSocket, buffer, sizeof(buffer), 0);
    }
    else if (argc > 1)
    {
        printf("User e password non corretti.\n");
        send(clientSocket, home, strlen(home), 0);
    }
    else send(clientSocket, home, strlen(home), 0); 
    
    while(1){
        /* attende risposta dal server */
        if((returnCode = recv(clientSocket, buffer, BUFFER_MAX, 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
            buffer[returnCode] = '\0';

            // if(strcmp(buffer, "l") == 0) {
            //     strcpy(buffer, cred.user);
            //     strcat(buffer," ");
            //     strcat(buffer,cred.password);
            //     send(clientSocket, buffer, sizeof(buffer), 0);
            // } else {
            //     printf("Server:\n%s\n" ,buffer);
            // }

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

int login(char *user, char *password)
{


    return 0;
}

void parser(char *argomenti[],int max){
    // printf("%s", argomenti[1]);
    // printf("%s", argomenti[2]);
    // printf("%s", argomenti[3]);
    int i;
    for (i = 1; i < max-3; i++)
    {
        if(strcmp(argomenti[i],"-a")){
            strcpy(cred.user,argomenti[i+2]);
            strcpy(cred.password, argomenti[i+3]);
            printf("user: %s\n", cred.user);
            printf("pass: %s\n", cred.password);
            break;
        }
        
    }

    if(i == max-2){
        printf("Usare: -a user password\n");
        exit(1);
    }

    
    

    // if(strcmp(argomenti[1],"-a") == 0){
    //     //printf("è vero bro\n");
    //     strcpy(cred.user,argomenti[2]);
    //     strcpy(cred.password, argomenti[3]);
    //     printf("user: %s\n", cred.user);
    //     printf("pass: %s\n", cred.password);
    // }
}