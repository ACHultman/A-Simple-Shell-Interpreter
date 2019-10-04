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
struct bg_pro* head;

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



int  bg_process_count(){
    int count = 0;
    struct bg_pro current = *head;
    while(current.next != NULL){
        count++;
        current = *current.next;
    }
    return count;
}

void bg_list_append(pid_t p, char *args[100]){
    struct bg_pro *new_pro = malloc(sizeof(struct bg_pro)); //unfreed
    char new_args[1000];
    strcpy(new_args, args[0]);

    int i = 1;
    while(args[i] != NULL){
        strcat(new_args, " ");
        strcat(new_args, args[i]);
        i++;
    }

    new_pro->pid = p;
    strcpy(new_pro->command, new_args);
    new_pro->next = NULL;

    if(head == NULL){
        head = new_pro;
    }
    else{
        struct bg_pro *current = head;
        while(current->next != NULL){
            current = current->next;
        }
        current->next = new_pro;
    }
}

void kill_bg_zombies(){
    if(bg_process_count > 0){
        pid_t ter = waitpid(0, NULL, WNOHANG);
        while(ter > 0){
            if(head->pid == ter){
                printf("%d %s has terminated.\n", head->pid, head->command);
                head = head->next;
            }
            else{
                struct bg_pro current = *head;
                while(current.next->pid != ter){
                    current = *current.next;
                }
                printf("%d %s  has terminated.\n", current.next->pid, current.next->command);
                current.next = current.next->next;
            }
            ter = waitpid(0,NULL,WNOHANG);
        }
    }
}

void print_bglist() {
    int count = 0;
    struct bg_pro *current = head;
    kill_bg_zombies();
    if(head == NULL){
        printf("Total Background jobs: %d\n", count);
        return;
    }
    count++;
    while(current->next != NULL){
        count++;
        printf("%d: ", current->pid);
        printf("%s\n", current->command);
        current = current->next;
    }
    printf("%d: ", current->pid);
    printf("%s\n", current->command);
    printf("Total Background jobs: %d\n", count);
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

void execute_bg_program(char* cmd, char *args[100]){
    pid_t p;
    int status;

    if((p = fork()) < 0){
        printf("ERROR: Forked failed.\n\n");
        exit(1);
    }
    else if (p == 0){
        if((execvp(cmd, args) < 0)){
            printf("ERROR: Execvp failed.\n\n");
            kill(p, SIGTERM);
            return;
        }
    }
    else{
        printf("args: %s\n", args[1]);
        bg_list_append(p, args);
    }
}


void process_bg(char *args[100]) {
    if(args[1]==NULL){
        printf("Command required after 'bg'.\n");
        return;
    }

    printf("args: %s\n", *args);
    execute_bg_program(args[1], &args[1]);
}

int main() {
    head = malloc(sizeof(struct bg_pro));
    int bailout = 0;
    head = NULL;
    while (!bailout) {
        char prompt[1000] = "";
        char *reply = readline(create_prompt(prompt));
        char *argsv[100];
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
            process_bg(argsv);
        } else if (!strcmp(argsv[0], "bglist")) {
            print_bglist();
        } else {
            execute_program(argsv[0], argsv);
        }
            kill_bg_zombies();
            free(reply);
    }
        printf("Quitting...\n");
        exit(0);
}


