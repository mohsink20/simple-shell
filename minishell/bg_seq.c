#include "myshell.h"

int sequential_processes[MAX_SEQUENTIAL_PROCESSES];
int sequential_process_count = 0;

int background_processes[MAX_BACKGROUND_PROCESSES];
int background_process_count = 0;

void handle_sequential_processes() {
    for (int i = 0; i < sequential_process_count; i++) {
        int status;
        pid_t result = waitpid(sequential_processes[i], &status, 0);

        if (result > 0) {
            printf("[%d] Done\t%d\n", i + 1, sequential_processes[i]);
            for (int j = i; j < sequential_process_count - 1; j++) {
                sequential_processes[j] = sequential_processes[j + 1];
            }
            sequential_process_count--;
            i--;
        }
    }
}

void handle_background_processes() {
    for (int i = 0; i < background_process_count; i++) {
        int status;
        pid_t result = waitpid(background_processes[i], &status, WNOHANG);

        if (result > 0) {
            printf("[%d] Done\t%d\n", i + 1, background_processes[i]);
            for (int j = i; j < background_process_count - 1; j++) {
                background_processes[j] = background_processes[j + 1];
            }
            background_process_count--;
            i--;
        }
    }
}
