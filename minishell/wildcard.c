#include "myshell.h"

void expand_wildcards(char** tokens) {
    char* expanded_tokens[MAX_TOKENS];
    int expanded_index = 0;

    for (int i = 0; tokens[i] != NULL; i++) {
        if (strpbrk(tokens[i], "*?") != NULL) {
            glob_t results;
            glob(tokens[i], GLOB_TILDE, NULL, &results);
            for (size_t j = 0; j < results.gl_pathc; j++) {
                expanded_tokens[expanded_index++] = strdup(results.gl_pathv[j]);
            }

            globfree(&results);
        } else {
            expanded_tokens[expanded_index++] = strdup(tokens[i]);
        }
    }
    for (int i = 0; i < expanded_index; i++) {
        tokens[i] = expanded_tokens[i];
    }
    tokens[expanded_index] = NULL;
}
