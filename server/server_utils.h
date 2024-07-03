/*
    7109803 Miranda Bellezza
    7112588 Daniele Fallaci
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <sys/time.h>
#include <sys/types.h>

#include "../vendor/cjson/cJSON.h"

char *utils_lowercase(char * stringa);
int utils_strIncludeOnly(char * str, char * digits);
int utils_isValidEmail(char * email);
void clean_stdin();

// Aggiunta alla libreria cJSONUtils
CJSON_PUBLIC(void) utils_sortByKey(cJSON * object, char * key);

//  hash utilites (sha256)
void inToSha256(const char *inToHash, char *destination);