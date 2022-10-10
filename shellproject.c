#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

// function that throws error
void throw_error()
{
    char error_message[30] = "An error has occurred\n";
    write(STDOUT_FILENO, error_message, strlen(error_message));
}

// boolean that accounts for empty space inputs!
int is_empty(char *string)
{
    if (!string) {
        return 1;
    }
    for (int i = 0; i < strlen(string); i++)
    {
        if (!(isspace(string[i]) || 
              string[i] == '\0' ||
              string == NULL)) {
            return 0;
        }
    }
    return 1;
}

// boolean that checks if file is empty
int file_empty(FILE* fd) {
    int ret = 0;
    int c = fgetc(fd);
    if (c == EOF) {
        ret = 1;
    } else {
        ungetc(c, fd);
    }
    return ret;
}

// function that finds size of file with fstat
int file_size(int fd) {
    struct stat *filestat = (struct stat *) malloc (sizeof(struct stat));
    fstat(fd, filestat);
    return filestat->st_size;
}

// check if pwd has arguments after it (it should not)
int pwd_check(char **input)
{
    if (strcmp(input[0],"pwd") == 0) {
        if (input[1] != NULL) {
            return 1;
        }
    }
    return 0;
}

// function that counts number of commands by character
int num_commands(char *command, char search)
{
    int num = 0;
    for(int i = 0; i <= strlen(command); i++) {
  		if(command[i] == search) {
  			num++;
 		}
	}
    return num+1;
}

// function that counts number of arguments in command_array
int num_args(char *single_command)
{
    int str_len = strlen(single_command);
    int num = 0;
    if (single_command[0] != ' ') {
        num++;
    }

    for(int i = 0; i < (str_len - 1); i++) {
        if (isspace(single_command[i]) && !(isspace(single_command[i+1]))) {
            num++;
        } 
    }
    return num;
}

// function that removes white spaces from strings
char *trim(char *str)
{
  char *end;

  while (isspace((unsigned char)*str)) {
      str++;
  }

  if (*str == 0) {
    return str;
  }

  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) {
      end--;
  }

  str[strlen(str)] = '\0';

  end[1] = '\0';
  return str;
}

// read lines based on number of commands
void read_line(char *command, char **commands_array, int len)
{
    char *strings = strtok(command, ";");
    
    int i = 0;
    while (strings != NULL) {
        char *trimmed = trim(strings);
        // if there is nothing in the string, put newline character
        if(is_empty(strings)) {
            commands_array[i++] = "\n";
        } else {
            commands_array[i++] = trimmed;
        }
        strings = strtok(NULL, ";");
    }
}

// check type of redirection
int redir_type(char *command)
{
    int ret = 0;
    int i = 0;
    if (!command) {
        return ret;
    }
    while (command[i] != '\0') {
        if (command[i] == '>') {
            ret++;
            if (command[i+1] == '+') {
                ret++;
            }
        }
        i++;
    }
    if (ret > 2) {
        return -1;
    }
    return ret;
}

