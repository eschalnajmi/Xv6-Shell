#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int main(int argc, char *argv[]){
        int loop = 0;
        int iteration = 0;

        while(loop == 0){
                iteration++;

                // get user's command line input
                char buf[150];
                char bufnospaces[150];

                printf(">>> ");
                gets(buf, 150);

                int words = 0;
                int* wordlen = (int*) malloc(100 * sizeof(int));
                int* wordlen_copy = wordlen;

                int characters = 0;
                int bufnospacesindex = 0;
                int firstchar = 0;

                // check for spaces in front of command
                for(int i = 0; i < strlen(buf); i++){
                        if(buf[i] != ' '){
                                firstchar = i;
                                break;
                        }
                }

                // loop through character array and find how many words are input
                for(int i = firstchar; i < strlen(buf); i++){
                        if(buf[i] == ' ' && buf[i+1] != ' ' && i != 0){
                                if(iteration == 1){
                                        wordlen_copy[words] = characters;
                                        words++;
                                } else {
                                        wordlen[words] = characters;
                                        words++;
                                }
                                characters = 0;
                                bufnospaces[bufnospacesindex] = buf[i];
                                bufnospacesindex++;
                        } else if(buf[i] == ' '){
                                continue;
                        } else{
                                bufnospaces[bufnospacesindex] = buf[i];
                                bufnospacesindex++;
                                characters++;
                        }
                }
                if(iteration == 1){
                    wordlen_copy[words] = (characters - 1);
                } else{
                    wordlen[words] = characters - 1;
                }
                words++;

                // create string array dynamically
                char **inputs = malloc(words * sizeof(char*));
                for(int i = 0; i < words; i++){
                        inputs[i] = malloc(16*sizeof(char));
                }

                int index = 0;

                // get each individual word from input
                for(int i = 0; i < words; i++){
                        if(iteration == 1){
                                memcpy(inputs[i], &bufnospaces[index], wordlen_copy[i]);
                                index += wordlen_copy[i] + 1;
                        } else{
                                memcpy(inputs[i], &bufnospaces[index], wordlen[i]);
                                index += wordlen[i] + 1;
                        }
                }

                // if input is exit then make a special case
                if(strcmp(inputs[0], "exit") == 0){
                        break;
                }


                // if input is cd make a special case 
                if(strcmp(inputs[0], "cd") == 0){
                        if(chdir(inputs[1]) == -1){
                                printf("directory non existant\n");
                        }
                        continue;
                }

                int fdinput = 0;
                int fdoutput = 1;
                int pipes = 0;
                int p[2];
                for(int i = 0; i < words - 1; i++){
                        if(strcmp(inputs[i], ">") == 0){
                                fdoutput = open(inputs[i+1], O_CREATE|O_WRONLY);
                                inputs[i] = 0;
                                i++;
                        } else if(strcmp(inputs[i], "<") == 0){
                                fdinput = open(inputs[i+1], O_RDONLY);

                                inputs[i] = 0;
                                i++;
                        } else if(strcmp(inputs[i], "<") == 0){
                                fdinput = open(inputs[i+1], O_RDONLY);
                                inputs[i] = 0;
                                i++;
                        } else if(strcmp(inputs[i], "|") == 0){
                                pipes = 1;
                                pipe(p);
                                //inputs[i] = 0;        
                        }
                }
                if(fdoutput < 0 || fdinput < 0){
                        printf("failed to open file\n");
                        break;
                }

                // execute the command input
                int pid = fork();
                if(pid == 0 && fdoutput != 1) {
                        close(1);
                        dup(fdoutput);
                        close(fdoutput);
                        exec(inputs[0], inputs);
                        exit(0);
                } else if(fdinput != 0 && pid == 0){
                        close(0);
                        dup(fdinput);
                        close(fdinput);
                        exec(inputs[0], inputs);
                        exit(0);
                } else if(pid == 0){
                        if(pipes == 1){
                                close(0);
                                dup(p[0]);
                                close(p[0]);
                                close(p[1]);
                        }
                        exec(inputs[0], inputs);
                        exit(0);
                } else if(pid < 0) {
                        printf("fork error\n");
                        exit(1);
                } else {
                        pid = wait((int*) 0);
                        if(pipes == 1){
                                close(p[0]);
                                close(p[1]);
                        }
                }

        }
        exit(0);
}
