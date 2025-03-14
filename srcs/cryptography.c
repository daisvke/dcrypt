#include "stockholm.h"

/* Generates an encryption key randomly.
    - charset: a set of characters that can be used for the key
    - strength: the width of the key
*/
char *fa_keygen(const char *charset, size_t strength)
{
    char *key = malloc((strength + 1) * sizeof(char));
    // Exit in case malloc fails
    if (key == NULL)
        return NULL;

    int charset_length = fa_strlen(charset);

    // Set the seed for the random number generator used by the rand() function.
    // By providing a seed, we can ensure that the sequence of random numbers
    // generated by rand() is different each time your program runs.
    // The seed is set according to the current time, which makes it unique.
    srand(time(NULL));

    for (size_t i = 0; i < strength; ++i)
    {
        int _random_index = rand() % charset_length;
        // Pick a random position from the charset
        key[i] = charset[_random_index];
    }
    key[strength] = '\0'; // null-terminate the key

    return key;
}