#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>

struct bg_pro{
    pid_t pid;
    char command[1024];
    struct bg_pro* next;
};
struct bg_pro* head = NULL;

char* create_prompt(char prompt[]){
    char cwd[256];
    char host[256];
    char *user;
    host[255] = '\0';
    cwd[255] = '\0';

    user = getlogin();
    gethostname(host, sizeof(host));
    getcwd(cwd, sizeof(cwd));

    strcat(prompt, "SSI: ");
    strcat(prompt, user);
    strcat(prompt, "@");
    strcat(prompt, host);
    strcat(prompt, ": ");
    strcat(prompt, cwd);
    strcat(prompt, "> ");

    return prompt;
}

void bg_list_append(pid_t p, char **args){
    struct bg_pro *current = head;
    while(current->next != NULL){
        current = current->next;
    }
    current->pid = p;
    strcpy(current->command, *args);
    current->next = NULL;
}


void print_bglist() {
    struct bg_pro* current = head;
    while(current != NULL){
        printf("%d: ", current->pid);
        printf("%s ", current->command);
        current = current -> next;
    }
}


void process_cd(char *args[10]) {
    char cwd[256];
    cwd[255] = '\0';
    getcwd(cwd, sizeof(cwd));
    if(args[1] == NULL || *args[1] == '~'){
        chdir(getenv("HOME"));
    }
    chdir(args[1]);

}

void execute_program(char* cmd, char *args[10]){
    pid_t pid;
    int status;

    if((pid = fork()) < 0){
        printf("ERROR: Forked failed.\n\n");
        exit(1);
    }
    else if (pid == 0){
        if((execvp(cmd, args) < 0)){
            printf("ERROR: Execvp failed.\n\n");
        }
    }
    else{
        while(wait(&status) == 0)
            ;
    }

}

void execute_bg_program(char* cmd, char *args[9]){
    printf("%s\n", cmd);
    printf("In execute_bg_program\n");

    pid_t p;
    int status;

    if((p = fork()) < 0){
        printf("ERROR: Forked failed.\n\n");
        exit(1);
    }
    else if (p == 0){
        if((execvp(cmd, args) < 0)){
            printf("ERROR: Execvp failed.\n\n");
        }
        head->pid = p;
        strcpy(head->command, *args);
        head->next = NULL;
    }
    else{
        bg_list_append(p, args);
        while(wait(&status) == 0)
            ;
    }

}


void process_bg(char *args[10]) {
    printf("Process bg...\n");
    char *new_args[10];
    printf("before for loop");
    for(int i = 1; i < 10; i++){
        new_args[i-1] = args[i];
    }
    new_args[9] = NULL;
    execute_bg_program(new_args[0], new_args);
}

/*
void execute_bg_program(char* cmd, char *args[10]){
    pid_t pid;
    int status;

    if((pid = fork()) < 0){
        printf("ERROR: Forked failed.\n\n");
        exit(1);
    }
    else if (pid == 0){
        if((execvp(cmd, args) < 0)){
            printf("ERROR: Execvp failed.\n\n");
        }
    }
    else{
        if((execvp(cmd, args) == 0)){

        }
        else{

        }
    }

}
*/

int main() {
    printf("MAIN\n");
    int bailout = 0;
    head = NULL;
    while (!bailout) {
        char prompt[1000] = "";
        char *reply = readline(create_prompt(prompt));
        printf("reply\n");
        char *argsv[10];
        int i = 0;
        char *token = strtok(reply, " ");
        if (token == NULL){
            continue;
        }
        for (i = 0; token != NULL; i++) {
            argsv[i] = token;
            token = strtok(NULL, " ");
        }

        argsv[i] = NULL;
        int end_of_argsv = i;

        if (!strcmp(argsv[0], "quit")) {
            bailout = 1;
        } else if (!strcmp(argsv[0], "cd")) {
            process_cd(argsv);
        } else if (!strcmp(argsv[0], "bg")) {
            printf("First argument is bg\n");
            process_bg(argsv);
        } else if (!strcmp(argsv[0], "bglist")) {
            print_bglist();
        } else {
            execute_program(argsv[0], argsv);
        }

            free(reply);
    }
        printf("Quitting...\n");
        exit(0);
}


