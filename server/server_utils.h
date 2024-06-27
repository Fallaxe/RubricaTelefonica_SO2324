#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../vendor/cjson/cJSON.h"

char * utils_lowercase(char * stringa);
int utils_strIncludeOnly(char * str, char * digits);

// Aggiunta alla libreria cJSONUtils
CJSON_PUBLIC(cJSON *) utils_sortByKey(cJSON *list, const char * const key);