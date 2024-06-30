/*
    -> ascolto connessioni
    -> messaggio di benvenuto
    -> la gestione delle varie connessioni è gestita da una fork()
    -> comunicazione bidirezionale?
*/

#include "server.h"

int removeFromList(cJSON *found, cJSON* list,int connectSocket, MSG buffer)
{
    int num = -1;
    int nContacts = cJSON_GetArraySize(found);

    do
    {
        strcat(buffer.message,"Inserisci l'indice del contatto da eliminare. x per tornare al menù\n");
        send(connectSocket,&buffer, sizeof(buffer), 0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0){
            printf("Errore nella ricezione dei dati.\n");
            return 0;
        }
        
        if (strcmp(utils_lowercase(buffer.message), "x") == 0)
            return 0;
        num = atoi(buffer.message);
        strcpy(buffer.message, "");
    } while(num < 1 || num > nContacts);

    // trova elemento corrispondente nell'elenco contatti
    int index = 0;
    cJSON * elementDeleted = cJSON_GetArrayItem(found, (num-1));
    cJSON * element = list->child;
    while(element){
        if(strcmp(cJSON_Print(elementDeleted), cJSON_Print(element)) == 0)
                break;
        index ++;
        element= element->next;
    }
    
    char elementStr[1024];
    snprintf(elementStr, sizeof(elementStr), "Contatto:\n\tNome: %s\n\tCognome: %s\n\tEtà: %d\n\tEmail: %s\n\tTelefono: %s\nATTENZIONE. Vuoi confermare la rimozione? [Y/N]\n",
                                            cJSON_GetObjectItem(element,"name")->valuestring, cJSON_GetObjectItem(element,"surname")->valuestring,
                                            cJSON_GetObjectItem(element,"age")->valueint, cJSON_GetObjectItem(element,"email")->valuestring,
                                            cJSON_GetObjectItem(element,"phone")->valuestring); 
    strcpy(buffer.message, elementStr);
    send(connectSocket,&buffer, sizeof(buffer), 0);

    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return 0;
    }
    if(strcmp(utils_lowercase(buffer.message), "y") != 0)
        return 0;

    printf("Client - User: eliminazione di %s\n", cJSON_Print(element));
        
    //cancella dall'array all'indirizzo selezionato
    cJSON_DeleteItemFromArray(list,index);
    saveDatabase(list);
    return 1;
}

int editFromList(cJSON *found, cJSON *list, int connectSocket, MSG buffer)
{
    int num = -1;
    int nContacts = cJSON_GetArraySize(found);

    do
    {
        strcat(buffer.message,"Inserisci l'indice del contatto da modificare. x per tornare al menù\n");
        send(connectSocket,&buffer, sizeof(buffer), 0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0){
            printf("Errore nella ricezione dei dati.\n");
            return 0;
        }
        
        if (strcmp(utils_lowercase(buffer.message), "x") == 0)
            return 0;
        num = atoi(buffer.message);
        printf("hai digitato: %s, num: %d\n", buffer.message, num);
        strcpy(buffer.message, "");
    } while(num < 1 || num > nContacts);

    // trova elemento corrispondente nell'elenco contatti
    int index = 0;
    cJSON * elementDeleted = cJSON_GetArrayItem(found, (num-1));
    cJSON * element = list->child;
    while(element){
        if(strcmp(cJSON_Print(elementDeleted), cJSON_Print(element)) == 0)
                break;
        index ++;
        element= element->next;
    }

    char elementStr[1024];
    snprintf(elementStr, sizeof(elementStr), "Contatto:\n\tNome: %s\n\tCognome: %s\n\tEtà: %d\n\tEmail: %s\n\tTelefono: %s\nATTENZIONE. Vuoi modificarlo? [Y/N]\n",
                                            cJSON_GetObjectItem(element,"name")->valuestring, cJSON_GetObjectItem(element,"surname")->valuestring,
                                            cJSON_GetObjectItem(element,"age")->valueint, cJSON_GetObjectItem(element,"email")->valuestring,
                                            cJSON_GetObjectItem(element,"phone")->valuestring); 
    strcpy(buffer.message, elementStr);
    send(connectSocket,&buffer, sizeof(buffer), 0);

    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return 0;
    }
    if(strcmp(utils_lowercase(buffer.message), "y") != 0)
        return 0;

    //edit
    printf("Client - User: modifica di %s\n", cJSON_Print(element));
    cJSON* nuovaPersona = creaPersona(connectSocket, buffer);

    if(nuovaPersona != NULL)
    {
        printf("Client - User: modificato con %s\n", cJSON_Print(nuovaPersona));
        cJSON_ReplaceItemInArray(list,index,nuovaPersona);
        saveDatabase(list);
        return 1;
    }
    cJSON_Delete(nuovaPersona);
    return 0;   
}

