/**
 * Description: This program executes multiple commands.
 * Author names: Talia Syed, Yinglin Tan
 * Author emails: talia.syed@sjsu.edu, yinglin.tan@sjsu.edu
 * Last modified date: 3/11/23
 * Creation date: 3/10/23
 **/

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_CHAR 100
#define MAX_LEN 30


int main(int argc, char *argv[]){
    bool file_exist = 0;
    FILE* cmd_fp; //declare file pointer

    //If user does not give exactly 1 input, print warning and exit as 2
    if (argc == 2){
        cmd_fp  = fopen(argv[1], "r");
        file_exist = 1;
    }

    //if file doesn't exist, print error message and exit as 1
    if (cmd_fp == NULL){
        printf("Error: cannot open file.\n");
        exit(1);
    }

    //go to end of file to check if it is empty; if not empty, go back to top
    if (cmd_fp != NULL) {
        fseek(cmd_fp, 0, SEEK_END); //go to end of file
        //if file is empty, print message and exit as 0
        if (ftell(cmd_fp) == 0){
            printf("File is empty!\n");
            exit(0);
        }
        fseek(cmd_fp, 0, SEEK_SET); //go back to top of file
    }

    int status; //declare status
    pid_t child; //declare child pid
    char* loop;
    //char commands[MAX_LEN] = {0}; //array to store commands
    char current_line[MAX_LEN]; //array to store current line read
    int line_count = 0; //counter for line count
    int cmd_count = 0;

    if(file_exist){
        loop = fgets(current_line, MAX_LEN, cmd_fp);
    }
    else{
        loop = fgets(current_line, MAX_LEN, stdin);
    }


    while(loop != NULL && line_count < MAX_CHAR){
        //convert to C string
        if (current_line[strlen(current_line) - 1] == '\n'){
            current_line[strlen(current_line) - 1] = '\0';
        }

        cmd_count++;

        child = fork();

        //exit if child process did not spawn
        if(child < 0){
            fprintf(stderr, "Fork failed");
            exit(1);
        }
        //child process
        else if (child == 0){
            //split current line into parts by words
            char *argument[MAX_LEN + 1] = {0};
            char *word = strtok(current_line, " ");
            int counter = 0;

            //separate each word based on space
            while(word != NULL && counter < MAX_LEN - 1){
                argument[counter] = word;
                counter++;
                word = strtok(NULL, " ");
            }
            argument[counter] = NULL;

            for(int k = 0; k < 10;k++){
                printf("in argument: %s\n", argument[k]);
            }

            fprintf(stdout,"Starting command %d: child %d pid of parent %d\n", cmd_count, getpid(), getppid());

            char output_file[MAX_LEN]; // array to hold stdout
            char error_file[MAX_LEN]; // array to hold stderr

            //push the logs to their respective files
            sprintf(output_file, "%d.out", (int) getpid());
            sprintf(error_file, "%d.err", (int) getpid());

            //open log files
            int fd_1 = open(output_file, O_RDWR | O_CREAT | O_APPEND, 0777);
            int fd_2 = open(error_file, O_RDWR | O_CREAT | O_APPEND, 0777);

            //send fd_1 to PID.out file and fd_2 to PID.err for the PID
            dup2(fd_1, 1);
            dup2(fd_2, 2);

            printf("log file done\n");
            fflush(stdout);

            //check if execvp ran properly
            if(execvp(argument[0], argument) == -1){
                perror(argument[0]);
                exit(2);
            }

        }

        //parent process
        while((child = wait(&status)) > 0){
            printf("start of parent\n");

            char output_file[MAX_LEN] = {0}; // array to hold stdout
            char error_file[MAX_LEN] = {0}; // array to hold stderr

            //push the logs to their respective files
            sprintf(output_file, "%d.out", child);
            sprintf(error_file, "%d.err", child);

            printf("sprintf done\n");

            //open log files
            int fd_1 = open(output_file, O_RDWR | O_CREAT | O_APPEND, 0777);
            dup2(fd_1, 1);

            fflush(stdout);

            int fd_2 = open(error_file, O_RDWR | O_CREAT | O_APPEND, 0777);
            dup2(fd_2, 2);

            printf("open log files done\n");

            //send fd_1 to PID.out file and fd_2 to PID.err for the PID

            printf("parent: log file done\n");

            //if process exited normally
            if (WIFEXITED(status)) {
                fprintf(stdout, "Finished child %d pid of parent %d\n", child, (int) getpid());
                fflush(stdout); //clear stdout
                fprintf(stderr, "Exited with exitcode = %d\n", WEXITSTATUS(status));
            }
            //if process killed
            else if (WIFSIGNALED(status)) {
                fprintf(stderr, "Killed with signal %d\n", WTERMSIG(status));
            }
            fclose(cmd_fp);
/*
            if(current_line){
                free(current_line);
            }*/
        }
        line_count++;

    }


    //notes:
    //how to print things in the correct order?
    //how to keep the loop going until last command?


    return 0;
}
