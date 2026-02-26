#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dictionary.h"
#include "levenshtein.h"

typedef struct {
    char word[MAX_WORD_LENGTH];
    int distance;
    int frequency;
} Candidate;

int compareCandidates(const void *a, const void *b) {
    Candidate *c1 = (Candidate *)a;
    Candidate *c2 = (Candidate *)b;
    if (c1->distance != c2->distance)
        return c1->distance - c2->distance;
    return c2->frequency - c1->frequency;
}

int main()
{
    static Dictionary dict;
    
    int count = load_dictionary(&dict, "lemmas_60k.csv");

    if (count == 0) {
        printf("Failed to load dictionary.\n");
        return 1;
    }

    printf("Loaded %d words\n", count);

    for(int i = 0; i < 10; i++)
    {
        printf("%s - %d\n", dict.words[i], dict.freq[i]);
    }

    char input[MAX_WORD_LENGTH];
    while (1) {
        printf("\nEnter word (or 'q' to quit): ");
        if (scanf("%99s", input) != 1 || strcmp(input, "q") == 0) break;

        // have to calculate levenshtein distance
        // optimising by calculating distance for words which are +- 2 length
        int wordLen = strlen(input);
        int maxLen = wordLen + 2;
        int minLen = wordLen - 2;

        static Candidate candidates[MAX];
        int candCount = 0;

        // Dictionary canditates
        for (int i = 0; i < dict.size; i++) {
            int dictLen = strlen(dict.words[i]);
            // only check words that are within 2 characters in length
            if (dictLen >= minLen && dictLen <= maxLen) {
                int dist = levenshtein(input, dict.words[i]);
                // use only words that are within a distance of 2
                if (dist <= 2) {
                    strcpy(candidates[candCount].word, dict.words[i]);
                    candidates[candCount].distance = dist;
                    candidates[candCount].frequency = dict.freq[i];
                    candCount++;
                }
            }
        }

        // sort candidates by distance (priority 1) and frequency (priority 2)
        qsort(candidates, candCount, sizeof(Candidate), compareCandidates);

        printf("Suggestions:\n");
        int limit = candCount < 10 ? candCount : 10;
        for (int i = 0; i < limit; i++) {
            printf("%s (dist: %d, freq: %d)\n", candidates[i].word, candidates[i].distance, candidates[i].frequency);
        }
        if (candCount == 0) printf("No close matches found.\n");
    }

    return 0;
}
