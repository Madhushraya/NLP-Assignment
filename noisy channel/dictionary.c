#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dictionary.h"

int load_dictionary(Dictionary *dict, const char *filename)
{
    FILE *file = fopen(filename, "r");

    if (file == NULL)
    {
        return 0;
    }

    char line[1024];
    int count = 0;

    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file) && count < MAX)
    {
        char *token = strtok(line, ",");
        int c = 0;

        while (token != NULL)
        {
            if (c == 1)
                strcpy(dict->words[count], token);

            if (c == 3)
                dict->freq[count] = atoi(token);

            token = strtok(NULL, ",");
            c++;
        }

        count++;
    }

    dict->size = count;
    fclose(file);
    return count;
}

int addKeyValue(Dictionary *dict, const char *word, const int dist) {
    if (dict->size < MAX) {
        strcpy(dict->words[dict->size], word);
        dict->freq[dict->size] = dist;
        dict->size++;
        return 1;
    }
    return 0;
}
