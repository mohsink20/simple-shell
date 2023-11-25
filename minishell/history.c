#include "myshell.h"

HistoryEntry history[MAX_HISTORY];
int history_count = 0;

void add_to_history(char* const tokens[]) {
    if (history_count < MAX_HISTORY) {
        int total_length = 0;

        for (int i = 0; tokens[i] != NULL; i++) {
            total_length += strlen(tokens[i]) + 1;
        }

        history[history_count].command = malloc(total_length);

        strcpy(history[history_count].command, tokens[0]);
        for (int i = 1; tokens[i] != NULL; i++) {
            strcat(history[history_count].command, " ");
            strcat(history[history_count].command, tokens[i]);
        }

        history_count++;
    } else {
        free(history[0].command);
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            history[i] = history[i + 1];
        }
        int last_index = MAX_HISTORY - 1;
        int total_length = 0;

        for (int i = 0; history[last_index - 1].command[i] != '\0'; i++) {
            total_length++;
        }

        history[last_index].command = malloc(total_length);

        strcpy(history[last_index].command, history[last_index - 1].command);
    }
}

void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i].command);
    }
}

char* get_command_from_history(int command_number) {
    if (command_number > 0 && command_number <= history_count) {
        return strdup(history[command_number - 1].command);
    }
    return NULL;
}

char* handle_history_recall(char* input) {
    if (history_count == 0) {
        printf("No command in history\n");
        return NULL;
    }

    if (input[0] == '!') {
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
            return strdup(history[history_count - 1].command);
        } else {
            printf("Invalid history command\n");
            return NULL;
        }
    }

    return NULL;
}
