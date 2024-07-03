/*
    7109803 Miranda Bellezza
    7112588 Daniele Fallaci
*/

#include "server.h"

// gestione segnali
static void customSigHandler(){
    close(serverSocket);
    if(ppidServerInit == getpid()){
        printf("\nProcesso padre(%d) terminato\n",ppidServerInit);
        exit(0);
    }
    
    printf("\nprocesso figlio con pid %d terminato\n",getpid());
    exit(0);
}

static void customSigPipeHandler(int signo){  
    printf("\nClient con pid %d disconnesso.\n",getpid());
    
    // se il processo era il sezione critica
    if(inCriticalSection == getpid()) {
        sem_post(*semPtr);
    }
    exit(0);
}

// Gestione argomenti per reset admin
static int parser(char const *argomenti[], int max) {

    int returnValue=0;

    // ricerca flag -r
    if(max > 1){

        if(strcmp(argomenti[1], resetArg) == 0){
            printf("Rilevato flag -r per il reset delle impostazioni.\n");
            returnValue = 1;
        }

        // troppi parametri:
        if(max>2 || returnValue == 0){
            printf("Parametri errati!\nL'utilizzo del flag -r può essere usato per il reset delle impostazioni.");
            exit(0);
        }
    }

    return returnValue;
}

static int createSettings(char const *argomenti[],int max){

    FILE *fp = fopen(FILE_USERS, "r");
    
    t_credenziali admin;

    if (fp == NULL || parser(argomenti,max) == 1) {
        if(fp == NULL)
            printf("Impostazioni non trovate\nCreazione del file impostazioni.\n");
        else {
            fclose(fp);
            printf("Ricevuta richiesta di reset del file impostazioni.\n");
        }

        //Richiesta nome/password e gestione in caso di overflow
        do{
        printf("nome admin: ");
            scanf("%24s",admin.user);
        }while(strlen(admin.user)>24);
        clean_stdin();
        
        do{   
            printf("password (!)max 24 caratteri, sarà applicato uno sha(!):");
            scanf("%24s",admin.password);
        }while(strlen(admin.password) > 24);
        clean_stdin();
        
        printf("%s : %s conferma? [y/N]", admin.user, admin.password);

        char scelta = getchar();
        clean_stdin();
        printf("hai scelto: %c\n",scelta);
        
        switch(scelta){
            case 'Y':
            
            case 'y':
                FILE *fpSettings = fopen(FILE_USERS,"w");
                if (fpSettings == NULL) {
                    printf("Errore nell'apertura del file.\n");
                    return 1;
                }
                printf("Impostazioni salvate con successo!\n\n");
                char hash[CONVERTION_SHA256_MAX];
                inToSha256(admin.password,hash);
                fprintf(fpSettings,"%s %s",admin.user, hash);

                fflush(fpSettings);
                fclose(fpSettings);
                return 1;

            default:
                printf("Impostazioni non salvate.\n");
                return 0;
        }
    }
    fscanf(fp,"%s %s",cred.user,cred.password);
    printf("Impostazioni server caricate con successo!\n");
    fclose(fp);
    return 1;
}

static cJSON * loadDatabase()
{
    cJSON *jsonArray;

    // apre file data in lettura
    FILE *fp = fopen(FILE_DB, "r"); 
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

static void saveDatabase(cJSON * jsonArray){
    // sort dell'array per cognome
        utils_sortByKey(jsonArray, "surname");
    // creazione di una stringa in formato json con tutti gli oggetti
        char *json_str = cJSON_Print(jsonArray);

     // salva la stringa JSON sul file database  
        FILE * fp = fopen(FILE_DB, "w"); 
        if (fp == NULL) { 
            printf("impossibile aprire il file dei dati\n"); 
            exit(-1);
        }
        fputs(json_str, fp);

        fflush(fp);
        fclose(fp);
        // free the JSON string and cJSON object 
        cJSON_free(json_str);
}

static int verifica(t_credenziali cred)
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
                break;                         
            }
        }
    }
    fclose(fptr);
    return login;
}

