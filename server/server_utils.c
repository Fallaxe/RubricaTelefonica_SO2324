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

////////////////////////////////////////// dalla libreria cJSONUtils /////////////////////////////////////////////////
static int compare_strings(const unsigned char *string1, const unsigned char *string2)
{
    if ((string1 == NULL) || (string2 == NULL))
    {
        return 1;
    }

    if (string1 == string2)
    {
        return 0;
    }

    return strcmp((const char*)string1, (const char*)string2);
}

CJSON_PUBLIC(cJSON *) utils_sortByKey(cJSON *list, const char * const key)
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

    while ((current_item != NULL) && (current_item->next != NULL) && (compare_strings(utils_lowercase(cJSON_GetObjectItem(current_item,key)->valuestring), utils_lowercase(cJSON_GetObjectItem((current_item->next),key)->valuestring)) < 0))
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
    first = utils_sortByKey(first, key);
    second = utils_sortByKey(second, key);
    result = NULL;

    /* Merge the sub-lists */
    while ((first != NULL) && (second != NULL))
    {
        cJSON *smaller = NULL;
        if (compare_strings(utils_lowercase(cJSON_GetObjectItem(current_item,key)->valuestring), utils_lowercase(cJSON_GetObjectItem((current_item->next),key)->valuestring)) < 0)
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