void sendMenu(int connectSocket, MSG buffer)
{
    strcat(buffer.message, divisore);
    strcat(buffer.message, menuHeader);
    strcat(buffer.message, divisore);
    strcat(buffer.message, "\n");
    
    strcat(buffer.message, scelte);
    
    // menu da admin
    if (buffer.isAdmin == 1)
        strcat(buffer.message, scelteadmin);
    else
        strcat(buffer.message, scelteLogin);
    
    // scelta per uscire
    strcat(buffer.message, sceltaUscita);

    send(connectSocket,&buffer, sizeof(buffer), 0);
}

static cJSON * loadDatabase()
{
    cJSON *jsonArray;

    // apre file data in lettura
    FILE *fp = fopen("data.json", "r"); 
    if (fp == NULL) {
        // se il database non esiste
        printf("Errore in lettura database: database non trovato.\n");
        return NULL;
    } else {  
        // calcolo dimensione del database
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // memoria per database
        char *fileContent = calloc((size + 1), sizeof(char));    
        fread(fileContent, sizeof(char), size, fp);

        fflush(fp);
        fclose(fp);

        // parse della stringa json
        jsonArray = cJSON_Parse(fileContent);

        free(fileContent);
    }
    return jsonArray;
}

void saveDatabase(cJSON * jsonArray){
    //creazione di una stringa in formato json con tutti gli oggetti
        utils_sortByKey(jsonArray, "surname");
        char *json_str = cJSON_Print(jsonArray);
     // write the JSON string to a file 
        FILE * fp = fopen("data.json", "w"); 
        if (fp == NULL) { 
            printf("impossibile aprire il file dei dati\n"); 
            exit(42);
        }
        fputs(json_str, fp);

        fflush(fp);
        fclose;
        // free the JSON string and cJSON object 
        cJSON_free(json_str);
}

