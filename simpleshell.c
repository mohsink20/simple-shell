#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKENS 100

char prompt[MAX_INPUT_SIZE] = "% ";

char** tokenize(char* line) {
    char** tokens = malloc(MAX_TOKENS * sizeof(char*));
    char* token = strtok(line, " \n");
    int i = 0;
    while (token != NULL) {
        tokens[i] = token;
        i++;
        token = strtok(NULL, " \n");
    }
    tokens[i] = NULL;
    return tokens;
}

void changePrompt(const char* newPrompt) {
    if (newPrompt != NULL) {
        snprintf(prompt, sizeof(prompt), "%s", newPrompt);
    }
}

int execute(char** tokens) {
    if (tokens[0] == NULL) {
        return 1;
    } else if (strcmp(tokens[0], "prompt") == 0) {
        if (tokens[1] != NULL) {
            changePrompt(tokens[1]);
        }
        return 1;
    } else if (strcmp(tokens[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
        return 1;
    } else if (strcmp(tokens[0], "cd") == 0) {
        if (tokens[1] == NULL) {
            chdir(getenv("HOME"));
        } else {
            if (chdir(tokens[1]) != 0) {
                perror("chdir");
            }
        }
        return 1;
    } else {
        glob_t results;
        int stat = glob(tokens[0], GLOB_TILDE, NULL, &results);
        if (stat == 0) {
            changePrompt(results.gl_pathv[0]);
            globfree(&results);
        }

        pid_t pid = fork();
        if (pid == 0) {
            execvp(tokens[0], tokens);
            perror("execvp");
            exit(1);
        } else {
            wait(NULL);
        }
    }

    return 0;
}

int main() {
    while (1) {
        printf("%s", prompt);

        char input[MAX_INPUT_SIZE];
        fgets(input, MAX_INPUT_SIZE, stdin);

        char** tokens = tokenize(input);

        int status = execute(tokens);

        free(tokens);

        if (status) {
            continue;
        }
    }

    return 0;
}
