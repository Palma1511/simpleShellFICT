#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<string.h>

#define FICT_RL_BUFSIZE 1024
#define FICT_TOK_DELIM " \t\r\n\a"
#define FICT_TOK_BUFSIZE 256

int fictCd(char **args);
int fictHelp(char **args);
int fictExit(char **args);

char *builtinStr[] = {"cd", "help", "exit"};

int (*builtinFunc[])(char **) = { &fictCd, &fictHelp, &fictExit};

int fictNumBuiltin() {
	return sizeof(builtinStr) / sizeof(char *);
}

int fictCd(char **args) {
	if(args[1] == NULL) {
		fprintf(stderr, "fict: expected argument to \"cd\"\n");
	} else {
		if(chdir(args[1]) != 0) {
			perror("fict");
		}
	}

	return 1;
}

int fictHelp(char **args) {
	int i;
	printf("Antonenko Kirill FICT shell!\n");
	for(i = 0; i < fictNumBuiltin(); i++) {
		printf("\t%s\n", builtinStr[i]);
	}
	return 1;
}

int fictExit(char **args) {
	return 0;
}



char *fictReadLine() {
	char *line = NULL;
	ssize_t bufsize = 0;

	if(getline(&line, &bufsize, stdin) == -1) {
		if(feof(stdin)) {
			exit(EXIT_SUCCESS);
		} else {
			perror("readline");
			exit(EXIT_FAILURE);
		}
	}

	return line;
}

char **fictGetToken(char *line) {
	int bufsize = FICT_TOK_BUFSIZE;
	int position = 0;
	char **tokens = malloc(bufsize * sizeof(char *));
	char *token;

	if(!tokens) {
		fprintf(stderr, "fict: allocator error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, FICT_TOK_DELIM);
	while(token != NULL) {
		tokens[position] = token;
		position++;

		if(position >= bufsize) {
			bufsize += FICT_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if(!tokens) {
				fprintf(stderr, "fict: allocator error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, FICT_TOK_DELIM);
	}

	tokens[position] = NULL;
	return tokens;
}

int fictLaunch(char **args) {
	pid_t pid, wpid;
	int status;

	pid = fork();
	if(pid == 0) {
		if(execvp(args[0], args) == -1) {
			perror("fict");
		}
		exit(EXIT_FAILURE);
	} else if(pid < 0) {
		perror("fict");
	} else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && WIFSIGNALED(status));
	}

	return 1;
}

int fictExecute(char **args) {
	int i;
	if(args[0] == NULL) {
		return 1;
	}

	for(i = 0; i < fictNumBuiltin(); i++){
		if(strcmp(args[0], builtinStr[i]) == 0) {
			return (*builtinFunc[i])(args);
		}
	}

	return fictLaunch(args);
}

void fictLoop() {
	char *line;
	char **args;
	int status;

	do {
		printf(">>> ");
		line = fictReadLine();
		args = fictGetToken(line);
		status = fictExecute(args);

		free(line);
		free(args);
	} while(status);
}



int main(int argc, char **argv){
	
	fictLoop();

	return EXIT_SUCCESS;
}
