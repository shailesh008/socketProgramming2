/*
** Author :- Shailesh
* Stud ID: 00001402507
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<time.h>
#include<stdint.h>
#define PORT 30000
#define PAID 0XFFFB
#define NOTPAID 0XFFF9
#define NOTEXIST 0XFFFA
// Access Permission packet
struct requestpacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t Acc_Per;
	uint8_t segment_No;
	uint8_t length;
	uint8_t technology;
	unsigned int SourceSubscriberNo;
	uint16_t endpacketID;
};

// response packet
struct responsepacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_No;
	uint8_t length;
	uint8_t technology;
	unsigned int SourceSubscriberNo;
	uint16_t endpacketID;
};
// function to print packet contents
void print(struct requestpacket request ) {
	printf(" packetID: %x\n",request.packetID);
	printf("Client id : %hhx\n",request.clientID);
	printf("Access permission: %x\n",request.Acc_Per);
	printf("Segment no : %d \n",request.segment_No);
	printf("length %d\n",request.length);
	printf("Technology %d \n", request.technology);
	printf("Subscriber no: %u \n",request.SourceSubscriberNo);
	printf("end of datapacket id : %x \n",request.endpacketID);


}


// function to initialise the data to the packet
struct requestpacket initialise () {
	struct requestpacket request;
	request.packetID = 0XFFFF;
	request.clientID = 0XFF;
	request.Acc_Per = 0XFFF8;
	request.endpacketID = 0XFFFF;
	return request;

}
// main function
int main(int argc, char**argv){
	struct requestpacket request;
	struct responsepacket response;
	char line[30];
	int i = 1;
	FILE *fp;
	int sockfd,n = 0;
	struct sockaddr_in cliaddr;
	socklen_t addr_size;
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	struct timeval tv;
	tv.tv_sec = 3;  /* 3 Secs Timeout */
	tv.tv_usec = 0;

	// Configuring the socket option to timeout in three second.
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
	int counter = 0;
	if(sockfd < 0) {
		printf("socket failed\n");
	}
	bzero(&cliaddr,sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	cliaddr.sin_port=htons(PORT);
	addr_size = sizeof cliaddr ;
	// Calling the initialise function to load the data in to the packet
	request = initialise();

	// reading input from a file

	fp = fopen("input.txt", "rt");

	if(fp == NULL)
	{
		printf("cannot open file\n");
	}
	while(fgets(line, sizeof(line), fp) != NULL) {
		counter = 0;
		n = 0;
		printf(" \n New Request \n");
		char * words;
		// splitting the line in to words
		words = strtok(line," ");
		request.length = strlen(words);
		request.SourceSubscriberNo = (unsigned) atoi(words);
		words = strtok(NULL," ");
		request.length += strlen(words);
		request.technology = atoi(words);
		words = strtok(NULL," ");
		request.segment_No = i;
		// printing the contents of the packet
		print(request);
		while(n <= 0 && counter < 3) { //  checking if the packet is recieved or not and sends the packet again and threshold value is set as 3
			sendto(sockfd,&request,sizeof(struct requestpacket),0,(struct sockaddr *)&cliaddr,addr_size);
			// receiving the response from the server
			n = recvfrom(sockfd,&response,sizeof(struct responsepacket),0,NULL,NULL);
			if(n <= 0 ) {
				// If no response recieved from server
				printf("Took more than three seconds to receive response from server\n");
				counter ++;
			}
			else if(n > 0) {
				// response recieved block
				printf("Status =");
				if(response.type == NOTPAID) {
					printf("SUBSCRIBER HAS NOT PAID MESSAGE\n");
				}
				else if(response.type == NOTEXIST ) {
					printf("SUBSCRIBER DOES NOT EXIST\n");
				}
				else if(response.type == PAID) {
					printf("PERMITTED TO ACCESS THE NETWORK\n");

				}
			}
		}
		// if server doesn't respond for more than three tries
		if(counter >= 3 ) {
			printf("Server does not respond");
			exit(0);
		}
		i++;
		printf("-------------------------------------------------------------------------------------------------\n");
	}
	fclose(fp);
}
