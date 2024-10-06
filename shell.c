#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

void shell_intro();
void user_input();
void echo_parse(char **tokenArray, int count);
void command_handler(char *input, char **tokenList, int tokenCount);
void help_cmd();
void cd_cmd(char **tokenList);
void mkdir_cmd(char **tokenList);
void exit_cmd();
void history_cmd();
int fork_function(char **tokenArray);
int fork_function_piped();
int fork_function_with_redirect(char **tokenArray, int tokenCount);

char *cmdHistory[100];
int cmdCount = 0;
char *commandList[] = {"help", "cd", "mkdir", "exit", "!!"};

// function: intro to shell
void shell_intro()
{
    printf("Welcome to Brandon's Simple Shell Project\n");
}

// individual command functions
void help_cmd()
{
    printf("Welcome to my SHELL's HELP Command!\n");
    printf("Here is a list of possible commands that you can use: (command: description)\n");
    printf(">> help: \n\tOpen up the help center\n");
    printf(">> cd [cd directory_name]:\n\tEnter into a new directory. No need to enter backslash!\n");
    printf(">> mkdir [mkdir directory_name]: \n\tMake a new directory. No Need to enter backslash!\n");
    printf(">> !!: \n\tUse a previously used command\n");
    printf(">> fork/exec: \n\tfork/exec program processes with redirection and piping of multiple processes\n"); // update and add decriptions
    printf(">> exit: \n\tExit the system\n");
    user_input();
}

void cd_cmd(char **tokenList)
{
    char cwd[1024];

    if (chdir(tokenList[1]) == 0)
    {
        printf("Directory changed!\n");
        getcwd(cwd, sizeof(cwd));
        printf("Current Directory is now: %s\n", cwd);
    }
    else if (chdir(tokenList[2]) == 0)
    {
        printf("Directory changed!\n");
        getcwd(cwd, sizeof(cwd));
        printf("Current Directory is now: %s\n", cwd);
    }
    else
    {
        perror("Command Failed\n");
    }
    user_input();
}

void mkdir_cmd(char **tokenList)
{

    if (mkdir(tokenList[1], 0777) == 0)
    {
        printf("Directory Made!\n");
    }
    else
    {
        perror("Error, try again\n");
    }
    user_input();
}

void exit_cmd()
{
    printf("Thank you for using my shell. Goodbye!\n");
    exit(0);
}

void history_cmd()
{
    if (cmdCount < 2)
    {
        perror("No Previous Commands\n");
        user_input();
        return;
    }

    char *lastCmd = strdup(cmdHistory[cmdCount - 2]);
    char *historyToken[100];
    int count = 0;

    if (lastCmd == NULL)
    {
        perror("Memory Error");
        user_input();
        return;
    }

    char *split = strtok(lastCmd, " ");
    while (split != NULL)
    {
        historyToken[count++] = split;
        split = strtok(NULL, " ");
    }

    if (strcmp(commandList[0], historyToken[0]) == 0)
    {
        help_cmd();
    }
    else if (strcmp(commandList[1], historyToken[0]) == 0)
    {
        cd_cmd(historyToken);
    }
    else if (strcmp(commandList[2], historyToken[0]) == 0)
    {
        mkdir_cmd(historyToken);
    }
    else if (strcmp(commandList[3], historyToken[0]) == 0)
    {
        exit_cmd();
    }
    else if (strcmp(commandList[4], historyToken[0]) == 0)
    {
        history_cmd();
    }
    else
    {
        perror("Not a valid command\n");
        user_input();
    }
}

void track_history(char *cmd)
{
    cmdHistory[cmdCount++] = strdup(cmd);
}

// function: built in command selection
void command_handler(char *input, char **tokenList, int tokenCount)
{

    if (strcmp(commandList[0], input) == 0)
    {
        help_cmd();
    }
    else if (strcmp(commandList[1], input) == 0)
    {
        cd_cmd(tokenList);
    }
    else if (strcmp(commandList[2], input) == 0)
    {
        mkdir_cmd(tokenList);
    }
    else if (strcmp(commandList[3], input) == 0)
    {
        exit_cmd();
    }
    else if (strcmp(commandList[4], input) == 0)
    {
        history_cmd();
    }
    else
    {
        for (int i = 0; i < tokenCount; i++)
        {

            if (strcmp(tokenList[i], ">") == 0 || strcmp(tokenList[i], "<") == 0)
            {
                fork_function_with_redirect(tokenList, tokenCount);
                user_input();
            }
            else if (strcmp(tokenList[i], "|") == 0)
            {
                fork_function_piped(tokenList, tokenCount);
                user_input();
            }
        }
        fork_function(tokenList);
    }
    user_input();
}

