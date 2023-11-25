#include "myshell.h"

char** tokenize(char* line) {
    char* copy = strdup(line);
    char** tokens = malloc(MAX_TOKENS * sizeof(char*));
    char* token = strtok(copy, " \n");
    int i = 0;
    while (token != NULL) {
        tokens[i] = strdup(token);
        i++;
        token = strtok(NULL, " \n");
    }
    tokens[i] = NULL;
    free(copy);
    return tokens;
}
