#include "server_utils.h"

char * utils_lowercase(char * stringa)
{
    for(char * ptr = stringa; *ptr; ptr++) *ptr = tolower(*ptr);
    return stringa;
}

int utils_strIncludeOnly(char * str, char * digits)
{
    int found;
    for(int i = 0; i < strlen(str); i++)
    {
        found = 0;
        for(int j = 0; j < strlen(digits); j++) 
        {
            if(str[i] == digits[j])
            {
                found = 1;
                break;
            }
        }
        if (found == 0) return 0;
    }
    return 1;
}

void hashToHexString(const unsigned char *hash, int length, char *output) {
    const char *hexChars = "0123456789abcdef";
    for (int i = 0; i < length; i++) {
        output[i * 2] = hexChars[(hash[i] >> 4) & 0xF];
        output[i * 2 + 1] = hexChars[hash[i] & 0xF];
    }
    output[length * 2] = '\0'; // determinare la fine della stringa
}

static void handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}

void inToSha256(const char *inToHash, char *destination)
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


    hashToHexString(mid,len,destination);
}

void clean_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

////////////////////////////////////////// dalla libreria cJSONUtils /////////////////////////////////////////////////
static int compare_strings(char *string1, char *string2)
{
    if ((string1 == NULL) || (string2 == NULL))
    {
        return 1;
    }

    string1 = utils_lowercase(string1);
    string1 = utils_lowercase(string1);

    if (string1 == string2)
    {
        return 0;
    }

    return strcmp(string1, string2);
}

static cJSON *sort_bykey(cJSON *list, const char * const key)
{
    cJSON *first = list;
    cJSON *second = list;
    cJSON *current_item = list;
    cJSON *result = list;
    cJSON *result_tail = NULL;

    if ((list == NULL) || (list->next == NULL))
    {
        /* One entry is sorted already. */
        return result;
    }

    while ((current_item != NULL) && (current_item->next != NULL) && (compare_strings(cJSON_GetObjectItem(current_item,key)->valuestring, cJSON_GetObjectItem((current_item->next),key)->valuestring) < 0))
    {
        /* Test for list sorted. */
        current_item = current_item->next;
    }
    if ((current_item == NULL) || (current_item->next == NULL))
    {
        /* Leave sorted lists unmodified. */
        return result;
    }

    /* reset pointer to the beginning */
    current_item = list;
    while (current_item != NULL)
    {
        /* Walk two pointers to find the middle. */
        second = second->next;
        current_item = current_item->next;
        /* advances current_item two steps at a time */
        if (current_item != NULL)
        {
            current_item = current_item->next;
        }
    }
    if ((second != NULL) && (second->prev != NULL))
    {
        /* Split the lists */
        second->prev->next = NULL;
        second->prev = NULL;
    }

    /* Recursively sort the sub-lists. */
    first =sort_bykey(first, key);
    second = sort_bykey(second, key);
    result = NULL;

    /* Merge the sub-lists */
    while ((first != NULL) && (second != NULL))
    {
        cJSON *smaller = NULL;
        if (compare_strings(cJSON_GetObjectItem(first,key)->valuestring, cJSON_GetObjectItem(second,key)->valuestring) < 0)
        {
            smaller = first;
        }
        else
        {
            smaller = second;
        }

        if (result == NULL)
        {
            /* start merged list with the smaller element */
            result_tail = smaller;
            result = smaller;
        }
        else
        {
            /* add smaller element to the list */
            result_tail->next = smaller;
            smaller->prev = result_tail;
            result_tail = smaller;
        }

        if (first == smaller)
        {
            first = first->next;
        }
        else
        {
            second = second->next;
        }
    }

    if (first != NULL)
    {
        /* Append rest of first list. */
        if (result == NULL)
        {
            return first;
        }
        result_tail->next = first;
        first->prev = result_tail;
    }
    if (second != NULL)
    {
        /* Append rest of second list */
        if (result == NULL)
        {
            return second;
        }
        result_tail->next = second;
        second->prev = result_tail;
    }

    return result;
}

CJSON_PUBLIC(void) utils_sortByKey(cJSON * object, char * key)
{
    if (object == NULL || key == NULL || cJSON_IsArray(object) == 0 || cJSON_GetArraySize(object) < 2)
    {
        return;
    }

    object->child = sort_bykey(object->child, key);
}