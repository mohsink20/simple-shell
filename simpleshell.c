#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glob.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKENS 100
#define MAX_HISTORY 100

char prompt[MAX_INPUT_SIZE] = "meowsh😺$ ";

char* history[MAX_HISTORY];
int history_count = 0;

// Signal handler function for SIGTSTP (CTRL-Z)
void sigtstp_handler(int signo) {
}

// Signal handler function for SIGINT (CTRL-C)
void sigint_handler(int signo) {
}

// Signal handler function for SIGQUIT (CTRL-\)
void sigquit_handler(int signo) {
}

void expand_wildcards(char** tokens) {
    char* expanded_tokens[MAX_TOKENS];
    int expanded_index = 0;

    for (int i = 0; tokens[i] != NULL; i++) {
        if (strpbrk(tokens[i], "*?") != NULL) {
            glob_t results;
            glob(tokens[i], GLOB_TILDE, NULL, &results);
            for (int j = 0; j < results.gl_pathc; j++) {
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
// Function to add a command to history
void add_to_history(const char *command) {
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(command);
    } else {
        free(history[0]);
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            history[i] = history[i + 1];
        }
        history[MAX_HISTORY - 1] = strdup(command);
    }
}

// Function to print command history
void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

// Function to get command from history by number
char* get_command_from_history(int command_number) {
    if (command_number > 0 && command_number <= history_count) {
        return strdup(history[command_number - 1]);
    }
    return NULL;
}

// Function to handle command history recall
char* handle_history_recall(char* input) {
    int command_number = atoi(input + 1);
    if (command_number > 0) {
        char* recalled_command = get_command_from_history(command_number);
        if (recalled_command != NULL) {
            return recalled_command;
        } else {
            printf("Command not found in history\n");
            return NULL;
        }
    } else if (strcmp(input, "!!") == 0) {
        if (history_count > 0) {
            return strdup(history[history_count - 1]);
        } else {
            printf("No command in history\n");
            return NULL;
        }
    } else {
        printf("Invalid history command\n");
        return NULL;
    }
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
        snprintf(prompt, sizeof(prompt), "%s", newprompt);
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
        int background = 0;
        int sequential = 0;
        int i;

        for (i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], "&") == 0) {
                background = 1;
                tokens[i] = NULL;
                break;
            } else if (strcmp(tokens[i], ";") == 0) {
                sequential = 1;
                tokens[i] = NULL;
                break;
            }
        }

        int pipe_index = -1;
        for (int j = 0; tokens[j] != NULL; j++) {
            if (strcmp(tokens[j], "|") == 0) {
                pipe_index = j;
                break;
            }
        }
        expand_wildcards(tokens);

         if (tokens[0][0] == '!') {
            int command_number = atoi(tokens[0] + 1);
            if (command_number > 0) {
                char* recalled_command = get_command_from_history(command_number);
                if (recalled_command != NULL) {
                    tokens = tokenize(recalled_command);
                    free(recalled_command);
                } else {
                    printf("Command not found in history\n");
                    return 1;
                }
            } else if (strcmp(tokens[0], "!!") == 0) {
                if (history_count > 0) {
                    char* last_command = strdup(history[history_count - 1]);
                    tokens = tokenize(last_command);
                    free(last_command);
                } else {
                    printf("No command in history\n");
                    return 1;
                }
            } else {
                printf("Invalid history command\n");
                return 1;
            }
        }

        add_to_history(tokens[0]);


        if (pipe_index != -1) {
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
                close(pipe_fd[0]);
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);

                execvp(first_command[0], first_command);
                perror("execvp");
                exit(1);
            } else if (pid > 0) {
                wait(NULL);
                close(pipe_fd[1]);

                pid_t pid2 = fork();

                if (pid2 == 0) {
                    close(pipe_fd[1]);
                    dup2(pipe_fd[0], STDIN_FILENO);
                    close(pipe_fd[0]);

                    execvp(second_command[0], second_command);
                    perror("execvp");
                    exit(1);
                } else if (pid2 > 0) {
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
            glob_t results;
            int stat = glob(tokens[0], GLOB_TILDE, NULL, &results);
            if (stat == 0) {
                change_prompt(results.gl_pathv[0]);
                globfree(&results);
            }

            pid_t pid = fork();
            if (pid == 0) {
                int fd_in = -1, fd_out = -1;
                for (int j = 1; tokens[j] != NULL; j++) {
                    if (strcmp(tokens[j], "<") == 0) {
                        fd_in = open(tokens[j + 1], O_RDONLY);
                        if (fd_in < 0) {
                            perror("open");
                            exit(1);
                        }
                        dup2(fd_in, STDIN_FILENO);
                        close(fd_in);
                        tokens[j] = NULL;
                        tokens[j + 1] = NULL;
                    } else if (strcmp(tokens[j], ">") == 0) {
                        fd_out = open(tokens[j + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                        if (fd_out < 0) {
                            perror("open");
                            exit(1);
                        }
                        dup2(fd_out, STDOUT_FILENO);
                        close(fd_out);
                        tokens[j] = NULL;
                        tokens[j + 1] = NULL;
                    } else if (strcmp(tokens[j], "2>") == 0) {
                        fd_out = open(tokens[j + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                        if (fd_out < 0) {
                            perror("open");
                            exit(1);
                        }
                        dup2(fd_out, STDERR_FILENO);
                        close(fd_out);
                        tokens[j] = NULL;
                        tokens[j + 1] = NULL;
                    }
                }

                execvp(tokens[0], tokens);
                perror("execvp");
                exit(1);
            } else {
                if (!background && !sequential) {
                    waitpid(pid, NULL, 0);
                } else if (background) {
                    pid_t shell_pid = getpid();
                    if (pid != 0) {
                  
                        freopen("/dev/null", "w", stdout);
                        freopen("/dev/null", "w", stderr);
                        printf("[%d] %d\n", shell_pid, pid);
                    }
                } else if (sequential) {
                    waitpid(pid, NULL, 0);
                    if (tokens[i + 1] != NULL) {
                        execute(tokens + i + 1); // Execute the command after ;
                    }
                }
            }
        }
    }

    return 0;
}


int main() {

    rl_bind_key('\t', rl_complete);
    using_history();

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
        char* input = readline(prompt);

        if (!input) {
            printf("\n");
            break;
        }

        if (input && *input) {
        
            if (input[0] == '!') {
                char* recalled_command = handle_history_recall(input);
                if (recalled_command != NULL) {
                    free(input);
                    input = recalled_command;
                } else {
                    free(input);
                    continue; 
                }
            }

            add_to_history(input);

            char** tokens = tokenize(input);

            
            expand_wildcards(tokens);

            int status = execute(tokens);

            // Free allocated memory
            for (int i = 0; tokens[i] != NULL; i++) {
                free(tokens[i]);
            }
            free(tokens);
        }

        free(input);
    }

    clear_history();

    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    return 0;
}
