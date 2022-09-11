//Includes necessary libraries
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAXLEN 400
#define _WEXITSTATUS(status)
#define _WIFEXITED(status)

//Global variables 
char dir[MAXLEN];
char cwd[MAXLEN];
char *args[MAXLEN];
char *cmnd; 
pid_t pid; 
int status;
size_t buffer;
int start;
int end;
int background;
char* curr;
int lines;
char delim[3] = {' ', '\t', '\n'};

//Creating necessary structs for linked list for storing background tasks 
struct node{
    int pid;
    char* command;
    struct node *next;
};

struct list{
    struct node *head; 
    struct node **tail; 
};

//Global variable 
struct list* pids;

//Function to add node to list
void add_node (struct list *l, int pid, char* command){
    struct node* n = malloc(sizeof(struct node));
    n->command = command;
    n->pid = pid;
    n->next = NULL;
    *l->tail = n;
    l->tail = &n->next;
}

//Function to create list
struct list* create_list() {
    struct list* l;
    struct node *n = calloc(1, sizeof(struct node)); 
    l = calloc(1, sizeof(struct list));
    l->head = n;
    l->tail = &n->next;
    return l;
}

//Function to print list of background tasks
void print_list(struct list *l) {
    struct node *n = l->head->next;
    if(n == NULL){ //If there are no current jobs
        printf("No jobs\n");
        return;
    }

    printf("Current jobs:\n"); //Prints all the current jobs
    while (n != NULL) {
        printf("Pid %d: %s\n", n->pid, n->command);
        n = n->next;
    }
}

//Get current directory
void get_directory(){
    
    if (getcwd(cwd, sizeof(cwd)) == NULL){
        perror("The directory could not be printed");
        return;
    }
    else {
        printf("%s: ", cwd);
        return; 
    }
}

//Function to parse input into command and arguments, and free resources
int handle_input(){
    buffer = MAXLEN;
    cmnd = (char*)malloc(buffer *sizeof(char));
    lines = getline(&cmnd, &buffer, stdin);
    curr = strdup(cmnd);

    if(lines == -1){ //Error handling
        exit(0);
    }
    
    char* token;
    int i = 0;
    while(1) {
        token = strsep(&curr, delim);
        if(token == NULL){
            return i;
        }
        if(token[strlen(token) - 1] == '&'){
            token[strlen(token) - 1] = '\0';
        }
        if(strlen(token)){
            args[i] = strdup(token);
            i++;
        }
    }
    free(curr);
    return i;
}

//Function for executing command flush using fork. 
void execute(int input){

    //The cmnd of input
    cmnd = args[0];

    //Implementing I/O redirection
    start = 0;
    end = 0;
    for (int i = 0; i < input; ++i){
        if (strcmp(args[i], "<") == 0) {
            if(i >= input - 1){
                printf("Write desired path for input-redirection: ");
                return;
            }
            start = i++;
            args[i] = NULL;
        }
        else if(strcmp(args[i], ">") == 0) {
            if(i >= input - 1) {
                printf("Write desired path for output-redirection: ");
                return;
            }
            end = ++i;
            args[i] = NULL;
        }
    }

    //Handle cd-command for changing directory 
    if(strcmp(cmnd, "cd") == 0){
        if(args[1] == 0){
            chdir(dir);
            printf("Empty path.\n");
            return;
        }
        chdir(args[1]);
        return;
    }
    
    //Setting background taks to 1 if input command is written with &
    background = 0;

    if (!strcmp(cmnd, "&")) {
        background = 1;
    }

    //Forking to create child processes
    pid_t pid = fork();


    if (pid == 0){

        int out = fileno(stdout);
        if(!strcmp(cmnd, "jobs")) {
            print_list(pids);
        } 
        if(start){
            freopen(args[start], "r", stdin); 
        }
        if(end){
            freopen(args[end], "w", stdout);
        }
        else {
            execvp(args[0], args);
        }  
        exit(0);
    }
    //Error handling
    else if(pid == -1){
        printf("ERROR: could not fork process");
        return;
    }

    else if (!background){
        wait(&status);
        if (WIFEXITED(status)){
            if(cmnd[strlen(cmnd)-1] == '\n') {
                cmnd[strlen(cmnd)-1]='\0';
            }

            int status_exit = WEXITSTATUS(status);
            //Printing argument and command 
            printf("%s", args[1]);
            printf("\nExit status [%s] = %d\n", cmnd, status_exit);
            free(cmnd);
            return;
        }
    }
    //Adds node to list (pids) if it is a backgrounf task
    else{
        add_node(pids, pid, cmnd);
    }

}

//Function to check status on zombies (child processes that have not been terminated)
int status_zombies(int pid) {
    int status;
    if (waitpid(pid, &status, WNOHANG))
    {
        if (WIFEXITED(status)){
            return WEXITSTATUS(status);
        }
    }
    return -1;
}

//Function to kill zombies
void delete_zombies (struct list *l) {
    struct node *first = pids->head;
    struct node *second = pids->head->next;
    while(second != NULL) {
        int status = status_zombies(second->pid);
        if(status != -1){
            char* sec_cmnd = second->command;
            if(sec_cmnd[strlen(sec_cmnd)-1] == '\n') {
                sec_cmnd[strlen(sec_cmnd)-1]='\0';
            }
            int status_exit = WEXITSTATUS(status);
            printf("Exit status [%s] = %d\n", sec_cmnd, status_exit);
            free(sec_cmnd);

            first->next = second->next;
            free(second);

        } 
        else{
            first = second;
        }
        second = first->next;
    }
}

//Main function
int main(){

    if (getcwd(cwd, sizeof(cwd)) == NULL){
      perror("Error with getcwd() command");
    }
    memcpy(dir, cwd, sizeof(cwd));
    pids = create_list(); //creating list to store all background tasks 
    int parsed_input = 0; //initialising input 

    //Will continue asking for tasks until input ctr D is typed
    while(1) {

        //Getting the current directory 
        get_directory();
        
        //Handling input
        parsed_input = handle_input();

        if(parsed_input != 0) //As long as number of commands is not zero
        {
            execute(parsed_input); //Executing command
            for(int i = 0; i < parsed_input; ++i) {
                free(args[i]); //Freeing arguments
            }
        }
        delete_zombies(pids); //Removing zombies
    }
    return 0;
}

