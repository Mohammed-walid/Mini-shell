/*
 * 
 *
 * Template file.
 *
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"

// Added: Signal handler for logging terminated child processes
void sigchld_handler(int sig) {
    FILE *logFile = fopen("shell.log", "a");
    if (logFile) {
        time_t now = time(NULL);
        fprintf(logFile, "Child process terminated at %s", ctime(&now));
        fclose(logFile);
    }
    waitpid(-1, NULL, WNOHANG);
}

void parse(std::vector<Token> &tokens)
{
    Command currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();

    // Added: Parsing logic to build currentCommand before it executes
    for (size_t i = 0; i < tokens.size(); i++) {
        Token token = tokens[i];
        if (token.type == TOKEN_COMMAND || token.type == TOKEN_ARGUMENT) {
            if (token.value.find('*') != std::string::npos || token.value.find('?') != std::string::npos) {
                glob_t glob_result;
                glob(token.value.c_str(), GLOB_NOCHECK | GLOB_TILDE, NULL, &glob_result);
                for (size_t j = 0; j < glob_result.gl_pathc; j++) {
                    currentSimpleCommand->insertArgument(strdup(glob_result.gl_pathv[j]));
                }
                globfree(&glob_result);
            } else {
                currentSimpleCommand->insertArgument(strdup(token.value.c_str()));
            }
        } 
        else if (token.type == TOKEN_PIPE) {
            currentCommand.insertSimpleCommand(currentSimpleCommand);
            currentSimpleCommand = new SimpleCommand();
        } 
        else if (token.type == TOKEN_REDIRECT) { 
            currentCommand._outFile = strdup(tokens[i + 1].value.c_str());
            i++; 
        } 
        else if (token.type == TOKEN_APPEND) { 
            currentCommand._outFile = strdup(tokens[i + 1].value.c_str());
            currentCommand._append = 1;
            i++;
        } 
        else if (token.type == TOKEN_INPUT) { 
            currentCommand._inputFile = strdup(tokens[i + 1].value.c_str());
            i++;
        } 
        else if (token.type == TOKEN_ERROR) { 
            currentCommand._errFile = strdup(tokens[i + 1].value.c_str());
            i++;
        } 
        else if (token.type == TOKEN_REDIRECT_AND_ERROR) { 
            currentCommand._outFile = strdup(tokens[i + 1].value.c_str());
            currentCommand._errFile = strdup(tokens[i + 1].value.c_str());
            currentCommand._out_error = 1;
            currentCommand._append = 1;
            i++;
        } 
        else if (token.type == TOKEN_BACKGROUND) { 
            currentCommand._background = 1;
        } 
        else if (token.type == TOKEN_EOF) {
            if (currentSimpleCommand->_numberOfArguments > 0) {
                currentCommand.insertSimpleCommand(currentSimpleCommand);
            } else {
                delete currentSimpleCommand;
            }
            break;
        }
    }

    currentCommand.execute();
    /*
      Implement this function 
      you must parse for all possible tokens scenario
      right now command.execute only print the format , you will inject it in your logic.

    */
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));
        
    // Added: Realloc fix to prevent the malloc(0) crash without changing the original line above
    _simpleCommands = (SimpleCommand **)realloc(_simpleCommands, _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    
    // Added: Missing variable initializations
    _append = 0;
    _out_error = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    
    // Added: Reset missing variables
    _append = 0;
    _out_error = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
        // Added: Newline to fix formatting
        printf("\n"); 
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    print();
    /*
        Write your code here
    */
    
    // Added: Command execution logic 
    if (_numberOfSimpleCommands > 0) {
        if (strcmp(_simpleCommands[0]->_arguments[0], "exit") == 0) {
            printf("Good bye!!\n");
            exit(0);
        }

        if (strcmp(_simpleCommands[0]->_arguments[0], "cd") == 0) {
            if (_simpleCommands[0]->_numberOfArguments > 1) {
                chdir(_simpleCommands[0]->_arguments[1]);
            } else {
                chdir(getenv("HOME"));
            }
        } else {
            int defaultin = dup(0);
            int defaultout = dup(1);
            int defaulterr = dup(2);

            int fdin;
            if (_inputFile) {
                fdin = open(_inputFile, O_RDONLY);
            } else {
                fdin = dup(defaultin);
            }

            int ret = -1;
            int fdout;
            int fderr;

            for (int i = 0; i < _numberOfSimpleCommands; i++) {
                dup2(fdin, 0);
                close(fdin);

                if (i == _numberOfSimpleCommands - 1) {
                    if (_outFile) {
                        if (_append) {
                            fdout = open(_outFile, O_CREAT | O_WRONLY | O_APPEND, 0666);
                        } else {
                            fdout = open(_outFile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
                        }
                    } else {
                        fdout = dup(defaultout);
                    }
                } else {
                    int fdpipe[2];
                    pipe(fdpipe);
                    fdout = fdpipe[1];
                    fdin = fdpipe[0]; 
                }

                dup2(fdout, 1);
                close(fdout);

                if (_out_error) {
                    dup2(1, 2); 
                } else if (_errFile) {
                    if (_append) {
                        fderr = open(_errFile, O_CREAT | O_WRONLY | O_APPEND, 0666);
                    } else {
                        fderr = open(_errFile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
                    }
                    dup2(fderr, 2);
                    close(fderr);
                } else {
                    dup2(defaulterr, 2);
                }

                ret = fork();
                if (ret == 0) {
                    signal(SIGINT, SIG_DFL);
                    execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
                    perror("execvp");
                    exit(1);
                } 
            }

            dup2(defaultin, 0);
            dup2(defaultout, 1);
            dup2(defaulterr, 2);
            close(defaultin);
            close(defaultout);
            close(defaulterr);

            if (_background == 0 && ret != -1) {
                waitpid(ret, NULL, 0); 
            }
        }
    }
    
    // Added: A return to safely exit the function and bypass the infinite 
    // recursive crash caused by the original clear() and prompt() calls below.
    clear();
    return;

    clear();
    prompt();
}

void Command::prompt()
{
    printf("myshell>");
    fflush(stdout);
    std::string input;
    while (true)
    {
        std::getline(std::cin, input); 
        
        // Added: EOF and empty input safety checks
        if (std::cin.eof()) exit(0);
        if (input.empty()) {
            printf("myshell>");
            fflush(stdout);
            continue;
        }

        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
        
        // Added: Reprint prompt for the loop iteration
        printf("myshell>");
        fflush(stdout);
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    // Added: Signal setup
    signal(SIGINT, SIG_IGN);
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    Command::_currentCommand.prompt();
    return 0;
}
