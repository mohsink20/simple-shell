#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKENS 100

char prompt[MAX_INPUT_SIZE] = "meowsh😺$ ";

// Signal handler function for SIGTSTP (CTRL-Z)
void sigtstp_handler(int signo) {
    // Ignore the signal
}

// Signal handler function for SIGINT (CTRL-C)
void sigint_handler(int signo) {
    // Ignore the signal
}

// Signal handler function for SIGQUIT (CTRL-\)
void sigquit_handler(int signo) {
    // Ignore the signal
}

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

void change_prompt(const char* newprompt) {
    if (newprompt != NULL) {
        if (strstr(newprompt, " ") != NULL) {
            snprintf(prompt, sizeof(prompt), "%s", newprompt);
        } else {
            snprintf(prompt, sizeof(prompt), "%s", newprompt);
        }
    }
}

int execute(char** tokens) {
    if (tokens[0] == NULL) {
        return 1;
    } else if (strcmp(tokens[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(tokens[0], "prompt") == 0) {
        if (tokens[1] != NULL) {
            change_prompt(strdup(tokens[1]));
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
        // Check for pipeline
        int pipe_index = -1;
        for (int i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], "|") == 0) {
                pipe_index = i;
                break;
            }
        }

        if (pipe_index != -1) {
            // Pipeline
            char** first_command = tokens;
            char** second_command = tokens + pipe_index + 1;
            tokens[pipe_index] = NULL;

            int pipe_fd[2];
            if (pipe(pipe_fd) == -1) {
                perror("pipe");
                exit(1);
            }

            pid_t pid = fork();

            if (pid == 0) {
                // Child process (left side of the pipe)
                close(pipe_fd[0]);
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);

                // Execute the first command
                execvp(first_command[0], first_command);
                perror("execvp");
                exit(1);
            } else if (pid > 0) {
                // Parent process
                wait(NULL);
                close(pipe_fd[1]);

                pid_t pid2 = fork();

                if (pid2 == 0) {
                    // Child process (right side of the pipe)
                    close(pipe_fd[1]);
                    dup2(pipe_fd[0], STDIN_FILENO);
                    close(pipe_fd[0]);

                    // Execute the second command
                    execvp(second_command[0], second_command);
                    perror("execvp");
                    exit(1);
                } else if (pid2 > 0) {
                    // Parent process
                    close(pipe_fd[0]);
                    wait(NULL);
                } else {
                    perror("fork");
                    exit(1);
                }
            } else {
                perror("fork");
                exit(1);
            }
        } else {
            // No pipeline
            glob_t results;
            int stat = glob(tokens[0], GLOB_TILDE, NULL, &results);
            if (stat == 0) {
                change_prompt(results.gl_pathv[0]);
                globfree(&results);
            }

            pid_t pid = fork();
            if (pid == 0) {
                // Redirection
                int fd_in = -1, fd_out = -1;
                for (int i = 1; tokens[i] != NULL; i++) {
                    if (strcmp(tokens[i], "<") == 0) {
                        fd_in = open(tokens[i + 1], O_RDONLY);
                        if (fd_in < 0) {
                            perror("open");
                            exit(1);
                        }
                        dup2(fd_in, STDIN_FILENO);
                        close(fd_in);
                        tokens[i] = NULL;
                        tokens[i + 1] = NULL;
                    } else if (strcmp(tokens[i], ">") == 0) {
                        fd_out = open(tokens[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                        if (fd_out < 0) {
                            perror("open");
                            exit(1);
                        }
                        dup2(fd_out, STDOUT_FILENO);
                        close(fd_out);
                        tokens[i] = NULL;
                        tokens[i + 1] = NULL;
                    } else if (strcmp(tokens[i], "2>") == 0) {
                        fd_out = open(tokens[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                        if (fd_out < 0) {
                            perror("open");
                            exit(1);
                        }
                        dup2(fd_out, STDERR_FILENO);
                        close(fd_out);
                        tokens[i] = NULL;
                        tokens[i + 1] = NULL;
                    }
                }

                execvp(tokens[0], tokens);
                perror("execvp");
                exit(1);
            } else {
                wait(NULL);
            }
        }
    }

    return 0;
}

int main() {
    // Set up signal handlers to ignore signals
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);

    FILE* fp = fopen("info.txt", "r");
    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        printf("%c", ch);
    }
    fclose(fp);

    printf("\n");
    while (1) {
        printf("%s", prompt);

        char input[MAX_INPUT_SIZE];
        if (!fgets(input, MAX_INPUT_SIZE, stdin)) {
            printf("\n");
            break;
        }

        char** tokens = tokenize(input);

        int status = execute(tokens);

        free(tokens);

        if (status) {
            continue;
        }
    }
    return 0;
}