cJSON *creaPersona(int connectSocket, MSG buffer)
{
    cJSON *jsonItem = cJSON_CreateObject();
    
    // banner
    strcpy(buffer.message, divisore);
    strcat(buffer.message, contattoHeader);
    strcat(buffer.message, divisore);

    // creazione del contatto
    strcat(buffer.message, "Inserire nome del contatto :\t");
    send(connectSocket,&buffer, sizeof(buffer),0);

    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return NULL;
    } 
    else {
        while(strlen(buffer.message) > 12) {
            strcpy(buffer.message, "Attenzione, massimo 12 caratteri.\nInserire nome del contatto :\t");
            send(connectSocket,&buffer, sizeof(buffer),0);

            if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
                printf("Errore nella ricezione dei dati.\n");
                return NULL;
            } 
        }
        cJSON_AddStringToObject(jsonItem, "name", buffer.message);
    }

    strcpy(buffer.message, "Inserire cognome del contatto :\t");
    send(connectSocket,&buffer, sizeof(buffer),0);

    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return NULL;
    } 
    else {
        while(strlen(buffer.message) > 12) {
            strcpy(buffer.message, "Attenzione, massimo 12 caratteri.\nInserire cognome del contatto :\t");
            send(connectSocket,&buffer, sizeof(buffer),0);

            if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
                printf("Errore nella ricezione dei dati.\n");
                return NULL;
            } 
        }
        cJSON_AddStringToObject(jsonItem, "surname", buffer.message);
    }

    strcpy(buffer.message, "Inserire eta del contatto :\t");
    send(connectSocket,&buffer, sizeof(buffer),0);

    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return NULL;
     } 
    else {
        while(atoi(buffer.message) > 100 || atoi(buffer.message) < 1){
            strcpy(buffer.message, "Attenzione, deve essere un numero tra 0 e 100.\nInserire eta del contatto :\t");
            send(connectSocket,&buffer, sizeof(buffer),0);

            if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
                printf("Errore nella ricezione dei dati.\n");
                return NULL;
            } 
        }
        cJSON_AddNumberToObject(jsonItem, "age", atoi(buffer.message));
    }

    strcpy(buffer.message, "Inserire email del contatto :\t");
    send(connectSocket,&buffer, sizeof(buffer),0);

    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return NULL;
    } 
    else {
        while(strlen(buffer.message) > 30){
            strcpy(buffer.message, "Attenzione, deve avere massimo 30 caratteri.\nInserire email del contatto :\t");
            send(connectSocket,&buffer, sizeof(buffer),0);

            if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
                printf("Errore nella ricezione dei dati.\n");
                return NULL;
            } 
        }
        cJSON_AddStringToObject(jsonItem, "email", buffer.message);
    }

    strcpy(buffer.message, "Inserire telefono del contatto :\t");
    send(connectSocket,&buffer, sizeof(buffer),0);

    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return NULL;
    } 
    else {
        while(strlen(buffer.message) > 16 || utils_strIncludeOnly(buffer.message, "+ 1234567890") == 0){
            strcpy(buffer.message, "Attenzione, massimo 16 caratteri.\nInserire telefono del contatto :\t");
            send(connectSocket,&buffer, sizeof(buffer),0);

            if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
                printf("Errore nella ricezione dei dati.\n");
                return NULL;
            } 
        }
         cJSON_AddStringToObject(jsonItem, "phone", buffer.message);
    }

    char elementStr[1024];
    snprintf(elementStr, sizeof(elementStr), "Aggiungo:\n\tNome: %s\n\tCognome: %s\n\tEtà: %d\n\tEmail: %s\n\tTelefono: %s\nVuoi confermare? [Y/N]\n",
                                            cJSON_GetObjectItem(jsonItem,"name")->valuestring, cJSON_GetObjectItem(jsonItem,"surname")->valuestring,
                                            cJSON_GetObjectItem(jsonItem,"age")->valueint, cJSON_GetObjectItem(jsonItem,"email")->valuestring,
                                            cJSON_GetObjectItem(jsonItem,"phone")->valuestring); 
    strcpy(buffer.message, elementStr);
    send(connectSocket,&buffer, sizeof(buffer), 0);

    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return NULL;
    } else if(strcmp(utils_lowercase(buffer.message), "y") != 0) {
        return NULL;
    }
    return jsonItem;
}

