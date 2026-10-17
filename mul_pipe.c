#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
char mypath[1024] = "/usr/bin:/bin";
void readline(char *arr, int len){
	char ch = '\0';
	int i = 0;
	while(ch != '\n'){
		ch = getchar();
		if(ch == EOF){
			arr[i] = '\n';
			putchar('\n');
			exit(1);
		}
		arr[i] = ch;
		i++;
	}
	return;
}

void getwords(char *newbuf[], int *len, char *line){
	int i = 0, j, k = 0;
	char buf[1024];
	int buflen = 0;
	while(line[i] == ' ' || line[i] == '\t'){
		i++;
	}
	while(line[i] != '\n'){
		if(line[i] == ' ' || line[i] == '\t'){
			buf[buflen] = '\0';
			buflen = 0;
			newbuf[k] = malloc(1 + strlen(buf));
			strcpy(newbuf[k], buf);
			k++;
			while(line[i] == ' ' || line[i] == '\t'){
				i++;	
			}
		}else{
			buf[buflen] = line[i];
			buflen++;
			i++;
		}
	}
	if(buflen != 0){
		buf[buflen] = '\0';
                buflen = 0;
                newbuf[k] = malloc(1 + strlen(buf));
                strcpy(newbuf[k], buf);
                k++;
	}
	newbuf[k] = NULL;
	(*len) = k;
	return;
}
int main(){
	int pid;
	char buf[1024];
	char buf2[1024];
	char temp[1024];
	char tempmypath[1024];
	char actualcmd[1024];
	int fd;
	int i, j;
	char *arrnow[1024];
	while(1){
		int handled = 0;
		char *str = getcwd(buf, 1024);
		printf("%s>", str);
		readline(buf2, 1024);
		char *newbuf[1024];
		int len = 0;
		getwords(newbuf, &len, buf2);
		if(len == 0){
			continue;
		}
		strcat(str, "/");
		if(strcmp(newbuf[0], "exit") == 0){
			return 0;	
		}
		if(newbuf[0][0] == 'P' && newbuf[0][1] == 'A' && newbuf[0][2] == 'T' && newbuf[0][3] == 'H' && newbuf[0][4] == '='){
			i = 5;
			while(newbuf[0][i] != '\0' ){
				mypath[i - 5] = newbuf[0][i];
				i++;
			}
			mypath[i - 5] = '\0';
			continue;
		}
		if(strcmp(newbuf[0], "cd") == 0){
			if (len < 2) {
				printf("cd: missing operand\n");
				continue;
			}
			if(newbuf[1][0] == '.' && newbuf[1][1] == '/' || newbuf[1][0] != '/'){
				strcpy(temp, newbuf[1]);
				newbuf[1] = realloc(newbuf[1], strlen(str) + strlen(newbuf[1]) + 1);
				strcat(str, temp);
				strcpy(newbuf[1], str);
			}
			if(chdir(newbuf[1]) == -1){
				perror("cd");
			}
			continue;
		}
		int k = 0, l = 0, count = 0;
		while(k < len){
			if(strcmp(newbuf[k], "|") == 0){
				count++;
			}
			k++;
		}
		int  end = count;
		k = 0;
		int fdp[count][2];
		while(k < count){
			pipe(fdp[k]);
			k++;
		}
		k = 0;
		count = 0;
		while(1){	
		if(newbuf[k][0] == '.' && newbuf[k][1] == '/'){
			strcpy(temp, newbuf[0]);
			newbuf[k] = realloc(newbuf[k], strlen(str) + strlen(newbuf[k]) + 1);
			strcat(str, temp);
			strcpy(newbuf[k], str);
		}else if(newbuf[k][0] != '/'){
			i = 0;
			strcpy(actualcmd, newbuf[k]);
			pid = fork();
			if (pid == -1) {
				perror("fork");
				continue;
			}
			if(pid == 0){
				if(end == 0){
					;
				}else if(count == 0){
					close(fdp[count][0]);
					close(1);
					dup(fdp[count][1]);	//write to write end
					close(fdp[count][1]);
				}else if(count == end){
					close(fdp[end - 1][1]);
					close(0);
					dup(fdp[end - 1][0]);
					close(fdp[end - 1][0]);
				}else{
					close(0);
					dup(fdp[count - 1][0]);
					close(fdp[count - 1][0]);
					close(1);
					dup(fdp[count][1]);
					close(fdp[count][1]);
				}
				i = 0;
				while(i < len){
					if(strcmp(newbuf[i], "<") == 0){
						if(newbuf[i + 1] == NULL){
    							printf("Missing filename\n");
					    		exit(EXIT_FAILURE);
						}
						close(0);
						fd = open(newbuf[i + 1], O_RDONLY);
						if(fd == -1){
							perror(newbuf[i + 1]);
							exit(EXIT_FAILURE);
						}
						newbuf[i] = NULL;
					}else if(strcmp(newbuf[i], ">") == 0){
						if(newbuf[i + 1] == NULL){
							printf("Missing filename\n");
							exit(EXIT_FAILURE);
                        }
						close(1);
						fd = open(newbuf[i + 1], O_WRONLY | O_CREAT);
						if(fd == -1){
							perror(newbuf[i + 1]);
							exit(EXIT_FAILURE);
						}
						newbuf[i] = NULL;
					}
					i++;
				}
				l = 0;
                int idx = k;
                while(newbuf[idx] != NULL && strcmp(newbuf[idx], "|") != 0){
                    arrnow[l] = malloc(strlen(newbuf[idx]) + 1);
                    strcpy(arrnow[l], newbuf[idx]);
                    l++;
                    idx++;
                }
                arrnow[l] = NULL;
				i = 0;
           		while(mypath[i] != '\0'){
					j = 0;
					while(mypath[i] != ':' && mypath[i] != '\0'){
						tempmypath[j] = mypath[i];
						i++;
						j++;
					}
					tempmypath[j] = '\0';
					strcpy(temp, actualcmd);
                        		newbuf[k] = realloc(newbuf[k], strlen(tempmypath) + strlen(newbuf[0]) + 2);
					strcat(tempmypath, "/");
					arrnow[0] = realloc(arrnow[0], strlen(tempmypath) + 1);
                    strcat(tempmypath, temp);
                    strcpy(arrnow[0], tempmypath);
                        		strcpy(newbuf[k], tempmypath);
					execv(arrnow[0], arrnow);
					strcpy(newbuf[k], actualcmd);
					if(mypath[i] == ':'){
						i++;
					}
				}
				perror(newbuf[0]);
				exit(EXIT_FAILURE);
			}else{
				if(end != 0){
        			if(count == 0){
            			close(fdp[count][1]);         
        			}else if(count == end){
        		    	close(fdp[end - 1][0]);        
        			}else{
            			close(fdp[count - 1][0]);
            			close(fdp[count][1]);
        			}
				}
				wait(0);
				handled = 1;
			}
		}
		if(handled != 1){
		pid = fork();
		if (pid == -1) {
    			perror("fork");
    			continue;
		}
		if(pid == 0){
			if(end == 0){
				;
			}else if(count == 0){
				close(fdp[count][0]);
				close(1);
				dup(fdp[count][1]);
				close(fdp[count][1]);
			}else if(count == end){
				close(fdp[end - 1][1]);
				close(0);
				dup(fdp[end - 1][0]);
				close(fdp[end - 1][0]);
			}else{
				close(0);
				dup(fdp[count - 1][0]);
				close(fdp[count - 1][0]);
				close(1);
				dup(fdp[count][1]);
				close(fdp[count][1]);
			}
			i = 0;
			while(i < len){
				if(strcmp(newbuf[i], "<") == 0){
					if(newbuf[i + 1] == NULL){
						printf("Missing filename\n");
						exit(EXIT_FAILURE);
					}
					close(0);
					fd = open(newbuf[i + 1], O_RDONLY);
					if(fd == -1){
						perror(newbuf[i + 1]);
						exit(EXIT_FAILURE);
					}
					newbuf[i] = NULL;
				}else if(strcmp(newbuf[i], ">") == 0){
					if(newbuf[i + 1] == NULL){
						printf("Missing filename\n");
						exit(EXIT_FAILURE);
					}
					close(1);
					fd = open(newbuf[i + 1], O_WRONLY | O_CREAT);
					if(fd == -1){
						perror(newbuf[i + 1]);
						exit(EXIT_FAILURE);
					}
					newbuf[i] = NULL;
				}
				i++;
			}
			l = 0;
			while(newbuf[k] != NULL && strcmp(newbuf[k], "|") != 0){
				arrnow[l] = malloc(strlen(newbuf[k]) + 1);
				strcpy(arrnow[l], newbuf[k]);
				l++;
				k++;
			}
			arrnow[l] = NULL;
			execv(arrnow[0], arrnow);
			perror("execv");
			exit(EXIT_FAILURE);
		}else{
			wait(0);
		}
		}
		while(newbuf[k] != NULL && strcmp(newbuf[k], "|") != 0){
			k++;
		}
		if(newbuf[k] != NULL && strcmp(newbuf[k], "|") == 0){
    		k++;
		}
		count++;
		if(newbuf[k] == NULL){
			k = 0;
			while(k < end){
				close(fdp[k][0]);
				close(fdp[k][1]);
				k++;
			}
			break;
		}
		}
	}
	return 0;
}
