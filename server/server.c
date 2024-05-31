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
    strcat(buffer.message, scelte);
    if (buffer.isAdmin == 1) {
        strcat(buffer.message, scelteadmin); // <-- qui va inserito il menu da admin
    }
    strcat(buffer.message, sceltaUscita);

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
    if (login == 0) printf("Login non effettuato.\n");
    fclose(fptr);
    return login;
}

void readContacts()
{
// TODO: c'è solo un prototipo di come fare letto su internet
//       in particolare va riletta tutta la cosa e fatto il parsing con le funzioni cJSON.
//       probabilmente prima va letto che è un array di oggetti e poi gli item uno ad uno.
//         // open the file 
//     FILE *fp = fopen("data.json", "r"); 
//     if (fp == NULL) { 
//         printf("Error: Unable to open the file.\n"); 
//         //return 1;
//         exit(42); 
//     } 
  
//     // read the file contents into a string 
//     char buffer[1024]; 
//     int len = fread(buffer, 1, sizeof(buffer), fp); 
//     fclose(fp); 

//     // parse the JSON data 
//     cJSON *json = cJSON_Parse(buffer); 
//     if (json == NULL) { 
//         const char *error_ptr = cJSON_GetErrorPtr(); 
//         if (error_ptr != NULL) { 
//             printf("Error: %s\n", error_ptr); 
//         } 
//         cJSON_Delete(json); 
//         return 1; 
//     } 
  
//     // access the JSON data 
//     cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name"); 
//     if (cJSON_IsString(name) && (name->valuestring != NULL)) { 
//         printf("Name: %s\n", name->valuestring); 
//     } 
  
//     // delete the JSON object 
//     cJSON_Delete(json); 
//     return 0;
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

void aggiungiPersona(int connectSocket, MSG buffer){
        //PROBLEMA: la modifica al "data.json" NON appare prima della chiusura del server.
        //SOLUZIONE: fflush() prima del fclose Quindi NON cancellarlo!
        //TODO: per il momento aggiunge e basta un oggetto al file, va fatto si che lo aggiunga
        //      all'array di item nel file, allo stato attuale sovrascrive l'intero file!
        //      1) lettura del file al boot del server?
        //      2) lettura del file solo su 'm'

        t_person persona;

        strcpy(buffer.message, "puoi modificare!\nInserire nome del contatto :\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
            strcpy(persona.name, buffer.message); 
            printf("Client - New contact: %s\n", buffer.message);
        }

        strcpy(buffer.message, "Inserire eta del contatto :\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
            persona.age = atoi(buffer.message);
            printf("Client - New contact: %s\n", buffer.message);
        }

        strcpy(buffer.message, "Inserire email del contatto :\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
            strcpy(persona.email, buffer.message);
            printf("Client - New contact: %s\n", buffer.message);
        }

        cJSON *jsonItem = cJSON_CreateObject();
        cJSON_AddStringToObject(jsonItem, "name", persona.name); 
        cJSON_AddNumberToObject(jsonItem, "age", persona.age); 
        cJSON_AddStringToObject(jsonItem, "email", persona.email);

        //creazione array di  oggetti json
        cJSON *jsonArray = cJSON_CreateArray();
        cJSON_AddItemToArray(jsonArray,jsonItem);

        //creazione di una stringa in formato json con tutti gli oggetti
        char *json_str = cJSON_Print(jsonArray);
        
         // write the JSON string to a file 
        FILE *fp = fopen("data.json", "w"); 
        if (fp == NULL) { 
            printf("impossibile aprire il file dei dati\n"); 
            exit(42);
        }
        printf("%s\n", json_str); 
        fputs(json_str, fp); 
        fflush(fp);
        fclose;
        // free the JSON string and cJSON object 
        cJSON_free(json_str);
        //cJSON_Delete(jsonArray);
        cJSON_Delete(jsonItem);
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
            aggiungiPersona(connectSocket, buffer);
            strcpy(buffer.message, ""); //pulisce buffer
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