MSG printContent(cJSON * array, int connectSocket,MSG buffer)
{
    char elementString[1024];
    int nContacts = 0;
    int contactsInPage = 4;
    int nPage = 1;
    
    if (array == NULL || cJSON_GetArraySize(array) == 0)
        strcat(buffer.message, "Non ci sono contatti salvati.\n\n");
    else
    {  
        nContacts = cJSON_GetArraySize(array);

        cJSON *element = array->child;
        int i = 1;


        while (element)
        {   
            //non invia ancora al client ma print solo al server
            //problema: se mandiamo  l'intera lista il buffer dovrebbe andare in overflow sulla recezione/invio? (non riceve tutto credo)
            //UPDATE: Per adesso invia 3 contatti per volta e chiede se si vuole cambiare pagina per continuare.

            // Inizio pagina
            if(i%(contactsInPage) == 1) {
                strcpy(buffer.message, divisore);
                strcat(buffer.message, cercaHeader);
                strcat(buffer.message, divisore);
                snprintf(elementString, sizeof(elementString), "%d contatti trovati. Pagina %d di %d.\n", nContacts, nPage, ((nContacts+(contactsInPage-1))/contactsInPage)); 
                strcat(buffer.message, elementString);
            }

            // Stringa Contatto
            snprintf(elementString, sizeof(elementString), "%d) Nome: %s\n   Cognome: %s\n   Età: %d\n   Email: %s\n   Telefono: %s\n\n",i,cJSON_GetObjectItem(element,"name")->valuestring,cJSON_GetObjectItem(element,"surname")->valuestring,cJSON_GetObjectItem(element,"age")->valueint,cJSON_GetObjectItem(element,"email")->valuestring,cJSON_GetObjectItem(element,"phone")->valuestring); 
            //printf("dimensioni elemento: %ld\n", strlen(elementString));
            strcat(buffer.message, elementString);

            // Fine pagina
            if (i < nContacts && i%contactsInPage == 0)
            {
                strcat(buffer.message, "Vuoi vedere la pagina successiva? [Y/N]\n");
                send(connectSocket,&buffer, sizeof(buffer),0);

                if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0)
                    printf("Errore nella ricezione dei dati.\n");
                else {
                    strcpy(cred.user, buffer.message);
                    printf("Client - User: %s\n", buffer.message);
                }

                strcpy(buffer.message, "");
                if (strcmp(utils_lowercase(buffer.message), "y") != 0)
                    break;
            }

            element = element->next;
            i++;
        }
    }
    return buffer;
}

MSG readContent(int connectSocket, MSG buffer)
{
    cJSON *jsonArray = loadDatabase();
    strcpy(buffer.message, "");
    return printContent(jsonArray,connectSocket,buffer);
}