static MSG login(int connectSocket, char *clientIP, MSG buffer){
    if(buffer.isAdmin == 1) {
        strcpy(buffer.message, "Sei già loggato.\n");
        return buffer;
    }
    t_credenziali cred;
        
    // Richiede User
    strcpy(buffer.message, "Inserire user:      ");
    send(connectSocket,&buffer, sizeof(buffer),0);

    strcpy(buffer.message, "");
    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        exit(-1);
    } else {
        strcpy(cred.user, buffer.message);
        printf("Client @ %s : %d - User: %s\n", clientIP, connectSocket, buffer.message);
    }

    // Richiede Password
    strcpy(buffer.message, "Inserire password:  ");
    send(connectSocket,&buffer, sizeof(buffer),0);

    strcpy(buffer.message, "");
    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        exit(-1);
    } else {
        strcpy(cred.password, buffer.message);
        printf("Client @ %s : %d - Password: %s\n", clientIP, connectSocket, buffer.message);
    }

    // verifica user e password
    if (verifica(cred)) {
        buffer.isAdmin = 1;
        strcpy(buffer.message, "Login effettuato con successo.\n\n");
        printf("Client @ %s : %d - Login effettuato.\n", clientIP, connectSocket);
    } else {
        strcpy(buffer.message, "Attenzione username o password errati.\n\n");
        printf("Client @ %s : %d - Username o password errati.\n", clientIP, connectSocket);
    }

    return buffer;
}

static void sendMenu(int connectSocket, MSG buffer)
{
    // banner
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

static cJSON *creaPersona(int connectSocket, MSG buffer)
{
    cJSON *jsonItem = cJSON_CreateObject();
    
    // banner
    strcpy(buffer.message, divisore);
    strcat(buffer.message, contattoHeader);
    strcat(buffer.message, divisore);

    // creazione del contatto
    strcat(buffer.message, "Inserire nome del contatto :\t");
    send(connectSocket,&buffer, sizeof(buffer),0);

    strcpy(buffer.message, "");
    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return NULL;
    } 
    else {
        while(strlen(buffer.message) < 1 || strlen(buffer.message) > 12) {
            strcpy(buffer.message, "Attenzione, non può essere lasciato vuoto e deve essere massimo 12 caratteri.\nInserire nome del contatto :\t");
            send(connectSocket,&buffer, sizeof(buffer),0);

            strcpy(buffer.message, "");
            if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
                printf("Errore nella ricezione dei dati.\n");
                return NULL;
            } 
        }
        cJSON_AddStringToObject(jsonItem, "name", buffer.message);
    }

    strcpy(buffer.message, "Inserire cognome del contatto :\t");
    send(connectSocket,&buffer, sizeof(buffer),0);

    strcpy(buffer.message, "");
    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return NULL;
    } 
    else {
        while(strlen(buffer.message) > 12) {
            strcpy(buffer.message, "Attenzione, massimo 12 caratteri.\nInserire cognome del contatto :\t");
            send(connectSocket,&buffer, sizeof(buffer),0);

            strcpy(buffer.message, "");
            if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
                printf("Errore nella ricezione dei dati.\n");
                return NULL;
            } 
        }
        cJSON_AddStringToObject(jsonItem, "surname", buffer.message);
    }

    strcpy(buffer.message, "Inserire eta del contatto :\t");
    send(connectSocket,&buffer, sizeof(buffer),0);

    strcpy(buffer.message, "");
    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return NULL;
     } 
    else {
        while(atoi(buffer.message) > 100 || atoi(buffer.message) < 1){
            strcpy(buffer.message, "Attenzione, deve essere un numero tra 1 e 100.\nInserire eta del contatto :\t");
            send(connectSocket,&buffer, sizeof(buffer),0);

            strcpy(buffer.message, "");
            if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
                printf("Errore nella ricezione dei dati.\n");
                return NULL;
            } 
        }
        cJSON_AddNumberToObject(jsonItem, "age", atoi(buffer.message));
    }

    strcpy(buffer.message, "Inserire email del contatto :\t");
    send(connectSocket,&buffer, sizeof(buffer),0);

    strcpy(buffer.message, "");
    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return NULL;
    } 
    else {
        while(strcmp(buffer.message, "") != 0 && (strlen(buffer.message) > 30 || !utils_isValidEmail(buffer.message))){
            strcpy(buffer.message, "Attenzione, deve avere massimo 30 caratteri ed essere un'email valida.\nInserire email del contatto :\t");
            send(connectSocket,&buffer, sizeof(buffer),0);

            strcpy(buffer.message, "");
            if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
                printf("Errore nella ricezione dei dati.\n");
                return NULL;
            } 
        }
        cJSON_AddStringToObject(jsonItem, "email", buffer.message);
    }

    strcpy(buffer.message, "Inserire telefono del contatto :\t");
    send(connectSocket,&buffer, sizeof(buffer),0);

    strcpy(buffer.message, "");
    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return NULL;
    } 
    else {
        while(strlen(buffer.message) > 16 || utils_strIncludeOnly(buffer.message, " 1234567890") == 0){
            strcpy(buffer.message, "Attenzione, massimo 16 caratteri e caratteri consentiti: ' 1234567890'.\nInserire telefono del contatto :\t");
            send(connectSocket,&buffer, sizeof(buffer),0);

            strcpy(buffer.message, "");
            if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
                printf("Errore nella ricezione dei dati.\n");
                return NULL;
            } 
        }
         cJSON_AddStringToObject(jsonItem, "phone", buffer.message);
    }

    char elementStr[1024];
    snprintf(elementStr, sizeof(elementStr), "Aggiungo:\n\tNome: %s\n\tCognome: %s\n\tEtà: %d\n\tEmail: %s\n\tTelefono: %s\nVuoi confermare? [y/N]\n",
                                            cJSON_GetObjectItem(jsonItem,"name")->valuestring, cJSON_GetObjectItem(jsonItem,"surname")->valuestring,
                                            cJSON_GetObjectItem(jsonItem,"age")->valueint, cJSON_GetObjectItem(jsonItem,"email")->valuestring,
                                            cJSON_GetObjectItem(jsonItem,"phone")->valuestring); 
    strcpy(buffer.message, elementStr);
    send(connectSocket,&buffer, sizeof(buffer), 0);

    strcpy(buffer.message, "");
    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Errore nella ricezione dei dati.\n");
        return NULL;
    } else if(strcmp(utils_lowercase(buffer.message), "y") != 0) {
        return NULL;
    }
    return jsonItem;
}

