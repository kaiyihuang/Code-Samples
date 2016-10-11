// Name of Author(s): Fangte Jiang, Kaiyi Huang
// Course Number and Name: CSE 434, Computer Networks
// Semester: Fall 2016
// Project Part: 2
// Time Spent: 12 hours 33 minutes

/*
Five sub functions, getnum, readfile, writefile, process, are used in the server program.
The program use vector clinumlist to record the times every client connected to the server.
Getnum() check each new connection to see whether it has connected once.
Readfile() uses the filename parameter passed in to open the file and uses flock() which is built in to allow multiclients to read the file but prevent them from revising the file. In the loop of sending file, the program will sleep one second to wait for client to receive. If loop without sleep, the client can only receive top lines. After finishing sending file, the program send a "EOF" signal, when client receive this signal, it will finish receiving process. If the file can not be open, it will send "ERROR: File not found" to notice the client.
Writefile() will open a new file and loop until receiving "EOF" and a flock is used to prevent other client from revising the content of the file.
Process() process the request of clients. It receives the message from the client and check the content of last character. Because the message from clients will contain \n, so the program compare the content after the comma with "r\n" and "w\n" to decide calling readfile() or writefile(). After the returning from sub function, it will send "Would you like to continue?" to clients to ask the client if the client wants to go ahead with another request. If the response is "Y", it will continue loop, else if will close the socket and exit.
In the main(), after bind() and listen(), the program can serves multiclients. After receiving the new  client number, program will check the number of children process which is recorded in the parameter children. If the number of children is larger than 5, the program will wait until one children process exit. If the number is less than 5, the program will fork new process and response to the new client.
*/
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/wait.h>  //head file for wait()

using namespace std;
int client_count = 0;

vector<int> clinumlist;

int getnum(int tester)   //check each new connection to see whether it has connected once.
{
    for(int i = 0; i<clinumlist.size(); i++)
    {
        if(clinumlist[i] == tester)  //if client number has connected once, return 1
            return 1;
    }
    return 0;
}


void readfile(char* filename, int sock)    //send the requested file to client
{
    if(access(filename, F_OK) != -1)
    {
        vector<string> stacky;		//restore whole file
        string temp;
        FILE *fp = fopen(filename, "r"); //open file
        flock(fileno(fp),LOCK_SH); 	//This is the lock for the file, it is built-in
        int n;
        char buffer[256];
        char *line = NULL;
        size_t len = 0;
        unsigned int i;
        ssize_t read;
        send(sock,"File found, writing",19,0);
        while(read = getline(&line,&len,fp) != -1)   //read whole file int vector stacky
        {
            stacky.insert(stacky.begin(),temp.assign(line,strlen(line)));
        }
        for(i=0;i<stacky.size();i++)                  //send whole file to the client
        {
            strcpy(line,stacky[i].c_str());
            //printf("Sending: %s\n",line);
            send(sock,line,len,0);
            usleep(205000);
        }
        send(sock,"EOF",4,0);			//send finish signal
        usleep(205000);
        flock(fileno(fp),LOCK_UN); //Releases the lock      //unlock the file
        fclose(fp);					//close the file
    }
    else
    {
        send(sock,"ERROR: File not found",22,0);
    }
}
void writefile(char* filename, int sock)		//write the requested file
{

    char buffer[256];
    bzero(buffer,256);
     FILE *fp = fopen(filename, "w");
     flock(fileno(fp),LOCK_EX); //This is the lock for the file, it is built-in
    while(strcmp(buffer,"EOF") != 0)		//write file
    {
        read(sock,buffer,256);
        if(strcmp(buffer,"EOF") != 0)
        {
            printf("Writing: %s\n",buffer);
            fprintf(fp,"%s\n",buffer);
        }
    }
    printf("Finished Writing\n");
    flock(fileno(fp),LOCK_UN); 				//Releases the lock
    fclose(fp);						//close the file
}


int process(int sock)				//process the request of clients
{
   		int n;
   		char buffer[256];
   		char* filename;
   		char* filemode;
   		while(1==1)
        {
            bzero(buffer,256);
            n = recv(sock,buffer,256,0);
            if (n <= 0) {			//if fail to receive
                perror("ERROR reading from socket");
                close(sock);
                exit(1);
            }
            printf("Here is the message: %s\n",buffer);
            //n = write(sock,"I got your message",18);
            filename = strtok(buffer,", ");
            filemode = strtok(NULL,", ");
            if (strcmp(filemode,"r\n") == 0)        //if request to read, call readfile()
            {
                readfile(filename, sock);
            }
            else if(strcmp(filemode,"w\n") == 0)   //if request to write, call writefile()
            {
                writefile(filename, sock);
            }
            else
            {
                continue;
            }
            send(sock,"Would you like to continue?",28,0);	//ask if the client want to go ahead //with another request
            bzero(buffer,256);
            n = recv(sock,buffer,256,0);
            if(strcmp(buffer, "Y") == 0)		//if answer is yes, keep looping
            {
                continue;
            }
            else if(strcmp(buffer, "N") == 0 )		//if answer is no, close the socket and exit
            {
                printf("Connection Closed\n");
                close(sock);
                exit(0);
            }
        }



}

int getnumber(int a[10000],int b)
{
    int i;
    for(i=0;i<10000;i++)
    {
        //printf("%d\n",a[i]);
        if(a[i]==b)
            return 1;
    }
    return 0;
}


int main(int argc, char *argv[])
{
    int i;
    int children = 0;
    int recvmsgsize;
    int clientlist[10000];
    int clientkappa = 0;
	char buffer[256];
	char *buffer2;
	int n, pid, tester;
	unsigned int cli_len;
    struct sockaddr_in serv_addr ,clientaddr;
    int sockfd,newsockfd,udpfd;
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    memset(&serv_addr,'0',sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1])); // Takes in the port from the command line
    serv_addr.sin_addr.s_addr = INADDR_ANY;
	if (sockfd < 0)
		perror("ERROR opening socket");
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		perror("ERROR on binding");
	listen(sockfd,5);
	cli_len = sizeof(clientaddr);


	while(1)
	{
	    newsockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &cli_len);
        printf("Got a connection\n");
        if (newsockfd < 0)
            perror("ERROR on accept");
        recv(newsockfd,buffer,4,0);
        tester = atoi(buffer);
        printf("The client number is %d\n",tester);
        i = getnum(tester); //Checks for client number on list
         if(i == 1)	    //if client has connected to the server, deny it
         {
            printf("Access denied\n");
            write(newsockfd,"Access denied",14);
            close(newsockfd);
         }
         else if(client_count >= 50)   //if now serving 50 clients, deny it
         {
             printf("Too many clients\n");
             write(newsockfd,"Too many clients",17);
            close(newsockfd);
         }
        else{

            children++;  // Last fork() was successful
            while (children >= 6)   //if have more than 5 children fork, wait until one of them exit
            {
                int status;
                // Wait for one child to exit
                if (wait(&status) == 0)
                {
                    children--;
                }
            }
            clinumlist.push_back(tester); //Adds client number to the list
            pid = fork();
            if (pid < 0) {
                perror("ERROR on fork");
                exit(1);
            }
            else if (pid == 0) {

                printf("Access granted\n");
                write(newsockfd,"Access granted",15);
                close(sockfd);
                process(newsockfd);
                exit(0);
            }
            else
            {
                close(newsockfd);
            }
        }

	}
}
