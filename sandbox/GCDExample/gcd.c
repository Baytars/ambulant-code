#include <stdio.h>
#include <sys/time.h>
#include <dispatch/dispatch.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "12345" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, const char * argv[])
{
	
	struct timeval start, end;
	long mtime, seconds, useconds;
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    dispatch_group_t group = dispatch_group_create();
    
    FILE *devNull = fopen("/dev/null", "a");

    const int NumConcurrentBlocks = 20;
    const int NumLoopsPerBlock = 100000;

	int i;
    for (i = 0; i < NumConcurrentBlocks; i++)
    {
 
        dispatch_group_async(group, queue, ^{
            int sockfd, numbytes;  
		    char buf[MAXDATASIZE];
		    struct addrinfo hints, *servinfo, *p;
		    int rv;
		    char s[INET6_ADDRSTRLEN];
		
		    if (argc != 2) {
		        fprintf(stderr,"usage: client hostname\n");
		        exit(1);
		    }
		
		    memset(&hints, 0, sizeof hints);
		    hints.ai_family = AF_UNSPEC;
		    hints.ai_socktype = SOCK_STREAM;
		
		    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		        return ;
		    }
		
		    // loop through all the results and connect to the first we can
		    for(p = servinfo; p != NULL; p = p->ai_next) {
		        if ((sockfd = socket(p->ai_family, p->ai_socktype,
		                p->ai_protocol)) == -1) {
		            perror("client: socket");
		            continue;
		        }
		
		        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
		            close(sockfd);
		            perror("client: connect");
		            continue;
		        }
		
		        break;
		    }
		
		    if (p == NULL) {
		        fprintf(stderr, "client %d: failed to connect\n", i);
		        return ;
		    }
		
		    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
		            s, sizeof s);
		    printf("client %d: connecting to %s\n", i, s);
		
		    freeaddrinfo(servinfo); // all done with this structure
		    
		   if (send(sockfd, "Hello, world!", 13, 0) == -1) {
		        perror("send");
		        exit(1);
		   }
		
		    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		        perror("recv");
		        exit(1);
		    }
		
		    buf[numbytes] = '\0';
		
		    printf("client %d: received '%s'\n", i,buf);
		
		    close(sockfd);
        });

    }


    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    dispatch_release(group);

    fclose(devNull);
    
    return 0;
}
