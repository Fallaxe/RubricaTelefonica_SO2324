/*
    -> ascolto connessioni
    -> messaggio di benvenuto
    -> la gestione delle varie connessioni è gestita da una fork()
    -> comunicazione bidirezionale?
*/
// cambia il prototipo di alcune cose per testarlo!
#include "server.h"

void hashToHexString(const unsigned char *hash, int length, char *output) {
    const char *hexChars = "0123456789abcdef";
    for (int i = 0; i < length; i++) {
        output[i * 2] = hexChars[(hash[i] >> 4) & 0xF];
        output[i * 2 + 1] = hexChars[hash[i] & 0xF];
    }
    output[length * 2] = '\0'; // determinare la fine della stringa
}
void handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}
void inToSha256(char *inToHash)
{
    unsigned char mid[EVP_MAX_MD_SIZE];
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if(mdctx == NULL) handleErrors();

    if(1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
        handleErrors();

    if(1 != EVP_DigestUpdate(mdctx, inToHash, strlen(inToHash)))
        handleErrors();

    unsigned int len = 0;
    if(1 != EVP_DigestFinal_ex(mdctx, mid, &len))
        handleErrors();

    EVP_MD_CTX_free(mdctx);


    hashToHexString(mid,len,inToHash);

}

void clean_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void removeFromList(cJSON *found, cJSON* list,int connectSocket, MSG buffer)
{
    strcat(buffer.message,"Inserisci l'indice del contatto da eliminare. x per tornare al menù\n");
    send(connectSocket,&buffer, sizeof(buffer), 0);

    int num = -1;
    int nContacts = cJSON_GetArraySize(found);
    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0)
        printf("Errore nella ricezione dei dati.\n");
    else {
        //controllo atoi
        num = atoi(buffer.message);
    }

    if (num > 0 && num <= nContacts){
        // trova elemento corrispondente nell'elenco contatti
        int index = 0;
        cJSON * elementDeleted = cJSON_GetArrayItem(found, (num-1));
        cJSON * element = list->child;
        while(element){
            if(strcmp(cJSON_Print(elementDeleted), cJSON_Print(element)) == 0) {
                break;
            }
            index ++;
            element= element->next;
        }
        printf("Client - User: eliminazione di %s\n", cJSON_Print(element));
        // non so se vogliamo aggiungere un controllo. Del tipo: "Vuoi veramente eliminare <nome contatto>?"
        
        //cancella dall'array all'indirizzo selezionato
        cJSON_DeleteItemFromArray(list,index);
        saveDatabase(list);
    }
}

void editFromList(cJSON *found, cJSON *list, int connectSocket, MSG buffer)
{
    strcat(buffer.message,"mandami l'indice del contatto da modificare");
    send(connectSocket,&buffer, sizeof(buffer), 0);

    int num = -1;
    int nContacts = cJSON_GetArraySize(found);
    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0)
        printf("Errore nella ricezione dei dati.\n");
    else {
        //controllo atoi
        num = atoi(buffer.message);
    }

    if (num > 0 && num <= nContacts){
        // trova elemento corrispondente nell'elenco contatti
        int index = 0;
        cJSON * elementDeleted = cJSON_GetArrayItem(found, (num-1));
        cJSON * element = list->child;
        while(element){
            if(strcmp(cJSON_Print(elementDeleted), cJSON_Print(element)) == 0) {
            break;
        }
        index ++;
        element= element->next;
        }

        strcpy(buffer.message, "");
        sprintf(buffer.message,"contatto: \n%s\n%s\n%d\n%s\n%s\nmodifichiamolo?[y/N]\n",cJSON_GetObjectItem(element,"name")->valuestring,cJSON_GetObjectItem(element,"surname")->valuestring,cJSON_GetObjectItem(element,"age")->valueint,cJSON_GetObjectItem(element,"email")->valuestring,cJSON_GetObjectItem(element,"phone")->valuestring);
        send(connectSocket,&buffer, sizeof(buffer), 0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0)
            printf("Errore nella ricezione dei dati.\n");
        else {
            if(strcmp((buffer.message),"y") == 0){
                //edit
                printf("Client - User: modifica di %s\n", cJSON_Print(element));
                cJSON* nuovaPersona = creaPersona(connectSocket);
                printf("Client - User: modificato con %s\n", cJSON_Print(nuovaPersona));
                cJSON_ReplaceItemInArray(list,index,nuovaPersona);
                saveDatabase(list);
            }
        }
    }
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

