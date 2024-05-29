/*
    -> ascolto connessioni
    -> messaggio di benvenuto
    -> la gestione delle varie connessioni è gestita da una fork()
    -> comunicazione bidirezionale?
*/

#include "server.h"

int serverSocket;

void sendMenu(int connectSocket, MSG buffer)
{
    if (buffer.isAdmin == 1) {
        strcat(buffer.message, scelte); // <-- qui va inserito il menu da admin
    } else {
        strcat(buffer.message, scelte);
    }
    send(connectSocket,&buffer, sizeof(buffer), 0);
}

int verifica(t_credenziali cred) {
    FILE * fptr;
    t_credenziali admin;
    int login = 0;

    fptr = fopen(FILE_USERS, "r");
    if (fptr == NULL) {
        printf("Errore file password non trovato.");
        return 0;
    } else {
        while(fscanf(fptr,"%s %s\n", admin.user, admin.password) != -1) {
            if ((strcmp(cred.user, admin.user) == 0) && (strcmp(cred.password, admin.password) == 0)) {
                login = 1;
                printf("Login effettuato.\n");
                break;                               
            }
        }
    }
    if (login == 0) printf("Login non effettato.\n");
    fclose(fptr);
    return login;
}

void login(int connectSocket, MSG buffer){
        if(buffer.isAdmin == 1) {
            strcpy(buffer.message, "Sei già loggato.\n");
            sendMenu(connectSocket, buffer);
            return;
        }
        t_credenziali cred;
        
        // Richiede User
        strcpy(buffer.message, "Inserire user:\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
            strcpy(cred.user, buffer.message);
            printf("Client - User: %s\n", buffer.message);
        }

        // Richiede Password
        strcpy(buffer.message, "Inserire password:\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
            strcpy(cred.password, buffer.message);
            printf("Client - Password: %s\n", buffer.message);
        }

        if ( verifica(cred) == 1) {
            buffer.isAdmin = 1;
            strcpy(buffer.message, "Sei entrato!\n");

        } else {
            strcpy(buffer.message, "Errore in username o password.\n");
        }
        sendMenu(connectSocket, buffer);
}

int choiseHandler(int connectSocket, MSG choise)
{
    MSG buffer;
    buffer.isAdmin = choise.isAdmin;

    switch(*choise.message)
    {
    case 'h':
        strcpy(buffer.message, benvenuto);
        sendMenu(connectSocket, buffer);
        break;
    //case 'v':
    //    printf("%s",visita);
    //    break;
    case 'l':
        login(connectSocket, buffer);
        break;
    case 'x':
        break;
    case 'm': // Per ora ho fatto solo il test per vedere se sei admin
        // test sul flag se sei admin
        if (choise.isAdmin == 1){
            strcpy(buffer.message, "Puoi modificare.\n");
        } else {
            strcpy(buffer.message, "Non hai l'autorizzazione a modificare.\n");
        }
        sendMenu(connectSocket, buffer);
        break;
    //    break;
    // case 5:
    //     break;
    // case 6:
    //     break;
    default:
        strcpy(buffer.message, "Comando non riconosciuto.\n");
        sendMenu(connectSocket, buffer);
        break;
    }
}

void customSigHandler(){
    printf("addio\n");
    close(serverSocket);
    exit(1);
}


void main(int argc, char const *argv[])
{
    void (*handler) (int);

    
    int connectSocket, returnCode;
    socklen_t clientAddressLen;
    struct sockaddr_in serverAddress, clientAddress;
    char *clientIP;

    MSG buffer;
    buffer.isAdmin = 0;

    if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("errore nella creazione del socket!");
        exit(serverSocket);
    }   

    signal(SIGINT,customSigHandler);

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

            while(1) {
                // attende richiesta dal client                
                if((returnCode = recv(connectSocket, &buffer, sizeof(buffer), 0)) < 0) {
                    printf("Errore nella ricezione dei dati.\n");
                } else {
                    printf("Client - isAdmin: %d, Message %s\n", buffer.isAdmin, buffer.message);
                    
                    // gestione delle richieste                
                    choiseHandler(connectSocket, buffer);
                }
                
                if(strcmp(buffer.message, "x") == 0) {
                    send(connectSocket, &buffer, sizeof(buffer), 0);
                    close(connectSocket);
                    printf("Client @ %s disconnesso.\n", clientIP);
                    break;
                }

                fflush(stdout);
                fflush(stdin);
                
            }

        }   
        else 
            close(connectSocket); 
    }
    

}