// function that parses arguments with redirection given one command
// then does the redirection
void parse_redirect(char *command, int redir)
{
    int fileStat = -1;
    // parsing part
    char **args = (char **) malloc (sizeof(char*)*2);
    char *tok = strtok(command, ">");
    // check if command is ONLY '>'
    if (!tok) {
        throw_error();
        return;
    }
    int i = 0; 
    while (tok != NULL) {
        char *trimmed = trim(tok);
        args[i] = trimmed;
        // use for debugging
        //printf("trimmed: %s\n", args[i]);
        // check if there is extra '>' in commmand line
        if (strchr(args[i], '>')) {
            throw_error();
            return;
        }
        tok = strtok(NULL, ";");
        i++;
    }
    if (redir == 2) {
        // use for debugging
        if (!args[1]) {
            throw_error();
            return;
        }
        //printf("arg check: %s\n", args[1]);
        args[1] = trim(args[1]+1);
        // use for debugging
        //printf("trimmed: %s\n", args[1]);
    }

    // this part actually does the redirection

    // part 1: parsing the two sides of the '>'/'>+'
    // FUTURE: account for edge cases (e.g. just '>')
    int input_arg_len = num_args(args[0]);
    // use for debugging
    //printf("number of args in input are: %d\n", input_arg_len);
    char **input = (char **) malloc (sizeof(char*)*(input_arg_len+1));
    char *output = args[1];
    int argcount = 0;
    char *split;
    while ((split = strsep(&args[0]," ")) != NULL) {
        char *trimmed = trim(split);
        input[argcount] = trimmed;
        // use for debugging
        //printf("redirect: input[%d] is \"%s\"\n", argcount, input[argcount]);
        argcount++;
    }
    input[input_arg_len] = NULL;
    // use for debugging
    //printf("redirect: output is \"%s\"\n", output);

    // check if input[0] is 'cd' or 'pwd' or 'exit' (throw error)!
    if (strcmp(input[0], "cd") == 0 || 
        strcmp(input[0], "pwd") == 0 ||
        strcmp(input[0], "exit") == 0) {
        throw_error();
        return;
    }

    // check if output exists and can be accessed
    if (!output) {
        throw_error();
        return;
    }

    // check if file already exists (BAD FOR NORMAL REDIR)
    fileStat = open(output, O_WRONLY);
    if (fileStat >= 0) {
        if (redir == 1) {
            throw_error();
            return;
        }
    }

    // if REDIR == 2 and file doesn't exist, create file
    if (redir == 2) {
        if (fileStat < 0) {
            fileStat = creat(output, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        }
    }
    
    int fileSize = file_size(fileStat);
    char *chonk = malloc(fileSize);
    //fprintf(stderr,"filesize: %d\n", fileSize);

    if (redir == 2) {
    // if REDIR == 2 and file DOES exist, read into the file
        if (fileStat >= 0) {
            // find out size of buffer
            close(fileStat);
            int file = open(output, O_RDWR | O_CREAT, 0664);
            if (!file) {
                throw_error();
                return;
            }
            read(file, chonk, fileSize);
            close(file);
            fileStat = open(output, O_WRONLY | O_APPEND, 0664);
        }
    }

    // if REDIR == 1 (file doesn't exist), create file
    int stat;
    pid_t forkret = fork();
    if (forkret < 0) {
        throw_error();
        exit(0);
    }
    else if (forkret == 0) {
        //int fd;
        if (redir == 1) {
            fileStat = creat(output, 0644);
        }
        else if (redir == 2) {
            fileStat = open(output, O_RDWR | O_CREAT | O_TRUNC, 0664);
        }
        // check if file path is valid
        if (access(output, F_OK) == 0) {
            dup2(fileStat, STDOUT_FILENO);
            close(fileStat);
            if (execvp(input[0],input) < 0) {
                throw_error();
                exit(0);
            }
        } else {
            throw_error();
            exit(0);
        }
    } else {
        waitpid(forkret, &stat, 0);
        write(fileStat, chonk, fileSize);
    }

}

void exit_command(char *command)
{
    if (strcmp(command, "exit\n") == 0) {
        exit(1);
    }
    else {
        return;
    }
}

void pwd_command(char *command)
{
    char buff[100];
    if (strcmp(command, "pwd\n") == 0) {
        getcwd(buff, 100);
        myPrint(buff);
        myPrint("\n");
    }
    else {
        return;
    }
}

void cd_command(char **input)
{
    if (!(input[1])) {
        char *go = getenv("HOME");
        chdir(go);
        // use for debugging
        //printf("cd-ed to HOME\n");
    } else if (access(input[1],F_OK) == 0) {
        chdir(input[1]);
        // use for debugging
        //printf("cd-ed to %s\n", input[1]);
    } else {
        throw_error();
    }
}

void nonbuiltin_command(char **input)
{
    int stat;
    pid_t forkret = fork();
    if (forkret < 0) {
        throw_error();
        exit(0);
    }
    else if (forkret== 0){
        if (execvp(input[0], input)<0) {
                throw_error();
                exit(0);
        }
        _exit(0);
    }
    else {
        waitpid(forkret, &stat, 0);
    }
}

int main(int argc, char *argv[]) 
{
    if (argc > 2){
        throw_error();
        exit(0);
    }
    char cmd_buff[514];
    char *pinput;
    FILE *fd;

    // batch mode
    if (argc == 2) {
        fd = fopen(argv[1], "read");
        if (fd == NULL) {
            throw_error();
            exit(0);
        }
    }

    while (1) {
        
        
        if (argc == 1) {
            myPrint("myshell> ");
            pinput = fgets(cmd_buff, 514, stdin);
        } else {
            // check if file is empty
            // printf("empty file? %d\n", file_empty(fd));
            if (file_empty(fd)) {
                exit(0);
            }
            pinput = fgets(cmd_buff, 514, fd);
        }

        
        // check if file is not longer than 514 characters
        if (!strchr(pinput, '\n')) {
            myPrint(pinput);
            char c[2] = {0};
            while ((c[0] = getc(fd)) != '\n') {
                myPrint(c);
            }
            myPrint("\n");
            throw_error();
            continue;
        }

        // check if pinput is NULL
        if (!pinput) {
            exit(0);
        }

        if (is_empty(pinput)) {
            continue;
        }
        myPrint(pinput);
        
        // exit command
        exit_command(pinput);

        // number of commands
        int commands_len = num_commands(pinput, ';');
        
        // use for debugging
        //printf("number of commands are: %d.\n", commands_len);

        // malloc array of commands
        char **commands_array = (char **) malloc (sizeof(char*)*commands_len);

        // create array of commands
        read_line(pinput, commands_array, commands_len);

        // number of arguments in a command
        for (int i = 0; i < commands_len; i++) {
            // check if command is empty
            if (is_empty(commands_array[i])) {
                continue;
            }

            // redirection goes here
            int redir = redir_type(commands_array[i]);
            // redir == 1 means >, redir == 2 means >+
            // redir == -1 means faulty redir, redir == 0 means NO REDIR
            if (redir == -1) {
                throw_error();
                continue;
            }
            else if (redir > 0) {
                //printf("redir value is: %d\n", redir);
                parse_redirect(commands_array[i], redir);
            }

            // THIS IS ALL NON REDIRECTION STUFF
            else {
                // note: this length doesn't account for no spaces between '>/>+'
                int length = num_args(commands_array[i]);

                // use for debugging
                //printf("length of command %d is: %d.\n", i, length);

                // check if it is a valid command
                if (is_empty(commands_array[i])) {
                    continue;
                }

                // malloc array for individual arguments in commands
                char **args = (char **) malloc (sizeof(char*)*(length+1));
                char *trimmed = commands_array[i];
                char *str = trimmed;

                // create array for individual arguments in commands
                char *split;
                int arg_count = 0;

                while( (split = strsep(&str," ")) != NULL ) {
                    if (strcmp(split, "") == 0) {
                        continue;
                    }
                    char *trimmed = trim(split);
                    args[arg_count] = trimmed;

                    // use for debugging
                    //printf("arg[%d] is \"%s\"\n", arg_count, args[arg_count]);
                    arg_count++;
                }
                args[arg_count] = NULL;

                // pwd command
                pwd_command(pinput);

                // cd command
                if (strcmp(args[0],"cd") == 0) {
                    if (arg_count > 2) {
                        throw_error();
                        continue;
                    }
                    cd_command(args);
                } 
                // non-built in commands
                else {
                    if (pwd_check(args)){
                        throw_error();
                        continue;
                    }
                    nonbuiltin_command(args);
                }
            }
        }
    }
