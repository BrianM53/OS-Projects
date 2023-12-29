#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

#include <vector>
#include <string>

#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

int main () {
    char prev[100];
    char temp[100];
    dup(0);
    dup(1);

    //copy stdin and stdout using dup
    for (;;) {
        //implement iteration over vector of bg pid (vector also declared outside of the loop)
        //waitpid() - using flag for non-blocking

        //implement date/time  with to/do
        time_t now = time(0);
        struct tm local_time;
        localtime_r(&now, &local_time);

        //implement username with getlogin()
        const char *username = getlogin();

        //implement curdir with getcwd()
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            return 1;
        }
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%b %d %H:%M:%S", &local_time);

        // need date/time, username, and absolute path to current dir
        cout << YELLOW << time_str << " " << username << ":" << cwd << "$ " << NC;
        
        // get user inputted command
        string input;
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        //chdir()
        //if dir (cd <dir>) is "-", then go to previous working directory
        //variable storing the previous directory (it needs to be declared outside of the loop)

        // get tokenized commands from user input
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }

        // // print out every command token-by-token on individual lines
        // // prints to cerr to avoid influencing autograder
        for (auto cmd : tknr.commands) {
            for (auto str : cmd->args) {
                cerr << "|" << str << "| ";
            }
            if (cmd->hasInput()) {
                cerr << "in< " << cmd->in_file << " ";
            }
            if (cmd->hasOutput()) {
                cerr << "out> " << cmd->out_file << " ";
            }
            cerr << endl;
        }



        //for piping 
        //for command in commands
        //  call pipe() to make pipe
        //  fork() - in child, redirect stdout, in parent redirect stdin
        //  ^ is already written?
        //add checks for the first or last command
        for(long unsigned int i = 0; i < tknr.commands.size(); i++){

            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }

            // fork to create child
            pid_t pid = fork();
            if (pid < 0) {  // error check
                perror("fork");
                exit(2);
            }

            //add check for background process - add pid to vector if bg and don't waitpid() in parent

            if (pid == 0) {  // if child, exec to run command

                int status = 0;
                waitpid(pid, &status, 0);

                // run single commands with no arguments
                //implement multiple arguments - iterate over args of current command to make 
                //char * array
                
                /* char** args = new char*[tknr.commands[i]->args.size() + 1];
                for (long unsigned int k = 0; k < tknr.commands[i]->args.size(); k++) {
                    args[k] = (char *) tknr.commands[i]->args.at(k).c_str();
                }
                args[tknr.commands[i]->args.size()] = nullptr; */

                //if current command is redirectedd, then open file and dup2 std(in/out) thats being redirected
                //implement it safely for both at the same time

                Command command = *(tknr.commands[i]);
                char** cmd = new char *[command.args.size() + 1];
                for(long unsigned int k = 0; k < command.args.size(); k++) {
                    cmd[k] = (char *) command.args[k].c_str();
                }
                cmd[command.args.size()] = nullptr;

                /* for (long unsigned int k = 0; k < tknr.commands[i]->args.size() +1; k++) {
                    delete[] args[k];
                }
                delete[] args; */


                if (tknr.commands[i]->hasInput()) {
                    int file = open((tknr.commands[i]->in_file).c_str(), O_RDONLY, 0600);
                    if (file == -1) {
                        perror("open");
                        exit(2);
                    }
                    dup2(file, STDIN_FILENO);
                    close(file);
                }

                if (tknr.commands[i]->hasOutput()) {
                    int file = open((tknr.commands[i]->out_file).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
                    if (file == -1) {
                        perror("open");
                        exit(2);
                    }
                    dup2(file, STDOUT_FILENO);
                    close(file);
                }

                //store previous directory
                //store current directorys
                //chdir to move directorys
                if (strcmp(tknr.commands[i]->args.at(0).c_str(), "cd") == 0) {
                    if (strcmp(tknr.commands[i]->args.at(1).c_str(), "-") == 0) {
                        getcwd(temp, sizeof(temp));
                        chdir((char *) prev);
                        memcpy(prev, temp, sizeof(temp));
                        continue;
                    }
                    else {
                        getcwd(prev, sizeof(prev));
                        chdir(tknr.commands[i]->args.at(1).c_str());
                        continue;
                    }
                }


                // In child, redirect output to write end of pipe
                if(i < tknr.commands.size() - 1) {
                    dup2(pipefd[1], 1);
                }
                
                // In child, execute the command

                if (execvp(cmd[0], cmd) < 0) {  // error check
                    perror("execvp");
                    exit(2);
                }

                // Close the read end of the pipe on the child side.
                close(pipefd[0]);
                close(pipefd[1]);
                
                return 1;
            }
            else {  // if parent, wait for child to finish

                int status = 0;
                waitpid(pid, &status, 0);
                if (status > 1) {  // exit if child didn't exec properly
                    exit(status);
                }

                dup2(pipefd[0], 0);

                close(pipefd[0]);
                close(pipefd[1]);

                if(i == tknr.commands.size() - 1) {
                    wait(0);
                }
            }
            //restore stdin/out
            close(pipefd[0]);
            close(pipefd[1]);
        }
        dup2(3, 0);
        dup2(4, 1);
    }
}