MSG search(int connectSocket, MSG buffer, operationOnList op)
{   
    cJSON *jsonArray = loadDatabase();
    cJSON *foundArr = cJSON_CreateArray();
    char searchName[25];
    
    if (jsonArray == NULL || cJSON_GetArraySize(jsonArray) == 0) {
        strcpy(buffer.message, "Non ci sono contatti salvati.\n");
        return buffer;
    }
        
    strcpy(buffer.message, "Inserire nome della persona da cercare:\n");
    send(connectSocket,&buffer, sizeof(buffer),0);

    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0)
        printf("Errore nella ricezione dei dati.\n");
    else {
        strcpy(searchName, buffer.message);
        printf("Client - Ricerca: %s\n", buffer.message);
    }

    cJSON *element = jsonArray->child;
    char nameSurname[25];
    char surnameName[25];
    while (element)
    {
        snprintf(nameSurname, sizeof(nameSurname), "%s %s", cJSON_GetObjectItem(element,"name")->valuestring, cJSON_GetObjectItem(element,"surname")->valuestring);
        snprintf(surnameName, sizeof(surnameName), "%s %s", cJSON_GetObjectItem(element,"surname")->valuestring, cJSON_GetObjectItem(element,"name")->valuestring);

        if(strstr((nameSurname), utils_lowercase(searchName)) || strstr(utils_lowercase(surnameName), utils_lowercase(searchName)))
            cJSON_AddItemToArray(foundArr, cJSON_Duplicate(element, 1));

        element = element->next;
    }

    cJSON_Delete(element);

    strcpy(buffer.message, "");    
        
    if(cJSON_GetArraySize(foundArr) > 0)
    {
        buffer = printContent(foundArr,connectSocket,buffer);
            
        if(op != NULL)
            strcpy(buffer.message, (op(foundArr,jsonArray,connectSocket, buffer)? "Modifica dei contatti effettuata.\n" : "Nessuna modifica effettuata\n"));
    }
    else
        strcpy(buffer.message, "Nessun contatto con questo nome.\n");

    return buffer;
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
        char hashPSWadmin[CONVERTION_SHA256_MAX];
        while(fscanf(fptr,"%s %s\n", admin.user, hashPSWadmin) != -1) {

            char hashPSWinserita[CONVERTION_SHA256_MAX];
            inToSha256(cred.password,hashPSWinserita);

            if ((strcmp(cred.user, admin.user) == 0) && (strcmp(hashPSWadmin, hashPSWinserita) == 0)) {
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

MSG login(int connectSocket, MSG buffer){
    if(buffer.isAdmin == 1) {
        strcpy(buffer.message, "Sei già loggato.\n");
        return buffer;
    }
    t_credenziali cred;
        
    // Richiede User
    strcpy(buffer.message, "Inserire user:      ");
    send(connectSocket,&buffer, sizeof(buffer),0);

    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0)
        printf("Errore nella ricezione dei dati.\n");
    else {
        strcpy(cred.user, buffer.message);
        printf("Client - User: %s\n", buffer.message);
    }

    // Richiede Password
    strcpy(buffer.message, "Inserire password:  ");
    send(connectSocket,&buffer, sizeof(buffer),0);

    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0)
        printf("Errore nella ricezione dei dati.\n");
    else {
        strcpy(cred.password, buffer.message);
        printf("Client - Password: %s\n", buffer.message);
    }

    if (verifica(cred)) {
        buffer.isAdmin = 1;
        strcpy(buffer.message, "Login effettuato con successo.\n\n");
    } else
        strcpy(buffer.message, "Attenzione username o password errati.\n\n");

    return buffer;
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
        
        cJSON *jsonArray = loadDatabase();

        if (jsonArray == NULL) {
            printf("Non sono presenti contatti. Creazione nuovo database.\n");
            jsonArray = cJSON_CreateArray();
        }

        cJSON* jsonItem = creaPersona(connectSocket, buffer);
        if(jsonItem != NULL)
        {
            cJSON_AddItemToArray(jsonArray,jsonItem);
            printf("Inserito:\n%s\n", cJSON_Print(jsonItem)); //stampa solo la persona appena inserita, non tutto l'array
            saveDatabase(jsonArray);
            cJSON_Delete(jsonItem);
            return 1;
        }
        cJSON_Delete(jsonItem);
        return 0;
}