static MSG printContent(cJSON * array, int connectSocket, char*clientIP,MSG buffer)
{
    char elementString[1024];
    int nContacts = 0;
    int nPage = 1;
    
    if (array == NULL || cJSON_GetArraySize(array) == 0)
        strcat(buffer.message, "Non ci sono contatti salvati.\n");
    else
    {  
        nContacts = cJSON_GetArraySize(array);

        cJSON *element = array->child;
        int i = 1;


        while (element)
        {   
            // Inizio pagina
            if(i%(RECORDS_INPAGE) == 1) {
                strcpy(buffer.message, divisore);
                strcat(buffer.message, cercaHeader);
                strcat(buffer.message, divisore);
                snprintf(elementString, sizeof(elementString), "%d contatti trovati. Pagina %d di %d.\n", nContacts, nPage, ((nContacts+(RECORDS_INPAGE-1))/RECORDS_INPAGE)); 
                strcat(buffer.message, elementString);
            }

            // Stringa Contatto
            snprintf(elementString, sizeof(elementString), "%d) Nome: %s\n   Cognome: %s\n   Età: %d\n   Email: %s\n   Telefono: %s\n\n",i,cJSON_GetObjectItem(element,"name")->valuestring,cJSON_GetObjectItem(element,"surname")->valuestring,cJSON_GetObjectItem(element,"age")->valueint,cJSON_GetObjectItem(element,"email")->valuestring,cJSON_GetObjectItem(element,"phone")->valuestring); 
            strcat(buffer.message, elementString);

            // Fine pagina
            if (i < nContacts && i%RECORDS_INPAGE == 0)
            {
                strcat(buffer.message, "Vuoi vedere la pagina successiva? [y/N]\n");
                send(connectSocket,&buffer, sizeof(buffer),0);
                
                strcpy(buffer.message, "");
                if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
                    printf("Errore nella ricezione dei dati.\n");
                    close(connectSocket);
                    exit(-1);
                }

                printf("Client @ %s : %d - User: %s\n", clientIP, connectSocket, buffer.message);

                if (strcmp(utils_lowercase(buffer.message), "y") != 0) {
                    strcpy(buffer.message, "");
                    break;
                }
                nPage++;
                strcpy(buffer.message, "");
            }

            element = element->next;
            i++;
        }
    }
    return buffer;
}

