/*
**Author Shailesh
* Stud ID : 00001402507
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include<strings.h>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>

#define PORT 30000
#define ENTRY 10
#define PAID 0XFFFB
#define NOTPAID 0XFFF9
#define NOTEXIST 0XFFFA
struct requestpacket{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t Acc_Per;
	uint8_t segment_No;
	uint8_t length;
	uint8_t technology;
	unsigned int SourceSubscriberNo;
	uint16_t endpacketID;
};

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

// Map to store the contents of the file locally.
struct Map {
	unsigned long subscriberNumber;
	uint8_t technology;
	int status;

};
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
// function to create a packet for response
struct responsepacket generateresponsepacket(struct requestpacket request) {
	struct responsepacket response;
	response.packetID = request.packetID;
	response.clientID = request.clientID;
	response.segment_No = request.segment_No;
	response.length = request.length;
	response.technology = request.technology;
	response.SourceSubscriberNo = request.SourceSubscriberNo;
	response.endpacketID = request.endpacketID;

	return response;
}


void readFile(struct Map map[]) {

	// reading the file and storing it locally
	char line[30];
	int i = 0;
	FILE *fp;

	fp = fopen("Database.txt", "rt");

	if(fp == NULL)
	{
		printf("cannot open file\n");
		return;
	}
	while(fgets(line, sizeof(line), fp) != NULL)
	{
		char * words;

		words = strtok(line," ");

		map[i].subscriberNumber =(unsigned) atol(words);
		words = strtok(NULL," ");

		map[i].technology = atoi(words);
		words = strtok(NULL," ");

		map[i].status = atoi(words);
		i++;
	}
	fclose(fp);
}
// function to check if the subscriber exist and returning the status
int check(struct Map map[],unsigned int subscriberNumber,uint8_t technology) {
	int value = -1;
	for(int j = 0; j < ENTRY;j++) {
		if(map[j].subscriberNumber == subscriberNumber && map[j].technology == technology) {
			return map[j].status;
		}
	}
	return value;
}
int main(int argc, char**argv){
	struct requestpacket request;
	struct responsepacket response;
	struct Map map[ENTRY];
	readFile(map);
	int sockfd,n;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(PORT);
	bind(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
	addr_size = sizeof serverAddr;
	printf("Server started \n");
	for (;;) {
		n = recvfrom(sockfd,&request,sizeof(struct requestpacket),0,(struct sockaddr *)&serverStorage, &addr_size);
		print(request);
		if(request.segment_No == 9) {
			exit(0);
		}

		if(n > 0 && request.Acc_Per == 0XFFF8) {
			// creating the response packet
			response = generateresponsepacket(request);

			int value = check(map,request.SourceSubscriberNo,request.technology);
			if(value == 0) {
				// not paid
				response.type = NOTPAID;
				printf("Subscriber has not paid");
			}
			else if(value == 1) {
				// paid
				printf("Subscriber has paid");
				response.type = PAID;
			}

			else if(value == -1) {
				// does not exist
				printf("Subscriber does not exist");
				response.type = NOTEXIST;
			}
			// sending the response packet
			sendto(sockfd,&response,sizeof(struct responsepacket),0,(struct sockaddr *)&serverStorage,addr_size);
		}
		n = 0;
		printf("-------------------------------------------------------------------------------------------\n");
	}
}
