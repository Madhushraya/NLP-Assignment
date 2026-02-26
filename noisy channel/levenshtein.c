#include <string.h>
#include "levenshtein.h"

#define MAX 100

static int min3(int a, int b, int c)
{
    int m = a;
    if (b < m)
    {
        m = b;
    }
    if (c < m)
    {
        m = c;
    }
    return m;
}

int levenshtein(const char *s, const char *t)
{
    int ls = strlen(s);
    int lt = strlen(t);

    int d[MAX + 1][MAX + 1];

    for (int i = 0; i <= ls; i++)
    {
        d[i][0] = i;
    }

    for (int j = 0; j <= lt; j++)
    {
        d[0][j] = j;
    }

    for (int i = 1; i <= ls; i++)
    {
        for (int j = 1; j <= lt; j++)
        {
            if (s[i - 1] == t[j - 1])
            {
                d[i][j] = d[i - 1][j - 1];
            }
            else
            {
                d[i][j] = min3(
                    d[i - 1][j] + 1,    // deletion
                    d[i][j - 1] + 1,    // insertion
                    d[i - 1][j - 1] + 2 // substitution
                );
            }
        }
    }

    return d[ls][lt];
}