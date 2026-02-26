#ifndef DICTIONARY_H
#define DICTIONARY_H

#define MAX 65000
#define MAX_WORD_LENGTH 100

typedef struct {
    char words[MAX][MAX_WORD_LENGTH];
    int freq[MAX];
    int size;
} Dictionary;

int load_dictionary(Dictionary *dict, const char *filename);

int addKeyValue(Dictionary *dict, const char *word, const int dist);

#endif