int choiseHandler(int connectSocket, MSG buffer,sem_t *sem)
{
    int valueSem;
    int tornaMenu = 1;

    if (strlen(buffer.message) > 1)
    {
        strcpy(buffer.message, "Comando non riconosciuto.\n");
        sendMenu(connectSocket, buffer);
        return 0;
    }

    switch(*buffer.message)
    {
    case 'h':
        strcpy(buffer.message, "");
        break;
    case 'v':
        buffer = readContent(connectSocket, buffer);
        tornaMenu = 0;
        break;
    case 's':
        buffer = search(connectSocket, buffer,NULL);
        // maggiore della stringa "Non ci sono contatti salvati.\n" e "Nessun contatto con questo nome.\n"
        if (strlen(buffer.message) > 35)
            tornaMenu = 0;
        break;
    case 'l':
        buffer = login(connectSocket, buffer);
        break;
    case 'm':
        if (buffer.isAdmin){
            // controlla semaforo
            sem_getvalue(sem,&valueSem);

            // attesa se semaforo occupato
            if(valueSem == 0){
                strcpy(buffer.message,"Qualcun altro sta modificando i contatti. Vuoi attendere? [Y/N]\n");
                send(connectSocket,&buffer, sizeof(buffer), 0);
                
                if(recv(connectSocket,&buffer,sizeof(buffer), 0) < 0) {
                    printf("Errore nella ricezione dei dati.\n");
                    strcpy(buffer.message, "Errore nella ricezione dei dati.\n");
                    break;
                }
                if(strcmp(utils_lowercase(buffer.message), "y") != 0) {
                    strcpy(buffer.message, "");
                    break;
                }
            }
            // chiude semaforo
            sem_wait(sem);
            semPtr = &sem;
            criticalSection = 1;

            buffer = search(connectSocket,buffer,editFromList);

            // libera semaforo
            sem_post(sem);
            criticalSection = 0;
        }
        else
            strcpy(buffer.message, "Comando non riconosciuto.\n");
        break;
    case 'r':
        if (buffer.isAdmin == 1){
            // controlla semaforo
            sem_getvalue(sem,&valueSem);

            // attesa se semaforo occupato
            if(valueSem == 0){
                strcpy(buffer.message,"Qualcun altro sta modificando i contatti. Vuoi attendere? [Y/N]\n");
                send(connectSocket,&buffer, sizeof(buffer), 0);
                
                if(recv(connectSocket,&buffer,sizeof(buffer), 0) < 0) {
                    printf("Errore nella ricezione dei dati.\n");
                    strcpy(buffer.message, "Errore nella ricezione dei dati.\n");
                    break;
                }
                if(strcmp(utils_lowercase(buffer.message), "y") != 0) {
                    strcpy(buffer.message, "");
                    break;
                }
            }
            // chiude semaforo
            sem_wait(sem);
            semPtr = &sem;
            criticalSection = 1;

            buffer = search(connectSocket,buffer,removeFromList);

            // libera semaforo
            sem_post(sem);
            criticalSection = 0;
        }
        else
            strcpy(buffer.message, "Comando non riconosciuto.\n");
        break;
        
    case 'a':
        if (buffer.isAdmin == 1){
            // controlla semaforo
            sem_getvalue(sem,&valueSem);

            // attesa se semaforo occupato
            if(valueSem == 0){
                strcpy(buffer.message,"Qualcun altro sta modificando i contatti. Vuoi attendere? [Y/N]\n");
                send(connectSocket,&buffer, sizeof(buffer), 0);
                
                if(recv(connectSocket,&buffer,sizeof(buffer), 0) < 0) {
                    printf("Errore nella ricezione dei dati.\n");
                    strcpy(buffer.message, "Errore nella ricezione dei dati.\n");
                    break;
                }
                if(strcmp(utils_lowercase(buffer.message), "y") != 0) {
                    strcpy(buffer.message, "");
                    break;
                }
            }
            // chiude semaforo
            sem_wait(sem);
            semPtr = &sem;
            criticalSection = 1;

            int isAdded = aggiungiPersona(connectSocket, buffer);
            
            // libera semaforo
            sem_post(sem);
            criticalSection = 0;

            strcpy(buffer.message, (isAdded ==1 ? "Aggiunto contatto!\n" : "Contatto non aggiunto.\n"));
        } else
            strcpy(buffer.message, "Comando non riconosciuto.\n"); 
        break;

    default:
        strcpy(buffer.message, "Comando non riconosciuto.\n");
        break;
    }

    // torna direttamente al menu
    if(tornaMenu) {
        sendMenu(connectSocket, buffer);
        return 1;
    }
    // dopo aver risolto l'operazione richiesta chiede se si vuole tornare al menu
    strcat(buffer.message, "Vuoi tornare al menu? [Y/N]\n");
    send(connectSocket,&buffer, sizeof(buffer), 0);

    if(recv(connectSocket,&buffer,sizeof(buffer), 0) < 0 || strcmp(utils_lowercase(buffer.message), "y") != 0) {
        strcpy(buffer.message, "x");
        send(connectSocket, &buffer, sizeof(buffer), 0);
        
        close(connectSocket);
        printf("\nClient con pid %d disconnesso.\n",getpid());
        exit(1);
    }
    
    strcpy(buffer.message, "");
    sendMenu(connectSocket, buffer);
    return 1;
}

