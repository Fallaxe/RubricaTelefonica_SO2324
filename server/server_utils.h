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
static int check_stdin();
void clean_stdin();

// Aggiunta alla libreria cJSONUtils
CJSON_PUBLIC(void) utils_sortByKey(cJSON * object, char * key);

//  hash utilites (sha256)
void hashToHexString(const unsigned char *hash, int length, char *output);
void inToSha256(const char *inToHash, char *destination);