#include "myshell.h"

int execute(char** tokens) {
      int background = 0;
        int sequential = 0;
        int i;

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
    } else if (strcmp(tokens[0], "history") == 0) {
        print_history();
        return 1;
    } else {
      
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

        char* recalled_command = handle_history_recall(tokens[0]);
        if (recalled_command != NULL) {
            char** recalled_tokens = tokenize(recalled_command);
            add_to_history(recalled_tokens);
            tokens = recalled_tokens;
            free(recalled_command);
        } else {
            add_to_history(tokens);
        }

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
                    waitpid(pid, NULL, 0);
                    waitpid(pid2, NULL, 0);
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
                    } else if (strcmp(tokens[j], ">>") == 0) {
                        fd_out = open(tokens[j + 1], O_WRONLY | O_CREAT | O_APPEND, 0666);
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
                if (!background) {
                    waitpid(pid, NULL, 0);
                } else {
                    printf("[%d] %d\n", background_process_count + 1, pid);
                    background_processes[background_process_count++] = pid;
                }

                if (!sequential && tokens[i + 1] != NULL) {
                    execute(tokens + i + 1);
                }
            }
        }
    }
if (sequential) {
    handle_sequential_processes();
    // Execute the next command in the sequence
    execute(tokens + i + 1);
} else {
    handle_background_processes();
}

    return 0;
}