void customSigHandler(){
    close(serverSocket);
    if(ppidServerInit == getpid()){
        printf("\nProcesso padre(%d) terminato\n",ppidServerInit);
        exit(0);
    }
    
    printf("\nprocesso figlio con pid %d terminato\n",getpid());
    exit(1);
}

void customSigPipeHandler(int signo){  
    printf("\nClient con pid %d disconnesso.\n",getpid());
    if(criticalSection == 1) {
        printf("Il client era in sezione critica. Sblocco il semaforo.\n");
        sem_post(*semPtr); // se il processo era il sezione critica rilascia semaforo
    }
    exit(0);
}


void main(int argc, char const *argv[])
{
    void (*handler) (int);
    ppidServerInit = getpid();
    
    sem_t *sem = mmap(NULL, sizeof(sem_t), PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    sem_init(sem,1,1);

    int connectSocket, returnCode;
    socklen_t clientAddressLen;
    struct sockaddr_in serverAddress, clientAddress;
    char *clientIP;

    MSG buffer;
    buffer.isAdmin = 0;

    while(createSettings() != 1){
        printf("ricominciamo dal principio!\n");
    }

    if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("errore nella creazione del socket!");
        exit(serverSocket);
    }
    
    //modo per evitare l'errore "errore nel binding: Address already in use" (SO_REUSEADDR)
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    signal(SIGPIPE, customSigPipeHandler);
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

                    if(strcmp(buffer.message, "x") == 0) {
                    send(connectSocket, &buffer, sizeof(buffer), 0);
                    close(connectSocket);
                    printf("Client @ %s disconnesso.\n", clientIP);
                    // chiude il processo figlio
                    exit(1);
                }                    
                    // gestione delle richieste                
                    choiseHandler(connectSocket, buffer,sem);
                }

                fflush(stdout);
                fflush(stdin);
                
            }

        }   
        else 
            close(connectSocket); 
    }
    

}

int createSettings(){

    FILE *fp = fopen("password.txt", "r");
    
    t_credenziali admin;
    // char name[25];
    // char password[25];

    if (fp == NULL) {
        // se il database non esiste
        printf("impostazioni non trovate.\nCreazione del file impostazioni.\n");

        //problemi di gestione in caso di overflow
        do{
        printf("nome admin: ");
            scanf("%24s",admin.user);
        }while(strlen(admin.user)>24);
        clean_stdin();
        
        //per ora non fa lo sha --> openssl manca/libreria
        do{   
            printf("password (!)max 24 caratteri, sarà applicato uno sha(!):");
            scanf("%24s",admin.password);
        }while(strlen(admin.password) > 24);
        clean_stdin();
        
        printf("%s : %s conferma? [Y/N]", admin.user, admin.password);

        char scelta;
        scanf("%c", &scelta);

        printf("hai scelto: %c\n",scelta);
        switch(scelta){

            case 'Y':
            
            case 'y':
                //fclose(fp) NON mi fa chiudere in lettura prima di riaprire in scrittura...(FUNZIONA UGUALE)
                FILE *fpSettings = fopen("password.txt","w");
                if (fpSettings == NULL) {
                    printf("Errore nell'apertura del file.\n");
                    return 1;
                }
                printf("impostazioni salvate con successo!\n\n");
                char hash[CONVERTION_SHA256_MAX];
                inToSha256(admin.password,hash);
                fprintf(fpSettings,"%s %s",admin.user, hash);
                //fwrite(hash,sizeof(unsigned char),32,fpSettings);

                fflush(fpSettings);
                fclose(fpSettings);
                return 1;

            default:
                fclose(fp);
                printf("impostazioni non salvate.\n");
                return 0;
        }
    }
    fscanf(fp,"%s %s",cred.user,cred.password);
    //printf("letti %s %s\n",cred.user,cred.password);
    printf("impostazioni server caricate con successo!\n");
    fclose(fp);
    return 1;
}