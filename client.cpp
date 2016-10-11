
// Name of Author(s): Fangte Jiang, Kaiyi Huang
// Course Number and Name: CSE 434, Computer Networks
// Semester: Fall 2016
// Project Part: 2
// Time Spent: 12 hours 33 minutes


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
//contains head file of wmemset()
#include <wchar.h>

//contains definitions of a number of data types used in system calls
#include <sys/types.h>

//definitions of structures needed for sockets
#include <sys/socket.h>

//in.h contains constants and structures needed for internet domain addresses
#include <netinet/in.h>

//netdb.h defines the structure hostent,
#include <netdb.h>

//This function is called when a system call fails. It displays a message about the error on stderr and then aborts the program.
#include <time.h>

using namespace std;

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a)
            return d;
    }
}
struct hostent1 {
	char* hostname;
	int client;
	int portnum;
};
hostent1 host;

void error(const char *msg)
{
	perror(msg);
	exit(0);
}
#define BUF_SIZE 256
size_t nbytes = 100;
int nNetTimeout = 1000;//1 second
void delay(int seconds); //delay function for second
int openTCPSock(struct sockaddr_in sockAddr); //open TCP socket using sockAddr
void sendData(int sock, char bufSend[], char bufRecv[]);
void RecvFile(int sock, char bufSend[], char bufRecv[], char* filename);
void SendFile(int sock, char bufSend [], char bufRecv [], char* filename);
int main( int argc, char *argv[] ) {
	//	myFile>>host.hostname>>host.client>>host.portnum;  //use file input the parameters
	if(argc != 4)
    {
        cout<<"Correct usage: client h c p"<<endl;
        exit(1);
    }

	host.hostname = argv[1];
	host.client = atoi(argv[2]);
	host.portnum = atoi(argv[3]);
	const char* temp = host.hostname; // string must be transfer to char array
	struct sockaddr_in sockAddr;
	bzero((char *)&sockAddr,sizeof(sockAddr));  //fullfill each byte with 0
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(temp);
	sockAddr.sin_port = htons(host.portnum);
	//while (1) {
		char bufSend[BUF_SIZE];		//send buff
		//bufSend[0]= host.client;
		sprintf(bufSend,"%d",host.client);
		char bufRecv[BUF_SIZE] = {0};	//receive buff
		int sock = openTCPSock(sockAddr);	//open socket
		sendData(sock, bufSend, bufRecv);
	//}
	return 0;
}
int openTCPSock(struct sockaddr_in sockAddr)
{
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   //open socket
	if (sock < 0)						//if fail to open socket
		error("ERROR opening socket");
	int iResult = connect(sock, (struct sockaddr *)&sockAddr, sizeof(sockAddr));
	if (iResult <0) {					//if fail to connect
		error("ERROR connecting");
	printf("Please enter the message: ");
		return 1;
	}
	return sock;
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout, sizeof(int)); //setup delay
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
}
void sendData(int sock, char bufSend[], char bufRecv[])
{
	//get the string input by user and send to server

		char s1[BUF_SIZE];
		char choice;
		char* filename;
		char* mode;
		char *s2 = NULL;
		printf("%s\n",bufSend);
		send(sock, bufSend, strlen(bufSend), 0);	//first, send client number
		memset(bufSend, 0, BUF_SIZE);  			//reset buffer
		recv(sock, bufRecv, BUF_SIZE, 0);
		printf("recv from server: %s\n", bufRecv);
		if(strcmp(bufRecv,"Access denied")==0)		//if server denies the connection, exit
            		exit(1);
	while (1)
	{
        redo:
		printf("please input request: ");
		getline(&s2,&nbytes,stdin);//get the data of user
		strcpy(bufSend,s2);
		send(sock, bufSend, strlen(bufSend), 0);//send the data, it is ASCII
		strcpy(s1,bufSend);
		filename = strtok(s2, ", ");		//slice the string by comma
		mode=strtok(NULL,", ");			//get the right part of the requrest
        memset(bufSend, 0, BUF_SIZE);			//reset buffer
		memset(bufRecv, 0, BUF_SIZE);			//reset buffer
		if(strcmp(mode,"r\n")==0)		//if request is read, call RecvFile()
			RecvFile(sock,bufSend,bufRecv,filename);
        else if(strcmp(mode,"w\n")==0)			//if request is write, call sendFile
            SendFile(sock,bufSend,bufRecv,filename);
        else						//deal with invalid command
            {
            printf("Invalid Command\n");
            goto redo;
            }
		//receive the data from server
		recv(sock, bufRecv, BUF_SIZE, 0);
		//print the data from server
		//delay(1);
		printf("recv from server: %s\n", bufRecv);
		choicy:
		printf(">");
        scanf("%c",&choice);				//get user's choice
        getchar();
		if (choice == 'N'|| choice == 'n')	//if choose no
		{
            send(sock,"N",2,0);
			close(sock);					//close socket
			printf("The socket has been closed\n");
			break;
		}
		else if(choice == 'Y'|| choice == 'y')		//if choose yes
		{
            send(sock,"Y",2,0);
            continue;
		}
		else
            goto choicy;
		memset(bufSend, 0, BUF_SIZE);			//reset buffer
		memset(bufRecv, 0, BUF_SIZE);			//reset buffer
	}
}
void delay(int seconds)    //function for delay n second
{
	clock_t start = clock();
	clock_t lay = (clock_t)seconds * CLOCKS_PER_SEC;
	while ((clock() - start) < lay)
		;
}
void RecvFile(int sock, char bufSend [BUF_SIZE], char bufRecv [BUF_SIZE],char* filename) //read file in server
{
    FILE *fp;
    fp = fopen(filename,"w");			//open file to write
	recv(sock, bufRecv, 100, 0);
	printf("recv from server: %s\n", bufRecv);
	if(strcmp(bufRecv, "ERROR: File not found") == 0)  //if file can not be found
		printf("No file can be received\n");
	else{
		memset(bufRecv, 0, BUF_SIZE);
		while(1)				//keep receiving the file
		{
			recv(sock, bufRecv, BUF_SIZE, 0);
			//printf("recv from server: %s\n", bufRecv);
			if (strcmp(bufRecv, "EOF") == 0)//if receive finish signal, close file and quit loop
			{
				printf("File receive finish\n");
				fclose(fp);
				break;
			}
			fprintf(fp,"%s",bufRecv);
			memset(bufRecv, 0, BUF_SIZE);
		}
	}
}

void SendFile(int sock, char bufSend [BUF_SIZE], char bufRecv [BUF_SIZE], char* filename)   //write the file in server
{
    ifstream in(filename);   //open file for read

    if (in.is_open())
    {
        vector<string> lines_in_reverse;
        string line;
        while (getline(in, line))
        {													// Store the lines in reverse order.
            lines_in_reverse.insert(lines_in_reverse.begin(), line);  //read file reversely
        }
        for(unsigned int i=0;i<lines_in_reverse.size();i++)		//send file to server for writing
        {
            strcpy(bufSend,lines_in_reverse[i].c_str());
            cout<<lines_in_reverse[i]<<endl;
            send(sock,bufSend,BUF_SIZE,0);
            usleep(200500);						//pause in process of sending file
        }
        send(sock,"EOF",4,0);					//send finish signal
        usleep(200500);						//pause in process of sending file
        in.close();
    }
    else
    {
        printf("Error: Unable to open file\n");
    }




}

