#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAX_SENTENCES    500
#define MAX_SENTENCE_LEN 1024
#define NUM_FEATURES     3
#define LEARNING_RATE    0.01
#define EPOCHS           1000

typedef struct {
    char text[MAX_SENTENCE_LEN];
    double features[NUM_FEATURES];
    double probability;  // Probabilistic score (P(y=1|x))
    int label;           // 1 -> important, 0 -> not important 
    int original_index;
} Sentence;



// Feature 1 - sentences near the top of the article tend to be more important 
double feat_position(int index, int total) {
    return (double)(total - index) / total;
}

// Feature 2 - longer sentences tend to carry more information, max 100 chars 
double feat_length(const char *text) {
    int len = strlen(text);
    return len > 100 ? 1.0 : (double)len / 100.0;
}

// Feature 3 - keyword count in the sentence
double feat_keywords(const char *text) {
    const char *keywords[] = {"tennis", "sport", "health", "benefit", "fitness"};
    int count = 0;

    // lowercase for case insensitive 
    char lower[MAX_SENTENCE_LEN];
    int i = 0;
    while (text[i]) {
        lower[i] = (text[i] >= 'A' && text[i] <= 'Z') ? text[i] + 32 : text[i];
        i++;
    }
    lower[i] = '\0';

    for (int k = 0; k < 5; k++) {
        if (strstr(lower, keywords[k])) count++;
    }
    return (double)count / 5.0;
}

void compute_features(Sentence *s, int index, int total) {
    s->features[0] = feat_position(index, total);
    s->features[1] = feat_length(s->text);
    s->features[2] = feat_keywords(s->text);
}

// Sigmoid activation for binary Maximum Entropy (Logistic Regression)
double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

// Dot product of weights and features
double linear_sum(double weights[], double features[]) {
    double sum = 0;
    for (int i = 0; i < NUM_FEATURES; i++)
        sum += weights[i] * features[i];
    return sum;
}

// Train using Maximum Entropy principle (Stochastic Gradient Descent)
void train(Sentence *training_data, int training_count, double weights[]) {
    // initialising weights to 0
    for (int i = 0; i < NUM_FEATURES; i++) weights[i] = 0.0;

    for (int epoch = 0; epoch < EPOCHS; epoch++) {
        for (int s = 0; s < training_count; s++) {
            // prediction (P(y=1|x))
            double predicted = sigmoid(linear_sum(weights, training_data[s].features));
            double error     = training_data[s].label - predicted;

            // weight update
            for (int f = 0; f < NUM_FEATURES; f++)
                weights[f] += LEARNING_RATE * error * training_data[s].features[f];
        }
    }
}

// Sort sentences by probability
int compare_by_prob(const void *a, const void *b) {
    double diff = ((Sentence *)b)->probability - ((Sentence *)a)->probability;
    return (diff > 0) ? 1 : -1;
}


int load_training(const char *filename, Sentence *data) {
    FILE *file = fopen(filename, "r");
    if (!file) { printf("Could not open %s\n", filename); return 0; }

    int count = 0;
    char line[MAX_SENTENCE_LEN];

    while (fgets(line, sizeof(line), file) && count < MAX_SENTENCES) {
        int len = strlen(line);
        if (len > 1 && line[len-1] == '\n') line[len-1] = '\0';

        // last char is for label separted by '|'
        char *divider = strrchr(line, '|');
        if (!divider) continue;

        *divider = '\0';                      
        int label = atoi(divider + 1); // atoi -> str to int

        if (strlen(line) < 5) continue;

        strcpy(data[count].text, line);
        data[count].label = label;
        data[count].original_index = count;
        count++;
    }

    fclose(file);
    return count;
}


int load_article(const char *filename, Sentence *data) {
    FILE *file = fopen(filename, "r");
    if (!file) { printf("Could not open %s\n", filename); return 0; }

    int count = 0;
    char line[MAX_SENTENCE_LEN];

    while (fgets(line, sizeof(line), file) && count < MAX_SENTENCES) {
        int len = strlen(line);
        if (len > 1 && line[len-1] == '\n') line[len-1] = '\0';
        if (strlen(line) < 5) continue;

        strcpy(data[count].text, line);
        data[count].original_index = count;
        count++;
    }

    fclose(file);
    return count;
}

int main() {
    Sentence training_data[MAX_SENTENCES];
    Sentence article[MAX_SENTENCES];

    int training_count = load_training("training.txt", training_data);
    if (training_count == 0) { printf("No training data found.\n"); return 1; }

    int article_count = load_article("article.txt", article);
    if (article_count == 0) { printf("No article sentences found.\n"); return 1; }

    // training 
    printf("Loaded %d training sentences and %d article sentences.\n",
           training_count, article_count);

    for (int i = 0; i < training_count; i++)
        compute_features(&training_data[i], i, training_count);

    double weights[NUM_FEATURES];
    train(training_data, training_count, weights);

    printf("Learned weights (MaxEnt):  position=%.3f  length=%.3f  keywords=%.3f\n",
           weights[0], weights[1], weights[2]);

    // inference
    for (int i = 0; i < article_count; i++) {
        compute_features(&article[i], i, article_count);
        article[i].probability = sigmoid(linear_sum(weights, article[i].features));
    }

    qsort(article, article_count, sizeof(Sentence), compare_by_prob);

    printf("\n--- Probabilistic Importance Ranking (Maximum Entropy Model) ---\n\n");
    for (int i = 0; i < article_count; i++)
        printf("[P = %5.1f%%] -> %s\n", article[i].probability * 100.0, article[i].text);

    return 0;
}