// function: fork/exec
int fork_function(char **tokenArray)
{

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return -1;
    }
    else if (pid == 0)
    {
        if (execvp(tokenArray[0], tokenArray) == -1)
        {
            perror("execvp failed");
            exit(1);
        }
    }
    else
    {
        wait(NULL);
    }

    return 0;
}

// fork with redirect
int fork_function_with_redirect(char **tokenArray, int tokenCount)
{
    char *input = NULL;
    char *output = NULL;

    for (int i = 0; i < tokenCount; i++)
    {
        if (strcmp(tokenArray[i], ">") == 0)
        {
            output = tokenArray[i + 1];
            tokenArray[i] = NULL;
            break;
        }
        else if (strcmp(tokenArray[i], "<") == 0)
        {
            input = tokenArray[i + 1];
            tokenArray[i] = NULL;
            break;
        }
    }

    // fork
    pid_t pid = fork();
    if (pid == 0)
    {
        if (output)
        {
            int out = open(output, O_WRONLY | O_TRUNC | O_CREAT, 600);
            if (out < 0)
            {
                perror("Error Occured");
                exit(1);
            }
            dup2(out, STDOUT_FILENO);
            close(out);
        }
        else if (input)
        {
            int in = open(input, O_RDONLY);
            if (in < 0)
            {
                perror("Error Occured");
                exit(1);
            }
            dup2(in, STDIN_FILENO);
            close(in);
        }

        execvp(tokenArray[0], tokenArray);
        printf("execvp failed");
        exit(1);
    }
    else if (pid > 0)
    {
        wait(NULL);
    }
    else
    {
        perror("fork failed");
    }
    return 0;
}

// forks with pipe
int fork_function_piped(char **tokenArray, int tokenCount)
{
    char *leftSide[50];
    char *rightSide[50];
    int pipeIndex = 0;

    // printf("made it to this point");
    for (int i = 0; i < tokenCount; i++)
    {
        if (strcmp(tokenArray[i], "|") == 0)
        {
            pipeIndex = i;
            break;
        }
    }
    //separate out commands 
    for (int i = 0; i < pipeIndex; i++)
    {
        leftSide[i] = tokenArray[i];
    }
    leftSide[pipeIndex] = NULL;

    for (int i = pipeIndex + 1; i < tokenCount; i++)
    {
        rightSide[i - pipeIndex - 1] = tokenArray[i];
    }
    rightSide[tokenCount - pipeIndex - 1] = NULL;
    //pipe
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe failed");
        return -1;
    }
    //fork the first process
    pid_t pid1 = fork();
    if (pid1 < 0)
    {
        perror("fork failed");
        return -1;
    }

    if (pid1 == 0)
    {

        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        if (execvp(leftSide[0], leftSide) == -1)
        {
            perror("execvp failed");
            exit(1);
        }
    }
    //fork the second process
    pid_t pid2 = fork();
    if (pid2 < 0)
    {
        perror("fork failed");
        return -1;
    }
    if (pid2 == 0)
    {

        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        if (execvp(rightSide[0], rightSide) == -1)
        {
            perror("execvp failed");
            exit(1);
        }
    }

    close(fd[0]);
    close(fd[1]);
    wait(NULL);
    wait(NULL);

    return 0;
}

// function: echo: prints array and prints if pipe is present
void echo_parse(char **tokenArray, int count)
{
    char pipeInput[] = "|";
    for (int i = 0; i < count - 1; i++)
    {
        if (strcmp(tokenArray[i], pipeInput) == 0)
        {
            printf("PIPE\n");
            printf("SPACE\n");
        }
        else
        {
            printf("%s\n", tokenArray[i]);
            printf("SPACE\n");
        }
    }
    user_input();
}

// function: get input and split into tokens to store into an array.
void user_input()
{
    char *str = NULL;
    size_t size = 0;
    char *tokensArray[100];
    int tokenCount = 0;
    char *lastToken, *firstToken;
    char echoInput[] = "ECHO";

    printf("Enter a command >> ");
    if (getline(&str, &size, stdin) != -1)
    {
        str[strcspn(str, "\n")] = 0;
        track_history(str);

        // tokenizes the string
        char *split = strtok(str, " ");
        while (split != NULL)
        {
            tokensArray[tokenCount++] = split;
            split = strtok(NULL, " ");
        }

        tokensArray[tokenCount] = NULL;

        // checks if last token is ECHO to trigger the function
        lastToken = tokensArray[tokenCount - 1];
        if (strcmp(lastToken, echoInput) == 0)
        {
            echo_parse(tokensArray, tokenCount);
        }

        firstToken = tokensArray[0];
        command_handler(firstToken, tokensArray, tokenCount);
    }
    else
    {
        perror("Error");
    }
    free(str);
}

// main program

int main()
{
    shell_intro();
    user_input();
    return 0;
}