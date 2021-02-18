//Amanda Elvarsdottir

#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <vector>
#include <sstream>
#include <fcntl.h>

using namespace std;

//https://stackoverflow.com/questions/55672661/what-this-character-sequence-033h-033j-does-in-c
#define clean() printf("\033[H\033[J")

void doStuff(char *in);

//I wanted my shell to greet the user when using my "cute shell"
void welcome_shell(){
    //The clear command (clear the shell)
    clean();

    printf("\n\n ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~");
    char* username = getenv("USER");
    printf("\n\n\n\t WELCOME @%s", username);
    printf(" TO THIS CUTE SHELL");
    printf("\n\n\t MADE BY: Amanda Elvarsdottir");
    printf("\n\n\n ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ \n");

    sleep(3); //sleeps after 3 sec (just a quick welcome)
    clean();
}

//https://www.tutorialspoint.com/parsing-a-comma-delimited-std-string-in-cplusplus
vector <string> getPath(string path){
    vector<string> results;
    stringstream sstream(path);
    while (sstream.good()) {
        string ss;
        getline(sstream, ss, ':'); //devided by colon. Splitting up paths.
        results.push_back(ss);
    }
    return results;
}

//the same as getPath except ':' and ';'
//We create the queue before handling it
vector <string> makeQueue(string path){
    vector<string> results;
    stringstream sstream(path);
    while (sstream.good()) {
        string ss;
        getline(sstream, ss, ';'); //devided by semicolon. Splitting up paths.
        results.push_back(ss);
    }
    return results;
}

//Handle the queue that was made
void handleQueue(char *in){
    vector <string> queue = makeQueue(in);
    for(int i=0; i < queue.size(); i++){
        doStuff((char *)queue.at(i).c_str());
    }
}

//Path execution
void pathExec(const char *in, char **args){
    int pathex = execv(args[0], args);
    if (pathex == -1){
        perror("Can't find path for execution");
    }
}

//Bang for #
void bang(char *in){
    string tostr(in);
    string val = tostr.substr(tostr.find("!") + 1);
    int i = stoi(val);
    HIST_ENTRY **list = history_list(); //Look up better.
    char *cmd = list[i-1]->line; //Because array starts with 0
    doStuff(cmd);
}

//Have to do for all types of redirections, easier with a method
void erasefile(char* args[], int n, int c){
    for(int i=n; i < 6; i++){
        if(i >= 4){
            args[i] = 0;
        }
        args[i] = args[i+2];
    }
}

void doStuff(char *in){

    //Had to move my variables from the else statement (where I execute) to here, for redirection.
    char *args[6];
    int status;
    char *path = getenv("PATH");
    //int filedescriptor;
    //array = split the input
    //From in class notes.
    int ct = sscanf(in, "%ms %ms %ms %ms %ms %ms", &args[0], &args[1], &args[2], &args[3], &args[4], &args[5]);



    //Change directories.
    if(strstr(in, "cd") != nullptr){
        //cout << "You've reached cd" << endl; cd wasn't working so I had to find it with cout.
        //https://man7.org/linux/man-pages/man2/chdir.2.html
        //https://www.geeksforgeeks.org/chdir-in-c-language-with-examples/
        //https://man7.org/linux/man-pages/man2/chdir.2.html
        string command = string(in);
        string path = command.substr(command.find("cd")+3, command.length());
        int ch = chdir(path.c_str());
        if (ch == -1){
            perror("Can't change directory");
        }
    }

    else if(strstr(in, "=") != nullptr){
        //Turn the input into a string, and substr (will probably break easily)
        string input = string(in);
        string variable = input.substr(0, input.find("="));
        string value = input.substr(input.find("=") + 1);
        cout << variable << " " << value << endl;
        int env = setenv(variable.c_str(), value.c_str(), 1);
        if(env == -1){
            perror("Can't set environment variables");
        }
    }

    //Run multiple commands with ";" inbetween and in that order (form a queue)
    else if(strstr(in, ";") != nullptr){
        handleQueue(in);
    }

    //Bang # and it prints out the bang from the history with that number (in the order it was called)
    else if(strstr(in, "!") != nullptr){
        bang(in);
    }
    //Exit programs, and say it's goodbye
    else if (strcmp("exit", in) == 0){
        printf("\nBye, bye!\n");
        exit(0);
    }

    else{
    //The execution itself
    //MOVED THE VARIABLES TO TOP OF THIS METHOD
    // From notes on euclid (http://euclid.nmu.edu/~rappleto/Classes/CS426/Assignments/TheBigAssignment/notes-2020b.txt)
    //char *path = getenv("PATH");
    //char *args[6];
    //int status;
    int pid = fork();
    if(pid == 0){ //if kid
        //int ct = sscanf(in, "%ms %ms %ms %ms %ms %ms", &args[0], &args[1], &args[2], &args[3], &args[4], &args[5]);
        //To be able to get full path
    
        //Redirection (for loop: the count -1)
        for(int i = 0; i < ct; i++){
            //File output redirection
            if(args[i] && (strcmp(args[i], ">" )) == 0){
                int filedescriptor = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0660); 
                // here the newfd is the file descriptor of stdout (i.e. 1)
                //Got this part from the email about dup2
                assert(filedescriptor != -1);
                assert((dup2(filedescriptor, 1)) != -1);
                //Create a method to erase the filename
                erasefile(args, i, ct);
            }
            //File output redirection "append-mode"
            //Same as ">" but O_APPEND
            if(args[i] && (strcmp(args[i], ">>" )) == 0){
                int filedescriptor = open(args[i+1], O_WRONLY | O_CREAT | O_APPEND, 0660); 
                // here the newfd is the file descriptor of stdout (i.e. 1)
                assert(filedescriptor != -1);
                assert((dup2(filedescriptor, 1)) != -1);
                //Create a method to erase the filename
                erasefile(args, i, ct);
            }
            //File input redirection
            if(args[i] && (strcmp(args[i], "<")) == 0){
                int filedescriptor = open(args[i+1], O_RDONLY);
                assert(filedescriptor != -1);
                assert((dup2(filedescriptor, 0)) != -1);
                erasefile(args, i, ct);
            }
        }
        vector <string> paths = getPath(path); 
        cout << "About to go through path execution" << endl;
        if(strstr(args[0],"/") != nullptr){
            pathExec(in, args);
        }else{
            for(int i =0; i < paths.size(); i++){
                string stringappend = paths.at(i)+"/"+args[0];
                int execution = execv(stringappend.c_str(), args);
                if(execution != 0){
                    perror("Failed to execute");
                }
            }
        }
    }else if(pid > 0){
        int k = waitpid(-1, &status, 0);
    }
    }
}

void handleIntrp(int sig){ //keyboard interruption
    cout << endl;
}

int main(){
    welcome_shell();

    while(true){
        char *buffer;
        signal(SIGINT, handleIntrp);
        buffer = readline("\n");
        int buflength = strlen(buffer);
        if(buflength != 0){
            add_history(buffer);
            doStuff(buffer);
        }  
    }
}