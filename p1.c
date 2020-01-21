/*
 * Author: Adam Hultman
 * Student Number: V00900702
 * Course: CSC 360
 * Section: A01, T02
 *
 * Assignment: Programming Assignment 1
 *          A Simple Shell Interpreter
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <sys/wait.h>


// Structure for a background process node.
struct bg_pro{
    pid_t pid;
    char command[1024];
    struct bg_pro* next;
};
struct bg_pro* bg_list_head;


/*
 * @Parameters: char array prompt.
 * @Returns: Filled char array prompt.
 * @Local Function Calls: NONE
 *
 * Adds user, host, and cwd to the prompt.
 */
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


/*
 * @Parameters: NONE
 * @Returns: int count
 * @Local Function Calls: NONE
 *
 * Counts and returns the number of processes in the background process linked list.
 */
int  bg_process_count(){
    int count = 0;
    struct bg_pro current = *bg_list_head;
    while(current.next != NULL){
        count++;
        current = *current.next;
    }
    return count;
}


/*
 * @Parameters: PID to be appended, CMD to be appended.
 * @Returns: NONE
 * @Local Function Calls: NONE
 *
 * Appends given PID and CMD to the background process linked list.
 */
void bg_list_append(pid_t p, char *args[100]){
    struct bg_pro *new_pro = malloc(sizeof(struct bg_pro)); //Allocate memory for new node.
    char new_args[1000];
    strcpy(new_args, args[0]);

    //Concatenates all non-empty arguments.
    int i = 1;
    while(args[i] != NULL){
        strcat(new_args, " ");
        strcat(new_args, args[i]);
        i++;
    }

    // Sets up data of new node.
    new_pro->pid = p;
    strcpy(new_pro->command, new_args);
    new_pro->next = NULL;

    if(bg_list_head == NULL){ //If list empty:
        bg_list_head = new_pro;
    }
    else{
        struct bg_pro *current = bg_list_head;
        while(current->next != NULL){ // Iterates to end of list.
            current = current->next;
        }
        current->next = new_pro; // Appends new node.
    }
}


/*
 * @Parameters: NONE
 * @Returns: NONE
 * @Local Function Calls: NONE
 *
 * Checks each linked list struct if its process has terminated.
 * If terminated, then removes from bg_list.
 */
void kill_bg_zombies(){
    if(bg_process_count > 0){
        pid_t ter = waitpid(0, NULL, WNOHANG);
        while(ter > 0){
            if(bg_list_head->pid == ter){
                printf("HEAD %d %s has terminated.\n", bg_list_head->pid, bg_list_head->command);
                bg_list_head = bg_list_head->next;
            }
            else{
                struct bg_pro *current = bg_list_head;
                while(current->next->pid != ter){
                    current = current->next;
                }
                printf("%d %s  has terminated.\n", current->next->pid, current->next->command);
                current->next = current->next->next;
            }
            ter = waitpid(0,NULL,WNOHANG);
        }
    }
}


/*
 * @Parameters: NONE
 * @Returns: NONE
 * @Local Function Calls: kill_bg_zombies()
 *
 * Prints all non-empty nodes of the background process linked list.
 */
void print_bglist() {
    int count = 0;
    kill_bg_zombies(); // Updates list
    struct bg_pro *current = bg_list_head;
    if(bg_list_head == NULL){
        printf("Total Background jobs: %d\n", count); //If no head, return count (0)
        return;
    }
    count++;
    // Iterate to end of list and print each entry.
    while(current->next != NULL){
        count++;
        printf("%d: ", current->pid);
        printf("%s\n", current->command);
        current = current->next;
    }
    //Print last entry.
    printf("%d: ", current->pid);
    printf("%s\n", current->command);
    printf("Total Background jobs: %d\n", count);
}

/*
 * @Parameters: char *args[] (the command)
 * @Returns: NONE
 * @Local Function Calls: NONE
 *
 * Executes a 'cd' command by invoking chdir().
 */
void process_cd(char *args[100]) {
    char cwd[256];
    cwd[255] = '\0';
    getcwd(cwd, sizeof(cwd));
    if(args[1] == NULL || *args[1] == '~'){ // If command to HOME DIR
        chdir(getenv("HOME"));
    }
    chdir(args[1]); //Executes cd command with path.

}


/*
 * @Parameters: *cmd, *args[]
 * @Returns: NONE
 * @Local Function Calls: NONE
 *
 * Tries to execute any BASH command it receives with fork() and execvp().
 */
void execute_program(char* cmd, char *args[100]){
    pid_t pid;
    int status;

    if((pid = fork()) < 0){ // Forks process and checks if error.
        printf("ERROR: Forked failed.\n\n");
        exit(1);
    }
    else if (pid == 0){ // If child process.
        if((execvp(cmd, args) < 0)){
            printf("ERROR: Execvp failed.\n\n");
        }
    }
    else{ // If parent process.
        while(wait(&status) == 0)
            ;
    }
}


/*
 * @Parameters: *cmd, *args[]
 * @Returns: NONE
 * @Local Function Calls: bg_list_append()
 *
 * Executes given command in a background process, appends it to list.
 */
void execute_bg_program(char* cmd, char *args[100]){
    pid_t p;

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
        bg_list_append(p, args);
    }
}


/*
 * @Parameters: *args[]
 * @Returns: NONE
 * @Local Function Calls: execute_bg_program()
 *
 * Checks if command after 'bg', then executes with trimmed arguments.
 */
void process_bg(char *args[100]) {
    if(args[1]==NULL){
        printf("Command required after 'bg'.\n");
        return;
    }
    execute_bg_program(args[1], &args[1]);
}


/*
 * MAIN FUNCTION
 * @Parameters: NONE
 * @Local Function Calls: create_prompt(), process_cd(), process_bg(), print_bglist(),
 * execute_program(), kill_bg_zombies()
 */
int main() {
    bg_list_head = malloc(sizeof(struct bg_pro)); //Allocates memory for background process linked list head.
    bg_list_head = NULL;
    int bailout = 0;
    while (!bailout) { // Loops while user does not enter "quit"
        char prompt[1000] = "";
        char *reply = readline(create_prompt(prompt)); // Sets reply to prompt.
        char *argsv[100];
        int i = 0;
        char *token = strtok(reply, " "); // Tokenize prompt by space.
        if (token == NULL){
            continue;
        }
        for (i = 0; token != NULL; i++) { //Continues to tokenize and add to *argsv[]
            argsv[i] = token;
            token = strtok(NULL, " ");
        }
        argsv[i] = NULL; //Makes sure end of argsv is NULL

        //Check whether the argument is one of the keywords, otherwise execute via BASH
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
            kill_bg_zombies(); //Checks if any background process is terminated.
            free(reply);
    } //end of while loop

        printf("Quitting...\n");
        free(bg_list_head);
        exit(0);
}//end of MAIN


