nclude <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

extern char **environment;

int main() {
    /* 输入的命令行 */
    char cmd[256];
    /* 命令行拆解成的各部分，以空指针结尾 */
    char *args[128];
    while (1) {
        /* 提示符 */
        printf("# ");
        fflush(stdin);
        fgets(cmd, 256, stdin);
        /* 清理结尾的换行符 */
        int i;
        for (i = 0; cmd[i] != '\n'; i++)
            ;
        cmd[i] = '\0';
        /* 拆解命令行 */
        args[0] = cmd;
        for (i = 0; *args[i]; i++)
            for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++)
                if (*args[i+1] == ' ') {
                    while(1) {
                        if (*args[i+1] == ' ') {
                            *args[i+1] = '\0';
                            args[i+1]++;
                        }
                        else {
                            break;
                        }
                    }
                    break;
                }
        args[i] = NULL;

        /* 没有输入命令 */
        if (!args[0])
            continue;

        /* 内建命令 */
        if (strcmp(args[0], "cd") == 0) {
            if (args[1]) {
               if(chdir(args[1])!=0)
                    printf("%s\n",strerror(errno));
            }
            else {
                printf("ERROR: Please input the dictionary!\n");
            }
            continue;
        }
        if (strcmp(args[0], "pwd") == 0) {
            char wd[4096];
            puts(getcwd(wd, 4096));
            continue;
        }
        if (strcmp(args[0], "exit") == 0)
            return 0;
        
        /*查看环境变量*/
        if (strcmp(args[0], "env") == 0) {
            char **env = environment;
            while (*env) {
                printf("%s\n", env);
                env++;
            }
            continue;
        }
        else if (strcmp(args[1], "-n") == 0) {
            if (unsetenv(args[2] != 0)) {
                printf("%s\n", strerror(errno));
            }
        }

        else {
            char name[128];
            char value[128];
            int flag = 0;
            int j = 0, k = 0;
            name[0] = 0;
            value[0] = 0;
            for (i=0; *(args[1] + i)!='\0'; i++) {
                if(*(args[1]+i)=='=') {
                    break;
                }
            }
            for (j=0; *args[1]+j != '\0'; j++) {
                if(j < i) {
                    name[j] =* (args[1] + j);
                }
                else if(j > i) {
                    value[k] =* (args[1] + j);
                    k++;
                }
            }
            name[i] = '\0';
            value[k] = '\0';
            if (name[0] == 0 || value[0] == 0) {
                printf("Illegal command!\n");
            }
            else {
                if(setenv(name, value, 1) != 0){
                    printf("%s\n", strerror(errno));
                }
            }

        }
        
        /* 外部命令 */
        pid_t pid = fork();//fork — create a child process

        if (pid < 0) {
            printf("%s\n", strerror(errno));
        }

        if (pid == 0) {
            /* 子进程 */
            execvp(args[0], args);
            /* execvp失败 */
            return 255;
        }
        /* 父进程 */
        wait(NULL);
    }
