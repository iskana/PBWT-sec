#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>

#define SOCKLEN socklen_t
#define BUFSIZE 10240

int prepSSock(int port=23456, int max_con=1, int aflg=0, char* addr=NULL);
int acceptSSock(int sock);
int prepCSock(char* host, int port=23456);
void closeSock(int s);
int sendFile(int sock, char* file);
int recvFile(int sock, char* file);
