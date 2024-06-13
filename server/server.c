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

void printContent()
{
    cJSON *jsonArray;
    char fileContent[1024];

    // apre file data in lettura
    FILE *fp = fopen("data.json", "r"); 
    if (fp == NULL) {
    // se il database non esiste
        printf("non esiste il file\n");
    } else {  
        // legge il file su una stringa 
        int len = fread(fileContent, 1, sizeof(fileContent), fp);
        fclose(fp);
  
        // parse della string con JSON
        jsonArray = cJSON_Parse(fileContent);

        cJSON *element;

        for (int i = 0; i < cJSON_GetArraySize(jsonArray); i++)
        {   
            //non invia ancora al client ma print solo al server
            //problema: se mandiamo  l'intera lista il buffer dovrebbe andare in overflow sulla recezione/invio? (non riceve tutto credo)
            element = cJSON_GetArrayItem(jsonArray,i);
            printf("elemento: %s,%d,%s\n",cJSON_GetObjectItem(element,"name")->valuestring,cJSON_GetObjectItem(element,"age")->valueint,cJSON_GetObjectItem(element,"email")->valuestring);
        }

    }
        //#######################################################
    
}

int verifica(t_credenziali cred)
{
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

int aggiungiPersona(int connectSocket, MSG buffer){
    //return 1 aggiunto correttamente
    //return 0 problema all'aggiunta!
        //PROBLEMA: la modifica al "data.json" NON appare prima della chiusura del server.
        //SOLUZIONE: fflush() prima del fclose Quindi NON cancellarlo!
        //TODO: per il momento aggiunge e basta un oggetto al file, va fatto si che lo aggiunga
        //      all'array di item nel file, allo stato attuale sovrascrive l'intero file!
        //      1) lettura del file al boot del server?
        //      2) lettura del file solo su 'm'

        //t_person persona;  // <- se li mettiamo direttamente sul json non serve
        
        // Legge Array con i Contatti ##############################
        cJSON *jsonArray;
        char fileContent[1024];

        // apre file data in lettura
        FILE *fp = fopen("data.json", "r"); 
        if (fp == NULL) {
            // se il database non esiste
            jsonArray = cJSON_CreateArray();
        } else {  
            // legge il file su una stringa 
            int len = fread(fileContent, 1, sizeof(fileContent), fp);
            fclose(fp);
  
            // parse della string con JSON
            jsonArray = cJSON_Parse(fileContent); 
            if (jsonArray == NULL) {
                // se database esiste ma è vuoto (tipo se diamo la possibilità di eliminare contatti)
                jsonArray = cJSON_CreateArray();
            }
        }
        //#######################################################

        cJSON *jsonItem = cJSON_CreateObject();

        strcpy(buffer.message, "Nuovo Contatto\nInserire nome del contatto :\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
            //strcpy(persona.name, buffer.message);
            if(strlen(buffer.message) > 12){
                printf("l'utente ha superato i limiti imposti\n");
                return 0;
            }
            cJSON_AddStringToObject(jsonItem, "name", buffer.message);
            printf("Client - New contact: %s\n", buffer.message);
        }

        strcpy(buffer.message, "");
        strcpy(buffer.message, "Inserire eta del contatto :\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
            
            if(atoi(buffer.message) > 100 || atoi(buffer.message) < 1){
                printf("l'utente ha superato i limiti imposti\n");
                return 0;
            }

            cJSON_AddNumberToObject(jsonItem, "age", atoi(buffer.message));
            printf("Client - New contact: %s\n", buffer.message);
        }

        strcpy(buffer.message, "Inserire email del contatto :\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
            
            if(strlen(buffer.message) > 25){
                printf("l'utente ha superato i limiti imposti\n");
                return 0;
            }
            cJSON_AddStringToObject(jsonItem, "email", buffer.message);
            printf("Client - New contact: %s\n", buffer.message);
        }

        // li ho inseriti direttamente quando legge la risposta del client
        //cJSON *jsonItem = cJSON_CreateObject();
        //cJSON_AddStringToObject(jsonItem, "name", persona.name); 
        //cJSON_AddNumberToObject(jsonItem, "age", persona.age); 
        //cJSON_AddStringToObject(jsonItem, "email", persona.email);

        //creazione array di  oggetti json
        cJSON_AddItemToArray(jsonArray,jsonItem);

        //creazione di una stringa in formato json con tutti gli oggetti
        char *json_str = cJSON_Print(jsonArray);
        
         // write the JSON string to a file 
        fp = fopen("data.json", "w"); 
        if (fp == NULL) { 
            printf("impossibile aprire il file dei dati\n"); 
            exit(42);
        }
        fputs(json_str, fp); 
        printf("%s\n", cJSON_Print(jsonItem)); //stampa solo la persona appena inserita, non tutto l'array
        
        fflush(fp);
        fclose;
        // free the JSON string and cJSON object 
        cJSON_free(json_str);
        //cJSON_Delete(jsonArray);
        cJSON_Delete(jsonItem);
        return 1;
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
    case 'v':
        printContent();
        break;
    case 'l':
        login(connectSocket, buffer);
        break;
    case 'x':
        break;
    case 'a':
        if (choise.isAdmin == 1){
            int isAdded = aggiungiPersona(connectSocket, buffer);
            strcpy(buffer.message, (isAdded ==1 ? 
            "aggiunto contatto!\n" :
            "errore nell'aggiunta:\nil nome deve essere minore di 12 \nl'età minore di 100 e maggiore di 0\ne la mail lunga al massimo 25 caratteri\n")); //pulisce buffer
        } else {
            strcpy(buffer.message, "Non hai l'autorizzazione a modificare.\n");
        }
        sendMenu(connectSocket, buffer);
        strcpy(buffer.message, "");
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
    close(serverSocket);
    if(ppidServerInit == getpid()){
        printf("\nProcesso padre(%d) terminato",ppidServerInit);
        exit(0);
    }
    printf("\nprocesso figlio con pid %d terminato\n",getpid());
    exit(1);
}


void main(int argc, char const *argv[])
{
    void (*handler) (int);
    ppidServerInit = getpid();
    
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
    
    //modo per evitare l'errore "errore nel binding: Address already in use" (SO_REUSEADDR)
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

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
            printf("client connected! @ %s : %d PID: %d PPID: %d\n",clientIP, connectSocket,getpid(),getppid());

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