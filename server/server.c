/*
    -> ascolto connessioni
    -> messaggio di benvenuto
    -> la gestione delle varie connessioni è gestita da una fork()
    -> comunicazione bidirezionale?
*/

#include "server.h"
#include <stdio.h>
//#include <stdlib.h>

void printMenu(){
    printf("%s\n",benvenuto);
    printf("%s\n",scelte);
}

void main(int argc, char const *argv[])
{
    printMenu();
}