cJSON *creaPersona(int connectSocket)
{
    MSG buffer;
    cJSON *jsonItem = cJSON_CreateObject();

    strcpy(buffer.message, "Nuovo Contatto\nInserire nome del contatto :\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
            return NULL;
        } else {
            //strcpy(persona.name, buffer.message);
            if(strlen(buffer.message) > 12){
                printf("l'utente ha superato i limiti imposti\n");
                return NULL;
            }
            cJSON_AddStringToObject(jsonItem, "name", buffer.message);
            printf("Client - New contact: %s\n", buffer.message);
        }

    strcpy(buffer.message, "Inserire cognome del contatto :\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
            return NULL;
        } else {
            if(strlen(buffer.message) > 12){
                printf("l'utente ha superato i limiti imposti\n");
                return NULL;
            }
            cJSON_AddStringToObject(jsonItem, "surname", buffer.message);
            printf("Client - New contact: %s\n", buffer.message);
        }

        strcpy(buffer.message, "Inserire eta del contatto :\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
            return NULL;
        } else {
            
            if(atoi(buffer.message) > 100 || atoi(buffer.message) < 1){
                printf("l'utente ha superato i limiti imposti\n");
                return NULL;
            }

            cJSON_AddNumberToObject(jsonItem, "age", atoi(buffer.message));
            printf("Client - New contact: %s\n", buffer.message);
        }

        strcpy(buffer.message, "Inserire email del contatto :\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
            return NULL;
        } else {
            
            if(strlen(buffer.message) > 30){
                printf("l'utente ha superato i limiti imposti\n");
                return NULL;
            }
            cJSON_AddStringToObject(jsonItem, "email", buffer.message);
            printf("Client - New contact: %s\n", buffer.message);
        }

        strcpy(buffer.message, "Inserire telefono del contatto :\t");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
            return NULL;
        } else {
            if(strlen(buffer.message) > 16 || utils_strIncludeOnly(buffer.message, "+ 1234567890") == 0){
                printf("l'utente ha superato i limiti imposti\n");
                return NULL;
            }
            cJSON_AddStringToObject(jsonItem, "phone", buffer.message);
            printf("Client - New contact: %s\n", buffer.message);
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
                strcat(buffer.message, menuHeader);
                strcat(buffer.message, divisore);
                snprintf(elementString, sizeof(elementString), "%d contatti trovati. Pagina %d di %d.\n", nContacts, nPage, ((nContacts+(contactsInPage-1))/contactsInPage)); 
                strcat(buffer.message, elementString);
            }

            // Stringa Contatto
            snprintf(elementString, sizeof(elementString), "%d) nome: %s\ncognome: %s\netà: %d\nemail: %s\ntelefono: %s\n\n",i,cJSON_GetObjectItem(element,"name")->valuestring,cJSON_GetObjectItem(element,"surname")->valuestring,cJSON_GetObjectItem(element,"age")->valueint,cJSON_GetObjectItem(element,"email")->valuestring,cJSON_GetObjectItem(element,"phone")->valuestring); 
            printf("dimensioni elemento: %ld\n", strlen(elementString));
            strcat(buffer.message, elementString);

            // Fine pagina
            if (i < nContacts && i%contactsInPage == 0)
            {
                strcat(buffer.message, "Vuoi vedere la pagina successiva? Y/N\n");
                send(connectSocket,&buffer, sizeof(buffer),0);

                if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0)
                    printf("Errore nella ricezione dei dati.\n");
                else {
                    strcpy(cred.user, buffer.message);
                    printf("Client - User: %s\n", buffer.message);
                }

                if (strcmp(utils_lowercase(buffer.message), "y") == 0) {
                    strcpy(buffer.message, "");
                } else {
                    strcpy(buffer.message, "");
                    break;
                }
            }

            element = element->next;
            i++;
        }
    }
    return buffer;
}