static int removeFromList(cJSON *found, cJSON* list,int connectSocket, char *clientIP, MSG buffer)
{
    int num = -1;
    int nContacts = cJSON_GetArraySize(found);

    do
    {
        strcat(buffer.message,"Inserisci l'indice del contatto da eliminare. x per tornare al menù\n");
        send(connectSocket,&buffer, sizeof(buffer), 0);

        strcpy(buffer.message, "");
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
    snprintf(elementStr, sizeof(elementStr), "Contatto:\n\tNome: %s\n\tCognome: %s\n\tEtà: %d\n\tEmail: %s\n\tTelefono: %s\nATTENZIONE. Vuoi confermare la rimozione? [y/N]\n",
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

    printf("Client @ %s : %d - eliminazione di %s\n", clientIP, connectSocket,cJSON_Print(element));
        
    //cancella dall'array all'indice selezionato
    cJSON_DeleteItemFromArray(list,index);
    saveDatabase(list);
    return 1;
}

static int editFromList(cJSON *found, cJSON *list, int connectSocket, char *clientIP, MSG buffer)
{
    int num = -1;
    int nContacts = cJSON_GetArraySize(found);

    do
    {
        strcat(buffer.message,"Inserisci l'indice del contatto da modificare. x per tornare al menù\n");
        send(connectSocket,&buffer, sizeof(buffer), 0);

        strcpy(buffer.message, "");
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
    snprintf(elementStr, sizeof(elementStr), "Contatto:\n\tNome: %s\n\tCognome: %s\n\tEtà: %d\n\tEmail: %s\n\tTelefono: %s\nATTENZIONE. Vuoi modificarlo? [y/N]\n",
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
    printf("Client @ %s : %d - modifica di %s\n", clientIP, connectSocket,cJSON_Print(element));
    cJSON* nuovaPersona = creaPersona(connectSocket, buffer);

    if(nuovaPersona != NULL)
    {
        printf("Client @ %s : %d - modificato con %s\n", clientIP, connectSocket,cJSON_Print(nuovaPersona));
        cJSON_ReplaceItemInArray(list,index,nuovaPersona);
        saveDatabase(list);

        cJSON_Delete(nuovaPersona);
        return 1;
    }
    printf("Client @ %s : %d - modifica annullata.\n", clientIP, connectSocket);
    cJSON_Delete(nuovaPersona);
    return 0;   
}

static MSG readContent(int connectSocket, char*clientIP,MSG buffer)
{
    cJSON *jsonArray = loadDatabase();
    strcpy(buffer.message, "");
    return printContent(jsonArray,connectSocket,clientIP,buffer);
}

static MSG search(int connectSocket, char*clientIP,MSG buffer, operationOnList op)
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

    strcpy(buffer.message, "");
    if((recv(connectSocket,&buffer,sizeof(buffer), 0)) < 0) {
        printf("Client @ %s : %d - Errore nella ricezione dei dati.\nClient @ %s : %d disconnesso.\n",clientIP,connectSocket,clientIP,connectSocket);
        close(connectSocket);
        exit(-1);
    } else {
        strcpy(searchName, buffer.message);
        printf("Client @ %s : %d - Ricerca: %s\n", clientIP, connectSocket, buffer.message);
    }

    cJSON *element = jsonArray->child;
    char nameSurname[25];
    char surnameName[25];

    // cerca se il nome inserito è sottostringa di "nome cognome" o "cognome nome"
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
        buffer = printContent(foundArr,connectSocket,clientIP,buffer);
            
        if(op != NULL)
            strcpy(buffer.message, (op(foundArr,jsonArray,connectSocket,clientIP, buffer)? "Modifica dei contatti effettuata.\n" : "Nessuna modifica effettuata\n"));
    }
    else
        strcpy(buffer.message, "Nessun contatto con questo nome.\n");

    return buffer;
}

static int aggiungiPersona(int connectSocket, char *clientIP, MSG buffer){        
        cJSON *jsonArray = loadDatabase();

        if (jsonArray == NULL) {
            printf("Non sono presenti contatti. Creazione nuovo database.\n");
            jsonArray = cJSON_CreateArray();
        }

        if(cJSON_GetArraySize(jsonArray) > RECORDS_MAX)
            return -1;

        cJSON* jsonItem = creaPersona(connectSocket, buffer);
        if(jsonItem != NULL)
        {
            cJSON_AddItemToArray(jsonArray,jsonItem);
            printf("Client @ %s : %d ha inserito:\n%s\n", clientIP,connectSocket,cJSON_Print(jsonItem)); //stampa solo la persona appena inserita
            saveDatabase(jsonArray);
            cJSON_Delete(jsonItem);
            return 1;
        }
        cJSON_Delete(jsonItem);
        return 0;
}

static int choiseHandler(int connectSocket, char*clientIP, MSG buffer,sem_t *sem)
{
    int valueSem;

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
        buffer = readContent(connectSocket, clientIP,buffer);
        break;
    case 's':
        buffer = search(connectSocket,clientIP, buffer,NULL);
        break;
    case 'l':
        buffer = login(connectSocket, clientIP, buffer);
        break;
    case 'm':
        if (buffer.isAdmin){
            // controlla semaforo
            sem_getvalue(sem,&valueSem);

            // attesa se semaforo occupato
            if(valueSem == 0){
                strcpy(buffer.message,"Qualcun altro sta modificando i contatti. Vuoi attendere? [y/N]\n");
                send(connectSocket,&buffer, sizeof(buffer), 0);\
                
                strcpy(buffer.message, "");
                if(recv(connectSocket,&buffer,sizeof(buffer), 0) < 0) {
                    printf("Errore nella ricezione dei dati.\n");
                    strcpy(buffer.message, "Errore nella ricezione dei dati.\n");
                    break;
                }
                if(strcmp(utils_lowercase(buffer.message), "y") != 0) {
                    strcpy(buffer.message, "");
                    break;
                }
                printf("Client @ %s : %d in attesa per la modifica dei contatti.\n", clientIP, connectSocket);
            }
            // chiude semaforo
            sem_wait(sem);
            semPtr = &sem;

            inCriticalSection = getpid();
            printf("Client @ %s : %d sta modificando contatti.\n", clientIP, connectSocket);

            buffer = search(connectSocket,clientIP,buffer,editFromList);

            // libera semaforo
            sem_post(sem);
            inCriticalSection = 0;
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
                strcpy(buffer.message,"Qualcun altro sta modificando i contatti. Vuoi attendere? [y/N]\n");
                send(connectSocket,&buffer, sizeof(buffer), 0);
                
                strcpy(buffer.message, "");
                if(recv(connectSocket,&buffer,sizeof(buffer), 0) < 0) {
                    printf("Errore nella ricezione dei dati.\n");
                    strcpy(buffer.message, "Errore nella ricezione dei dati.\n");
                    break;
                }
                if(strcmp(utils_lowercase(buffer.message), "y") != 0) {
                    strcpy(buffer.message, "");
                    break;
                }
                printf("Client @ %s : %d in attesa per la rimozione di contatti.\n", clientIP, connectSocket);
            }
            // chiude semaforo
            sem_wait(sem);
            semPtr = &sem;

            inCriticalSection = getpid();
            printf("Client @ %s : %d sta rimuovendo contatti.\n", clientIP, connectSocket);

            buffer = search(connectSocket,clientIP,buffer,removeFromList);

            // libera semaforo
            sem_post(sem);
            inCriticalSection = 0;
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
                strcpy(buffer.message,"Qualcun altro sta modificando i contatti. Vuoi attendere? [y/N]\n");
                send(connectSocket,&buffer, sizeof(buffer), 0);
                
                strcpy(buffer.message, "");
                if(recv(connectSocket,&buffer,sizeof(buffer), 0) < 0) {
                    printf("Errore nella ricezione dei dati.\n");
                    strcpy(buffer.message, "Errore nella ricezione dei dati.\n");
                    break;
                }
                if(strcmp(utils_lowercase(buffer.message), "y") != 0) {
                    strcpy(buffer.message, "");
                    break;
                }
                printf("Client @ %s : %d in attesa per l'aggiunta di contatti.\n", clientIP, connectSocket);
            }
            // chiude semaforo
            sem_wait(sem);
            
            semPtr = &sem;
            inCriticalSection = getpid();
            printf("Client @ %s : %d sta aggiungendo contatti.\n", clientIP, connectSocket);

            int isAdded = aggiungiPersona(connectSocket, clientIP, buffer);
            
            // libera semaforo
            sem_post(sem);
            inCriticalSection = 0;

            switch(isAdded)
            {
                case -1:
                    strcpy(buffer.message, "Raggiunto limite contatti.\n");
                    break;
                case 0:
                    strcpy(buffer.message, "Contatto non aggiunto.\n");
                    break;
                default:
                    strcpy(buffer.message, "Aggiunto contatto!\n");
                    break;
            }
        } else
            strcpy(buffer.message, "Comando non riconosciuto.\n"); 
        break;

    default:
        strcpy(buffer.message, "Comando non riconosciuto.\n");
        break;
    }

    // torna direttamente al menù
    if(strlen(buffer.message) < 42) {  // se il buffer contiene una stringa singola es.("Attenzione username o password errati.") torna direttamente al menu.
        sendMenu(connectSocket, buffer);
        return 1;
    }

    // dopo aver eserguito correttamente l'operazione di visita o ricerca chiede se si vuole tornare al menu
    strcat(buffer.message, "Vuoi tornare al menu? [y/N]\n");
    send(connectSocket,&buffer, sizeof(buffer), 0);

    strcpy(buffer.message, "");
    if(recv(connectSocket,&buffer,sizeof(buffer), 0) < 0 || strcmp(utils_lowercase(buffer.message), "y") != 0) {
        strcpy(buffer.message, "x");
        send(connectSocket, &buffer, sizeof(buffer), 0);
        
        close(connectSocket);
        printf("\nClient con pid %d disconnesso.\n",getpid());
        exit(0);
    }
    
    strcpy(buffer.message, "");
    sendMenu(connectSocket, buffer);
    return 1;
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

    // messaggio bidirezionale condiviso
    MSG buffer;
    buffer.isAdmin = 0;

    // Creazione setting
    while(createSettings(argv,argc) != 1){
        printf("Setting:\n");
    }

    // creazione del socket
    if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Errore nella creazione del socket.\n");
        exit(-1);
    }
    
    // gestione dei segnali
    signal(SIGPIPE, customSigPipeHandler);
    signal(SIGINT,customSigHandler);

    serverAddress.sin_family = AF_INET;                       // IPv4
    serverAddress.sin_port = htons(SERVERPORT);               // porta
    serverAddress.sin_addr.s_addr = inet_addr(SERVERADDRESS); // indirizzo server

    // bind del socket
    if((returnCode = bind(serverSocket,(struct sockaddr*) &serverAddress, sizeof(serverAddress)))< 0){
        perror("Errore nel binding del socket.\n");
        exit(-1);
    }

    // listen
    if((returnCode = listen(serverSocket,MAX_CLIENT)) < 0){
        perror("Errore nella listen sul socket.\n");
        exit(-1);
    }

    printf("Server online CTRL+C per terminare\n");

    clientAddressLen = sizeof(clientAddress);

    while(1)
    {
        if((connectSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen)) < 0){
            perror("Errore in accept.\n");
            close(serverSocket);
            exit(-1);
        }
        // crea un processo figlio con fork()
        if(fork() == 0){
            clientIP = inet_ntoa(clientAddress.sin_addr);
            printf("Client connesso! @ %s : %d PID: %d PPID: %d\n",clientIP, connectSocket,getpid(),getppid());

            while(1) {
                strcpy(buffer.message, "");

                // attende richiesta dal client                
                if((returnCode = recv(connectSocket, &buffer, sizeof(buffer), 0)) < 0) {
                    printf("Client @ %s : %d - Errore nella ricezione dei dati.\nClient @ %s : %d disconnesso.\n",clientIP,connectSocket,clientIP,connectSocket);
                    close(connectSocket);
                    exit(-1);
                } else {
                    printf("Client @ %s : %d - isAdmin: %d, Message %s\n",clientIP,connectSocket,buffer.isAdmin,buffer.message);

                    if(strcmp(buffer.message, "x") == 0)
                    {
                        send(connectSocket, &buffer, sizeof(buffer), 0);
                        printf("Client @ %s : %d disconnesso.\n", clientIP, connectSocket);
                        close(connectSocket);
                        // chiude il processo figlio
                        exit(0);
                    }                    

                    // gestione delle richieste                
                    choiseHandler(connectSocket, clientIP, buffer,sem);
                }
            }
        }   
        else 
            close(connectSocket);
    }
}
