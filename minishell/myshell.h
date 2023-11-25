#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glob.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKENS 100
#define MAX_HISTORY 100
#define MAX_BACKGROUND_PROCESSES 100
#define MAX_SEQUENTIAL_PROCESSES 100

extern char prompt[MAX_INPUT_SIZE];
extern int sequential_processes[MAX_SEQUENTIAL_PROCESSES];
extern int sequential_process_count;

extern int background_processes[MAX_BACKGROUND_PROCESSES];
extern int background_process_count;


typedef struct {
    char* command;
} HistoryEntry;

void sigtstp_handler();
void sigint_handler();
void sigquit_handler();

void expand_wildcards(char** tokens);

void add_to_history(char* const tokens[]);
void print_history();
char* get_command_from_history(int command_number);
char* handle_history_recall(char* input);

char** tokenize(char* line);

void change_prompt(const char* newprompt);

int execute(char** tokens);
void handle_sequential_processes();
void handle_background_processes();
