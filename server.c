#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>

#define BACKLOG 10 
#define MAXDATASIZE 1000 // max number of bytes we can get at once 
#define MAXBUFLEN 1000

struct packet { 
unsigned int total_frag; 
unsigned int frag_no; 
unsigned int size; 
char* filename;
char filedata[1000]; 
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
 if (sa->sa_family == AF_INET) {
 return &(((struct sockaddr_in*)sa)->sin_addr);
 }
 return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int extractpacket(char*input, struct packet *extracted ) {

char*copy;//need to initialize?
int i=0, j=0, k;
for(k=0; k++; k<4){
	while(input[i]!=':'){
		copy[j]=input[i];
		i++;
		j++;
	}//exit, input[i]=":"
	i++;
	j=0;
	if(k=0)
		extracted->total_frag = atoi(copy);
	else if (k=1)
		extracted->frag_no = atoi(copy);
	else if (k=2)
		extracted->size = atoi(copy);
	else if (k=3)
		strncpy ( extracted->filename, copy, sizeof(extracted->filename) );//how big is extracted.filename?
	else
	printf("what?");
}
for (k=0; k++; k<extracted->size){
	extracted->filedata[k]=input[i];
}

return 0;
}


int main(int argc, char *argv[]) {
			
FILE *serverfile;
										
int status;
int sockfd, new_fd, numbytes; 
char buf[MAXDATASIZE];

char* listen_port;
struct addrinfo hints;
struct addrinfo *res, *p; // will point to the results

struct sockaddr_storage their_addr;
socklen_t addr_size;
size_t addr_len;
char s[INET6_ADDRSTRLEN];
int yes=1;

listen_port=argv[0];

// first, load up address structs with getaddrinfo():
memset(&hints, 0, sizeof hints); // make sure the struct is empty
hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
hints.ai_socktype = SOCK_DGRAM; // UDP datagram sockets
hints.ai_flags = AI_PASSIVE; // fill in my IP for me


if ((status = getaddrinfo(NULL, listen_port, &hints, &res)) != 0) {
 fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
 exit(1);
}
// res now points to a linked list of 1 or more struct addrinfos

// [should do error-checking on getaddrinfo(), and walk
// the "res" linked list looking for valid entries 
// make a socket: 
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
if (sockfd < 0) {
    printf("Error creating socket.\n");	
    exit(EXIT_FAILURE);
  }

// Allow listening port to be reused if defunct.
// lose the pesky "Address already in use" error message
if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
 printf("Error configuring socket.\n");
 exit(1);
} 

// bind it to the port we passed in to getaddrinfo():
status= bind(sockfd, res->ai_addr, res->ai_addrlen);
if (status != 0) {
    printf("Error binding socket.\n");
    exit(EXIT_FAILURE);
  }

/* Doesn't apply for UDP?
// Listen for connections. 
status = listen(sockfd, BACKLOG);
if (status != 0) {
    printf("Error listening on socket.\n");
    exit(EXIT_FAILURE);
}
*/
 printf("server: waiting for connections...\n");
int waiting=1; 

while(waiting){
	addr_len = sizeof their_addr;

	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
	 (struct sockaddr *)&their_addr, &addr_len)) == -1) {
		printf("Error recvfrom.\n");
		exit(1);
	 }
	//recived packet 
	if(numbytes==0){//EOF packet, file stream should be closed.
		// Close the connection with the client.
		fclose(serverfile);
		close(new_fd);
		waiting=0;
	}
	else{//not EOF
		printf("listener: got packet from %s\n",inet_ntop(their_addr.ss_family, 
	get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
		printf("listener: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("listener: packet contains \"%s\"\n", buf);
		struct packet extracted; 
		extractpacket(buf, &extracted); 
		if(extracted.frag_no ==1)
		{
			/*read the file name from the packet and create a corresponding file stream*/
			serverfile = fopen(extracted.filename, "w+");
		}
		//Data read from packets, written to file stream
		for (int i=0; i++; i<extracted.size)
		{
			fputc( extracted.filedata[i], serverfile );
		}

		//send ACK
		if ((numbytes = sendto(sockfd, "ACK", strlen("ACK"), 0, their_addr, addr_len)) == -1) {
 			printf("Error sending ACK.\n");
 			exit(1);
		}
	}
}

// ... do everything until you don't need res anymore ....
freeaddrinfo(res); // free the linked-list


return 0;
}
