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

void process_cd(char *args[10]);

void process_bg(char *args[10]);

void process_bglist(char *args[10]);

char * create_prompt(char prompt[1000]);

void execute_program(char *args[10]){
    char *cmd = args[0];
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
        while (wait(&status) != pid)
            ;
    }

}

int main() {
    int bailout = 0;
    while (!bailout) {
        char prompt[1000] = "";

        char *reply = readline(create_prompt(prompt));

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
        printf("----------------------------------------\n");
        for (i = 0; argsv[i] != NULL; ++i)
            printf("%s\n", argsv[i]);

        /* Note that readline strips away the final \n */
        /* For Perl junkies, readline automatically chomps the line read */
        if (!strcmp(argsv[0], "exit")) {
            bailout = 1;
        } else if (!strcmp(argsv[0], "cd")) {
            process_cd(argsv);
        } else if (!strcmp(argsv[0], "bg")) {
            process_bg(argsv);
        } else if (!strcmp(argsv[0], "bglist")) {
            process_bglist(argsv);
        } else {
            execute_program(argsv);
        }


            free(reply);
    }
        printf("Auf Wiedersehen!\n");
}

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

void process_bglist(char *args[10]) {

}

void process_bg(char *args[10]) {

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
