#include "comm.h"

int prepSSock(int port, int max_con, int aflg, char* addr){
	int sock0;
	struct sockaddr_in saddr;

	if( (sock0=socket(AF_INET, SOCK_STREAM, 0)) < 0 ) { 
		perror("socket()");
		exit(1);
	}
	
	memset((char*)&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	if(aflg)
		saddr.sin_addr.s_addr = inet_addr(addr);
	else
		saddr.sin_addr.s_addr =  INADDR_ANY;
	
	if( bind(sock0, (struct sockaddr*)&saddr, sizeof(saddr))<0) {
		perror("bind()");
		close(sock0);
		exit(1);
	}

	if (listen(sock0, max_con) < 0) {
		perror("listen()");
		exit(1);
	}
	return(sock0);
}

int acceptSSock(int sock){
	int sock0, sock1;
	sock0=sock;
	struct sockaddr_in caddr;
	SOCKLEN len = sizeof(caddr);

	if ((sock1 = accept(sock0, (struct sockaddr *)&caddr, &len)) < 0) {
		perror("accept()");   
		exit(1);
	}
	return(sock1);
}

void closeSock(int s){
	close(s);
}

int prepCSock(char* host, int port){
	if(host==NULL){
		fprintf(stderr,"error: no hostname.\n");
		exit(1);
	}

	int sock;
	struct sockaddr_in  addr;
	struct hostent*     hp;
	
	if( (sock=socket(AF_INET, SOCK_STREAM, 0)) <0 ) {
		perror("socket()");
		exit(1);
	}
	
	memset((char*)&addr, 0, sizeof(addr));
	
	if( (hp=gethostbyname(host))==NULL) {
		perror("gethostbyname()");
		exit(1);
	}
	
	bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);
	
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		perror("connect()");
		exit(1);
	}
	return(sock);
}

int send_all(int sock, char *buf, size_t size)
{
	int	retval = 0;
	int	rslt;
	int	size_to_send = size;

	while(size_to_send > 0){
		rslt = send(sock, &buf[size - size_to_send], size_to_send, 0);
		if(rslt < 0){
			perror("send()");
			retval = -1;
			break;
		}else if(rslt == 0){
			fprintf(stderr, "Sent data is not enough\n");
			retval = -1;
			break;
		}
		size_to_send -= rslt;
	}
	return retval;
}

int sendFile(int sock, char* file){
	char buf[BUFSIZE];
	int rslt;
	FILE *fp;
	if((fp=fopen(file,"rb"))==NULL){
		fprintf(stderr,"error: cannot open %s.\n",file);
		exit(1);
	}
	//send filesize
	fseek(fp, 0, SEEK_END);
	int flen=ftell(fp);
	int oflen=flen;
	fseek(fp, 0, SEEK_SET);
	rslt = send_all(sock, (char *)&flen, sizeof(flen));

	while(flen>0){
		if(flen>=BUFSIZE){
			fread(buf, sizeof(char), BUFSIZE, fp);
			rslt = send_all(sock, buf, BUFSIZE);
			if(rslt < 0){
				fprintf(stderr, "%s(%d) send_all() failed\n", __FILE__, __LINE__);
				exit(-1);
			}
		}else{
			fread(buf, sizeof(char), flen, fp);
			rslt = send_all(sock, buf, flen);
			if(rslt < 0){
				fprintf(stderr, "%s(%d) send_all() failed\n", __FILE__, __LINE__);
				exit(-1);
			}
		}
		flen -= BUFSIZE;
		//		std::cerr<<buf;
	}
	fclose(fp);
	return(oflen);
}

int recv_all(int sock, char *buf, size_t size)
{
	int	retval = 0;
	int	rslt;
	int	size_to_recv = size;

	while(size_to_recv > 0){
		rslt = recv(sock, &buf[size - size_to_recv], size_to_recv, 0);
		if(rslt < 0){
			perror("recv()");
			retval = -1;
			break;
		}else if(rslt == 0){
			fprintf(stderr, "Received data is not enough\n");
			retval = -1;
			break;
		}
		size_to_recv -= rslt;
	}
	return retval;
}

int recvFile(int sock, char* file){
	char buf[BUFSIZE];
	int flen;
	int rslt;

	rslt = recv_all(sock, (char*)&flen, sizeof(flen));
	if(rslt < 0){
		fprintf(stderr, "%s(%d) recv_all() failed\n", __FILE__, __LINE__);
		exit(-1);
	}
	
	int oflen = flen;
	
	FILE *fp;
	if((fp=fopen(file,"wb"))==NULL){
		fprintf(stderr,"error: cannot open %s.\n",file);
		exit(1);
	}
	while(flen>0){
		if(flen>=BUFSIZE){
			rslt = recv_all(sock, buf, BUFSIZE);
			if(rslt < 0){
				fprintf(stderr, "%s(%d) recv_all() failed\n", __FILE__, __LINE__);
				exit(-1);
			}
			fwrite(buf, sizeof(char), BUFSIZE, fp);
		}else{
			rslt = recv_all(sock, buf, flen);
			if(rslt < 0){
				fprintf(stderr, "%s(%d) recv_all() failed\n", __FILE__, __LINE__);
				exit(-1);
			}
			fwrite(buf, sizeof(char), flen, fp);
		}
		flen -= BUFSIZE;
	}
	fclose(fp);
	
	if((fp=fopen(file,"rb"))==NULL){
		fprintf(stderr,"error: cannot open %s.\n",file);
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	if(oflen != ftell(fp)){
		fprintf(stderr,"error: %s is broken.\n",file);
		exit(1);
	}
	fclose(fp);
	return(oflen);
}

#ifdef DEBUG_MAIN
int main(int argc,char **argv)
{
	int s = prepSSock();

	std::string file = "test";
	sendFile(s, (char *)file.c_str());

	closeSock(s);

	return(0);
}  
#endif
