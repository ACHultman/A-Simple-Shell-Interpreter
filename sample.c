#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>





int main(){
	
	
	char prompt[1000];
	char cwd[256];
	char host[256];
	char* user;
	host[255] = "\0";
	cwd[255] = "\0";
	
	user = getlogin();	
	gethostname(host, sizeof(host));
	getcwd(cwd, sizeof(cwd));	
	
	strcat(prompt, "SSI: ");
	strcat(prompt, user);
	strcat(prompt, "@");
	strcat(prompt, host);
	strcat(prompt, ": ");
	strcat(prompt, cwd);
	
	//SSI: %s@%s: %s ", user, host, cwd

	int bailout = 0;
	while (!bailout) {

		char* reply = readline(prompt);
		/* Note that readline strips away the final \n */
		/* For Perl junkies, readline automatically chomps the line read */

		if (!strcmp(reply, "bye")) {
			bailout = 1;
		} 
		else if (!strcmp(reply, "ls")) {
			printf("\nYou said: %s\n\n", reply);
		}
		else{
			printf("Command not found");
		}
	
		free(reply);
	}
	printf("Bye Bye\n");
}