void readContent(int connectSocket, MSG buffer)
{
    cJSON *jsonArray = loadDatabase();
    strcpy(buffer.message, "");
    buffer = printContent(jsonArray,connectSocket,buffer);
    sendMenu(connectSocket, buffer);
}

MSG search(int connectSocket, MSG buffer, operationOnList op)
{   
    cJSON *jsonArray = loadDatabase();
    cJSON *foundArr = cJSON_CreateArray();
    char searchName[25];
    
    if (jsonArray == NULL || cJSON_GetArraySize(jsonArray) == 0)
    {
        strcpy(buffer.message, "");
    } else
    {
        strcpy(buffer.message, "Inserire nome della persona da cercare:\n");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
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
      
        if(cJSON_GetArraySize(foundArr) > 0)
        {
            strcpy(buffer.message, "");
            buffer = printContent(foundArr,connectSocket,buffer); //diventata una op?
            
            if(op != NULL)
                op(foundArr,jsonArray,connectSocket, buffer);
        } else
        {
            strcpy(buffer.message, "Nessun contatto trovato.\n");
        }
    }
    return buffer;
}

int verifica(t_credenziali cred_inserita)
{
    FILE * fptr;
    //t_credenziali admin;
    int login = 0;

    inToSha256(cred_inserita.password);

    if ((strcmp(cred.user, cred_inserita.user) == 0) && (strcmp(cred.password, cred_inserita.password) == 0)) {
        login = 1;
        printf("Login effettuato.\n");                     
    }

    if (login == 0) printf("Login non effettuato %d.\n",login);
    fclose(fptr);
    return login;
}

void login(int connectSocket, MSG buffer){
        if(buffer.isAdmin == 1) {
            strcpy(buffer.message, "Sei già loggato.\n");
            sendMenu(connectSocket, buffer);
            return;
        }
        t_credenziali cred_inserite;
        
        // Richiede User
        strcpy(buffer.message, "Inserire user:      ");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
            strcpy(cred_inserite.user, buffer.message);
            printf("Client - User: %s\n", buffer.message);
        }

        // Richiede Password
        strcpy(buffer.message, "Inserire password:  ");
        send(connectSocket,&buffer, sizeof(buffer),0);

        if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
            printf("Errore nella ricezione dei dati.\n");
        } else {
            strcpy(cred_inserite.password, buffer.message);
            printf("Client - Password: %s\n", buffer.message);
        }

        if ( verifica(cred_inserite) == 1) {
            buffer.isAdmin = 1;
            strcpy(buffer.message, "Login effettuato con successo.\n\n");

        } else {
            strcpy(buffer.message, "Attenzione username o password errati.\n\n");
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
        
        cJSON *jsonArray = loadDatabase();

        if (jsonArray == NULL) {
            printf("Array vuoto\n");
            jsonArray = cJSON_CreateArray();
        }

        cJSON* jsonItem = creaPersona(connectSocket);
        if(jsonItem != NULL)
        {
            cJSON_AddItemToArray(jsonArray,jsonItem);
            printf("%s\n", cJSON_Print(jsonItem)); //stampa solo la persona appena inserita, non tutto l'array
            saveDatabase(jsonArray);
            cJSON_Delete(jsonItem);
            return 1;
        }
        cJSON_Delete(jsonItem);
        return 0;
}

