#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define TABLE_SIZE 100000
#define MAX_WORD 64

// Each candidate word that follows a context and its frequency
typedef struct NextWord {
    char *word;
    int count;
    struct NextWord *next;
} NextWord;

// Hash table entry where the context string is the key
typedef struct {
    char *context;
    NextWord *list;
    int total;
} HashEntry;

HashEntry *table[TABLE_SIZE];

// Hashes context strings to table indices
unsigned int hash(const char *text) {
    unsigned int result = 0;
    while (*text) result = result * 31 + *text++;
    return result % TABLE_SIZE;
}

// Retrieves entry or creates a new one using linear probing for collisions
HashEntry* get_entry(const char *context) {
    unsigned int slot = hash(context);
    while (table[slot] && strcmp(table[slot]->context, context) != 0)
        slot = (slot + 1) % TABLE_SIZE;

    if (!table[slot]) {
        table[slot] = calloc(1, sizeof(HashEntry));
        table[slot]->context = strdup(context);
    }
    return table[slot];
}

// Bumps frequency of a word following a specific context
void record(const char *context, const char *next_word) {
    if (!context || context[0] == '\0') return;

    HashEntry *entry = get_entry(context);
    entry->total++;

    NextWord *candidate = entry->list;
    while (candidate) {
        if (strcmp(candidate->word, next_word) == 0) {
            candidate->count++;
            return;
        }
        candidate = candidate->next;
    }

    NextWord *new_word = malloc(sizeof(NextWord));
    new_word->word = strdup(next_word);
    new_word->count = 1;
    new_word->next = entry->list;
    entry->list = new_word;
}

// Sorts word suggestions by occurrence count (descending)
int compare_by_count(const void *a, const void *b) {
    return (*(NextWord**)b)->count - (*(NextWord**)a)->count;
}

// Retrieves the probability of a word given a context with simple smoothing
double get_prob(const char *context, const char *word) {
    unsigned int slot = hash(context);
    while (table[slot] && strcmp(table[slot]->context, context) != 0)
        slot = (slot + 1) % TABLE_SIZE;

    if (!table[slot] || !table[slot]->list) return 0.000001; 

    NextWord *candidate = table[slot]->list;
    while (candidate) {
        if (strcmp(candidate->word, word) == 0)
            return (double)candidate->count / table[slot]->total;
        candidate = candidate->next;
    }
    return 0.000001; 
}

// Shows top 5 predictions or prints failure message if context is unknown
int predict(const char *context, const char *label) {
    unsigned int slot = hash(context);
    while (table[slot] && strcmp(table[slot]->context, context) != 0)
        slot = (slot + 1) % TABLE_SIZE;

    if (!table[slot] || !table[slot]->list) {
        printf("\n[%s Context: \"%s\"] -> Sequence not found\n", label, context);
        return 0;
    }

    printf("\n[%s Context: \"%s\"]\n", label, context);

    NextWord *candidate = table[slot]->list;
    NextWord *sorted[500];
    int count = 0;
    while (candidate && count < 500) { 
        sorted[count++] = candidate; 
        candidate = candidate->next; 
    }

    qsort(sorted, count, sizeof(NextWord*), compare_by_count);

    int show = count < 5 ? count : 5;
    for (int i = 0; i < show; i++)
        printf(" -> %-15s (%.1f%%)\n", sorted[i]->word, (sorted[i]->count * 100.0) / table[slot]->total);

    return 1;
}

// Calculates and prints the perplexity of a sentence
void compute_perplexity(char **words, int n) {
    if (n < 2) {
        printf("\n[Perplexity: N/A (too short)]\n");
        return;
    }
    double log_sum = 0;
    char ctx[MAX_WORD * 2];

    for (int i = 1; i < n; i++) {
        double p;
        if (i >= 2) {
            sprintf(ctx, "%s %s", words[i-2], words[i-1]);
            p = get_prob(ctx, words[i]);
            if (p <= 0.000001) p = get_prob(words[i-1], words[i]);
        } else {
            p = get_prob(words[i-1], words[i]);
        }
        log_sum += log2(p);
    }
    
    double entropy = -log_sum / (n - 1);
    printf("\n[Sentence Perplexity: %.2f]\n", pow(2, entropy));
}

int main() {
    FILE *file = fopen("t8.shakespeare.txt", "r");
    if (!file) { printf("File not found\n"); return 1; }

    printf("Learning Shakespeare (Trigrams & Bigrams)...\n");

    char w1[MAX_WORD] = "", w2[MAX_WORD] = "", w3[MAX_WORD] = "";
    char ctx[MAX_WORD * 2];

    // Build the n-gram model by sliding through the text
    while (fscanf(file, "%63s", w3) == 1) {
        if (w2[0]) {
            record(w2, w3); // Bigram
            if (w1[0]) {
                sprintf(ctx, "%s %s", w1, w2); // Trigram context concatenation
                record(ctx, w3);
            }
        }
        strcpy(w1, w2);
        strcpy(w2, w3);
    }
    fclose(file);

    char input[1024];
    while (printf("\nSentence: ") && fgets(input, sizeof(input), stdin)) {
        char *words[100];
        int n = 0;
        char *token = strtok(input, " \t\n\r");
        while (token && n < 100) { words[n++] = token; token = strtok(NULL, " \t\n\r"); }

        if (n == 0) continue;
        if (strcmp(words[0], "q") == 0) break;

        compute_perplexity(words, n);

        // Predict next word
        if (n >= 2) {
            sprintf(ctx, "%s %s", words[n-2], words[n-1]);
            predict(ctx, "Trigram");
        }
        predict(words[n-1], "Bigram");
    }

    return 0;
}
