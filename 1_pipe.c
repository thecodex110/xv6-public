#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int getwords(char *line, char *words[])
{
	int n = 0;
	char *t = strtok(line, " \t\n");

	while(t != NULL && n < 63){
		words[n] = t;
		n++;
		t = strtok(NULL, " \t\n");
	}
	words[n] = NULL;
	return n;
}

void redirect(char *words[])
{
	int i, j = 0, fd;

	for(i = 0; words[i] != NULL; i++){
		if(strcmp(words[i], "<") == 0){
			if(words[i + 1] == NULL){
				printf("Missing filename\n");
				exit(1);
			}
			fd = open(words[i + 1], O_RDONLY);
			if(fd == -1){
				perror(words[i + 1]);
				exit(1);
			}
			dup2(fd, 0);
			close(fd);
			i++;
		}else if(strcmp(words[i], ">") == 0){
			if(words[i + 1] == NULL){
				printf("Missing filename\n");
				exit(1);
			}
			fd = open(words[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if(fd == -1){
				perror(words[i + 1]);
				exit(1);
			}
			dup2(fd, 1);
			close(fd);
			i++;
		}else{
			words[j] = words[i];
			j++;
		}
	}
	words[j] = NULL;
}

int runcmd(char *words[], int in, int out, int fd[2])
{
	int pid = fork();

	if(pid == -1){
		perror("fork");
		return -1;
	}
	if(pid == 0){
		if(in != -1)
			dup2(in, 0);
		if(out != -1)
			dup2(out, 1);
		if(fd != NULL){
			close(fd[0]);
			close(fd[1]);
		}
		redirect(words);
		if(words[0] == NULL)
			exit(1);
		execvp(words[0], words);
		perror(words[0]);
		exit(1);
	}
	return pid;
}

int main()
{
	char line[1024];
	char cwd[1024];
	char *left[64];
	char *right[64];
	char *bar;
	int fd[2];
	int n1, n2, p1, p2;

	while(1){
		if(getcwd(cwd, sizeof(cwd)) == NULL)
			strcpy(cwd, "?");
		printf("%s> ", cwd);
		fflush(stdout);

		if(fgets(line, sizeof(line), stdin) == NULL){
			printf("\n");
			break;
		}

		bar = strchr(line, '|');

		if(bar != NULL && strchr(bar + 1, '|') != NULL){
			printf("Only one pipe is allowed\n");
			continue;
		}

		if(bar == NULL){
			n1 = getwords(line, left);
			if(n1 == 0)
				continue;
			if(strcmp(left[0], "exit") == 0)
				break;
			if(strcmp(left[0], "cd") == 0){
				if(n1 < 2){
					printf("cd: missing operand\n");
					continue;
				}
				if(chdir(left[1]) == -1)
					perror("cd");
				continue;
			}
			p1 = runcmd(left, -1, -1, NULL);
			if(p1 > 0)
				waitpid(p1, NULL, 0);
		}else{
			*bar = '\0';
			n1 = getwords(line, left);
			n2 = getwords(bar + 1, right);
			if(n1 == 0 || n2 == 0){
				printf("Command missing around pipe\n");
				continue;
			}
			if(pipe(fd) == -1){
				perror("pipe");
				continue;
			}

			p1 = runcmd(left, -1, fd[1], fd);
			p2 = runcmd(right, fd[0], -1, fd);

			close(fd[0]);
			close(fd[1]);

			if(p1 > 0)
				waitpid(p1, NULL, 0);
			if(p2 > 0)
				waitpid(p2, NULL, 0);
		}
	}
	return 0;
}
