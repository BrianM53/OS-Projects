Keywords
Shell, processes, pipes, redirection

Introduction
In this programming assignment, you will implement your Linux shell, the Aggie Shell. The Linux shell in your OS lets a user navigate through the file system and performs multiple tasks using simple commands. It also offers capabilities of interprocess communication and file input/out redirection. Your shell should function almost as well as Bash. Each Linux command (e.g., cat, echo, cd, etc.) should run as a child process in this assignment. These commands are executables in your system and are recognized by a call to an exec()-family function. Refer to this website for some interesting and commonly used Linux commands. 

Features of the Aggie Shell
Command Pipelining
While the individual Linux commands are useful for doing specific tasks (e.g., grep for searching, ls for listing files, echo for printing), sometimes the problems at hand are more complicated. We may want to run a series of commands that require the output of one command to be fed as the input of the next. The Linux shell lets you run a series of commands by putting the pipe character (“|”) between each command. It causes the standard output of one command to be redirected into the standard input of the next. 

Input/Output Redirection
Sometimes, the output of a program is not intended for immediate use. Even if someone doesn't intend to look at the output of your program, it is still helpful to have it print out status/logging messages during execution which can be reviewed to help pinpoint bugs. Since it is impractical to have all messages from all system programs printed out to a screen to be reviewed later, sending that data to a file is convenient and desirable. Sometimes it is also done out of necessity (where the result file is packaged for consumption by another entity). Output redirection is implemented by changing the standard output (and sometimes also standard error) to point to a file opened for writing.
An example of output redirection is as follows:
shell> echo "This text will go to a file" > temp.txt 

Executing this command will result in temp.txt holding the contents “This text will go to a file”. Note that “>” is used for output redirection. If we execute this command without output redirection, the string “This text will go to a file” will be printed out to stdout instead. 
We can execute the cat command to verify the contents of temp.txt as a result of the previous command; in this case, cat prints out the contents of the file to stdout:
shell> cat temp.txt
This text will go to a file

Other times, a program might require an extensive list of input commands. It would be a waste of time to type them out individually. Instead, pre-written text in a file can be redirected to serve as the input of the program as if it were entered in the terminal window. In short, the shell implements input redirection by redirecting the standard input of a program to a file opened for reading.
An example of input redirection is as follows:
shell> cat < temp2.txt
This text came from a file

In this example, the content of the file temp2.txt is “This text came from a file”. We redirected the contents of temp2.txt to serve as the input, which results in the shown output.
Implementation hint: For input redirection, you should use the dup2() system call to redirect the file descriptor that corresponds to stdin to the one referred to by the file descriptor opened for reading. Similarly, for output redirection, you should use the dup2() system call to redirect the file descriptor that corresponds to stdout to the one referred to by the file descriptor opened for writing.
Background Processes
When you run a command in the shell, the shell suspends until the command completes its execution. We often do not notice this because many commonly used commands finish soon after they start. However, if the command takes a while to finish, the shell stays inactive, and you cannot use it for that duration. For instance, typing sleep 5 in the shell causes the shell to suspend for 5 seconds. After that, the prompt returns, and you can type the next command. You can change this behavior by sending the program to the background and continuing to use the shell. If you type sleep 5 & in the shell, for example, it will return the shell immediately because the corresponding process for the sleep runs in the background instead of in the foreground where regular programs run.
Implementation Hint: The “&” symbol is removed from the tokenized command, and a boolean is set in the command object that tells you that it runs in the background. From the parent process, do not call waitpid(), which you have been doing for the regular processes. You should instead put the pid into a list/vector of background processes that are currently running. Periodically check on the list to ensure they do not become zombies or do not stay in that state for too long. A good frequency of check is before scanning the next input from the user inside the main loop.  Remember that the waitpid() function suspends when called on a running process. Therefore, calling it as it is on background processes may cause your whole program to get suspended. There is an option in waitpid() that makes it non-blocking, which is the desired way of calling it on background processes. You can find this option on the man pages.
cd commands
You must use the chdir() system call to execute the cd command functionality. For the particular command, cd -, you must keep track of the previous directory; the system call getcwd() may be useful here.
Single/Double Quotes
White spaces are usually treated as argument separators except when they are used inside quotes. For example, notice the difference between the following two commands:
shell> echo -e "cat\ta.txt"
cat	a.txt
shell> echo "-e cat\ta.txt"
-e cat\ta.txt

Note that the “-e” flag for the echo command prints the string with the interpretation of some symbols. Now, in the first command, the string is put inside quotes to make sure that it is interpreted as a single string. As a result of using the -e option, the string is printed after interpreting the “\t” as a tab. In the second example, “-e” is part of the string, which means the character “\t” is not interpreted as a tab but rather literally so.
Also, note the following example:
shell> echo -e '<<<<< This message contains a |||line feed >>>>>\n'
<<<<< This message contains a |||line feed >>>>>

The example does not consider the above command to have redirections or pipes because the corresponding symbols are inside quotation marks. It also interprets the ‘\n’ character due to the -e option given outside the quotation marks.
Your Tasks
Design a simple shell that implements a subset of the functionality of the Bourne Again Shell (BASH). The requirements are detailed below and are followed by the feature list and associated rubrics:
1. Feature Implementation: the list of features is defined in the Shell Features and Rubrics section below):
- Continually prompt the user for the next command input. Print a custom prompt to be shown before taking each command. This should include your user name, current date-time, and the absolute path to the current working directory. The system calls getenv("USER"), time()+ctime(), and getcwd() will help you with this. Example:
Sep 23 18:31:46 user:/home/user$

- Execute commands passed in by the user, parsed by the provided classes:
For executing a command from the shell, you must use the fork()+execvp(...) function pair. You cannot use the system()function to do it because that creates a child process internally without giving us explicit control.
In addition, your shell must wait for the executed command to finish, which is achieved by using the waitpid(...) function.
The provided classes Tokenizer and Command parse the user input into argument lists stored in a vector. Further documentation is provided in the respective header files.
- Support input redirection from a file (e.g., command < filename) and output redirection to a file  (e.g. command > filename). Note that a single command can also have both input and output redirection.
- Allow piping multiple commands together connected by “|” symbols in between them (e.g. command1 | command2). Every process preceding the symbol must redirect its standard output to the standard input of the following process. This is done using an Interprocess Communication (IPC) mechanism called pipe that is initiated by calling the pipe() system call.
- Run the user command in the background if the command contains a “&” symbol at the end (e.g., command & or command arglist &). Note that you must avoid creating zombie processes in this case.
- Allow directory handling commands (e.g., pwd, cd). Note that some of these commands are not recognized by the exec() functions because there are no executables by the same name. These are some additional shell features that must be implemented using system calls (i.e., chdir()) instead of forwarding to exec().