int choiseHandler(int connectSocket, MSG choise,sem_t *sem)
{
    MSG buffer;
    buffer.isAdmin = choise.isAdmin;

    printf("dimensioni risposta: %ld\n", strlen(choise.message));

    if (strlen(choise.message) > 1)
    {
        strcpy(buffer.message, "Comando non riconosciuto.\n");
        sendMenu(connectSocket, buffer);
        return 0;
    }

    switch(*choise.message)
    {
    case 'h':
        strcpy(buffer.message, "");
        sendMenu(connectSocket, buffer);
        break;
    case 'v':
        readContent(connectSocket, buffer);
        break;
    case 's':
        buffer = search(connectSocket, buffer,NULL);
        if (strlen(buffer.message) == 0)
                strcpy(buffer.message, "Non ci sono contatti salvati.\n");
        sendMenu(connectSocket, buffer);
        break;
    case 'l':
        login(connectSocket, buffer);
        break;
    case 'm':
        if (choise.isAdmin == 1){
            sem_wait(sem);
            buffer = search(connectSocket,buffer,editFromList);
            if (strlen(buffer.message) == 0)
                strcpy(buffer.message, "Non ci sono contatti salvati.\n");
            sem_post(sem);
        }
        else
            strcpy(buffer.message, "Comando non riconosciuto.\n");

        sendMenu(connectSocket, buffer);
        break;
    case 'r':
        if (choise.isAdmin == 1){
            sem_wait(sem);
            buffer = search(connectSocket,buffer,removeFromList);
            if (strlen(buffer.message) == 0)
                strcpy(buffer.message, "Non ci sono contatti salvati.\n");
            sem_post(sem);
        }
        else
            strcpy(buffer.message, "Comando non riconosciuto.\n");

        sendMenu(connectSocket, buffer);
        break;
    case 'x':
        break;
    case 'a':

        if (choise.isAdmin == 1){

            //GESTIONE DI INFORMAZIONE CHE SEI IN CODA PER LA RICHIESTA (NON FUNZIONA E NON SO PERCHE)
            // int valueSem;
            // int returnCode;
            // sem_getvalue(sem,&valueSem);
            // printf("%d\n",valueSem);
            // if(&valueSem == 0){//?
            //     strcpy(buffer.message,"sei in coda per l'accesso alla richiesta\n!");
            //     send(connectSocket,&buffer, sizeof(buffer), 0);

            //     // if((returnCode=recv(connectSocket, &buffer, sizeof(buffer), 0)) < 0) {
            //     //         printf("Errore nella ricezione dei dati.\n");
            //     //     }
            //     strcpy(buffer.message,"");
            // }
            sem_wait(sem); //lock semaphore
            //"problema" se nell'attesa scrivo viene messo nel buffer del nome
            int isAdded = aggiungiPersona(connectSocket, buffer);
            strcpy(buffer.message, (isAdded ==1 ? 
            "aggiunto contatto!\n" :
            "errore nell'aggiunta:\nil nome deve essere minore di 12 \nl'età minore di 100 e maggiore di 0\ne la mail lunga al massimo 25 caratteri\n")); //pulisce buffer
            sem_post(sem); // unlock semaphore
        } else {
            strcpy(buffer.message, "Comando non riconosciuto.\n");
        }
        sendMenu(connectSocket, buffer);       
        break;
    default:
        strcpy(buffer.message, "Comando non riconosciuto.\n");
        sendMenu(connectSocket, buffer);
        break;
    }
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
                    choiseHandler(connectSocket, buffer,sem);
                }
                
                if(strcmp(buffer.message, "x") == 0) {
                    send(connectSocket, &buffer, sizeof(buffer), 0);
                    close(connectSocket);
                    printf("Client @ %s disconnesso.\n", clientIP);
                    // chiude il processo figlio
                    exit(1);
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
    

    if (fp == NULL) {
        // se il database non esiste
        printf("impostazioni non trovate.\nCreazione del file impostazioni.\n");
        printf("*Creazione dei parametri di login*\n");
        printf("-> i parametri devono contenere un massimo di 25 caratteri, i restanti saranno ignorati!\n\n");
        printf("nome admin: ");
        scanf("%24s",cred.user);
        clean_stdin();
        
        printf("password (!)sarà applicato uno sha(!):");
        scanf("%24s",cred.password);
        clean_stdin();
        

        printf("%s : %s conferma? [y/N]", cred.user, cred.password);

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

                inToSha256(cred.password);
                fprintf(fpSettings,"%s %s",cred.user, cred.password);

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