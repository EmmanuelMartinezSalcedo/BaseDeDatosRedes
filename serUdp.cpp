#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <netdb.h>
#include <errno.h>

#include <iostream>
#include "stdio.h"
#include <chrono> 
#include <thread> 
#include <vector> 

#include <cstring>
#include <iomanip>
#include <sstream>

#define PORT 8081
#define PORTSLAVE 8082
#define MAXLINE 1024


int sockfd;
int slaveSockfd;
using namespace std;
void initializePayload(char* payload, int size) 
{
    memset(payload, '_', size);
}
char* getPayload(const char* Data)
{
    char numeroStr[5];
    strncpy(numeroStr, Data + 11, 4);
    numeroStr[4] = '\0';

    int numeroCaracteres = atoi(numeroStr);

    const char* inicioPayload = Data + 15;

    char* payload = new char[numeroCaracteres + 1];

    strncpy(payload, inicioPayload, numeroCaracteres);

    payload[numeroCaracteres] = '\0';

    return payload;
}

int CalculateSpaces(const char* query) {
    int spaces = 0;
    for (int i = 0; query[i] != '\0'; ++i) {
        if (query[i] == ' ') {
            spaces++;
        }
    }
    return spaces;
}

char* createErrorInvalid()
{
    const char* prefix = "E";
    const char* errorCode = "015";
    const char* errorMessage = "Invalid command";

    int totalLength = strlen(prefix) + strlen(errorCode) + strlen(errorMessage) + 1;

    char* error = new char[totalLength];

    strcpy(error, prefix);
    strcat(error, errorCode);
    strcat(error, errorMessage);

    return error;
}

char* createErrorChecksum()
{
    const char* prefix = "E";
    const char* errorCode = "018";
    const char* errorMessage = "Failed. Send Again";

    int totalLength = strlen(prefix) + strlen(errorCode) + strlen(errorMessage) + 1;

    char* error = new char[totalLength];

    strcpy(error, prefix);
    strcat(error, errorCode);
    strcat(error, errorMessage);

    return error;
}

char* createErrorFormat()
{
    const char* prefix = "E";
    const char* errorCode = "014";
    const char* errorMessage = "Invalid Format";

    int totalLength = strlen(prefix) + strlen(errorCode) + strlen(errorMessage) + 1;

    char* error = new char[totalLength];

    strcpy(error, prefix);
    strcat(error, errorCode);
    strcat(error, errorMessage);

    return error;
}

int calculateCheckSum(const char* str) 
{
    int sum = 0;
    int count = 0;
    while (*str != '\0' && *str != '_') {
        sum += static_cast<int>(*str);
        ++str;
        ++count;
    }
    return sum;
}

char* createMenu()
{
    const char* prefix = "M";
    const char* errorCode = "179";
    const char* errorMessage = "\nCommands:\nCreate: C <<Participant 1>> <<Participant 2>>\nRead:   R <<Participant>>\nUpdate: U <<Participant OLD>> <<Participant NEW>>\nDelete: D <<Participant>>\nMenu:   M\nQuit:   Q\n";

    int totalLength = strlen(prefix) + strlen(errorCode) + strlen(errorMessage) + 1;

    char* error = new char[totalLength];

    strcpy(error, prefix);
    strcat(error, errorCode);
    strcat(error, errorMessage);

    return error;
}

char* createQuit()
{
    const char* prefix = "Q";

    int totalLength = strlen(prefix) + 1;

    char* error = new char[totalLength];

    strcpy(error, prefix);

    return error;
}

bool VerifyCheckSum(const char* Payload, const char* Data)
{
    const char* ultimosDigitos = Data + strlen(Data) - 10;

    int checksumData = atoi(ultimosDigitos);

    int checksumPayload = calculateCheckSum(Payload);

    return (checksumData == checksumPayload);
}

void formatNumber(int number, int size, char* Num) {
    ostringstream ss;
    ss << setw(size) << setfill('0') << number;
    string result = ss.str();

    strcpy(Num, result.c_str());
}

const char* createPayload(int payload_full_size, const char* query) 
{
    char* payload = new char[payload_full_size];
    initializePayload(payload, payload_full_size);
    char* query_payload = getPayload(query);
    char* msg = new char[payload_full_size];
    if (VerifyCheckSum(query_payload, query) == 0)
    {
        strncpy(payload, createErrorChecksum(), 23);
    }
    else if (query_payload[0] != 'C' && query_payload[0] != 'c' && query_payload[0] != 'R' && query_payload[0] != 'r' &&
             query_payload[0] != 'U' && query_payload[0] != 'u' && query_payload[0] != 'D' && query_payload[0] != 'd' &&
             query_payload[0] != 'M' && query_payload[0] != 'm' && query_payload[0] != 'Q' && query_payload[0] != 'q') 
    {
        strncpy(payload, createErrorInvalid(), 20);
    }
    else if (query_payload[0] == 'M' || query_payload[0] == 'm') 
    {
        strncpy(payload, createMenu(), 184);
    }
    else if (query_payload[0] == 'Q' || query_payload[0] == 'q') 
    {
        strncpy(payload, createQuit(), 2);
    }
    else if (query_payload[0] == 'C' || query_payload[0] == 'c') 
    {
        if (CalculateSpaces(query_payload) != 2 || query_payload[1] != ' ')
        {
            strncpy(payload, createErrorFormat(), 19);
        }
        else
        {
            char a[1];
            char b[400];
            char c[400];
            sscanf(query_payload, "%s %s %s", a, b, c);
            

            int participantSize = 3;
            int Bsize = strlen(b);
            int Csize = strlen(c);
            char* Bs = new char[participantSize];
            char* Cs = new char[participantSize];
            formatNumber(Bsize, participantSize, Bs);
            formatNumber(Csize, participantSize, Cs);

            strcpy(payload, "C");
            strncat(payload, Bs, participantSize);
            strncat(payload, b, Bsize);
            strncat(payload, Cs, participantSize);
            strncat(payload, c, Csize);
        }
    }
    else if (query_payload[0] == 'R' || query_payload[0] == 'r') 
    {
        if (CalculateSpaces(query_payload) != 2 || query_payload[1] != ' ')
        {
            strncpy(payload, createErrorFormat(), 19);
        }
        else
        {
            char a[1];
            char b[800];
            char c[1];
            sscanf(query_payload, "%s %s %s", a, b, c);
            
            
            int participantSize = 3;
            int Bsize = strlen(b);
            int Csize = strlen(c);
            char* Bs = new char[participantSize];
            formatNumber(Bsize, participantSize, Bs);

            strcpy(payload, "R");
            strncat(payload, Bs, participantSize);
            strncat(payload, b, Bsize);
            strncat(payload, c, Csize);
        }
    }
    else if (query_payload[0] == 'U' || query_payload[0] == 'u') 
    {
        if (CalculateSpaces(query_payload) != 2 || query_payload[1] != ' ')
        {
            strncpy(payload, createErrorFormat(), 19);
        }
        else
        {
            char a[1];
            char b[400];
            char c[400];
            sscanf(query_payload, "%s %s %s", a, b, c);
            

            int participantSize = 3;
            int Bsize = strlen(b);
            int Csize = strlen(c);
            char* Bs = new char[participantSize];
            char* Cs = new char[participantSize];
            formatNumber(Bsize, participantSize, Bs);
            formatNumber(Csize, participantSize, Cs);

            strcpy(payload, "U");
            strncat(payload, Bs, participantSize);
            strncat(payload, b, Bsize);
            strncat(payload, Cs, participantSize);
            strncat(payload, c, Csize);
        }
    }
    else if (query_payload[0] == 'D' || query_payload[0] == 'd') 
    {
        if (CalculateSpaces(query_payload) != 1 || query_payload[1] != ' ')
        {
            strncpy(payload, createErrorFormat(), 19);
        }
        else
        {
            char a[1];
            char b[800];
            sscanf(query_payload, "%s %s", a, b);
            

            int participantSize = 3;
            int Bsize = strlen(b);
            char* Bs = new char[participantSize];
            formatNumber(Bsize, participantSize, Bs);

            strcpy(payload, "D");
            strncat(payload, Bs, participantSize);
            strncat(payload, b, Bsize);
        }
    }

    payload[payload_full_size - 1] = '\0';
    for (int i = 0; i < payload_full_size; ++i) {
        if (payload[i] == '\0') {
            payload[i] = '_';
            break;
        }
    }
    return payload;
}

char* createData(int payload_full_size, int data_size, const char* query, int sequenceNumber)
{
    const char* payload = createPayload(payload_full_size, query);
	
    int seqN_size = 10;
    char* seqN = new char[seqN_size];
    formatNumber(sequenceNumber, seqN_size, seqN);

    int payloadS_size = 4;
    int payloadSize = 0;
    int i = 0;
    while (payload[i] != '_')
    {
        payloadSize++;
        i++;
    }
    char* payloadS = new char[payloadS_size];
    formatNumber(payloadSize, payloadS_size, payloadS);

    int checkS_size = 10;
    int checkSumValue = calculateCheckSum(payload);
    char* checkS = new char[checkS_size];
    formatNumber(checkSumValue, checkS_size, checkS);

    char* data = new char[data_size];
    strcpy(data, "D");
    strncat(data, seqN, seqN_size);
    strncat(data, payloadS, payloadS_size);
    strncat(data, payload, payload_full_size);
    strncat(data, checkS, checkS_size);
    

    // cout << "D|" << seqN << "|" << payloadS << "|" << payload << "|" << checkS << endl;

    return data;
}

const char* createACK(int payload_full_size, int data_size, int sequenceNumber)
{
    char* payload = new char[payload_full_size];
    initializePayload(payload, payload_full_size);
    payload[payload_full_size - 1] = '\0';

    int seqN_size = 10;
    char* seqN = new char[seqN_size];
    formatNumber(sequenceNumber, seqN_size, seqN);

    int payloadS_size = 4;
    int payloadSize = 0;
    char* payloadS = new char[payloadS_size];
    formatNumber(payloadSize, payloadS_size, payloadS);

    int checkS_size = 10;
    int checkSumValue = 0;
    char* checkS = new char[checkS_size];
    formatNumber(checkSumValue, checkS_size, checkS);

    char* data = new char[data_size];
    strcpy(data, "A");
    strncat(data, seqN, seqN_size);
    strncat(data, payloadS, payloadS_size);
    strncat(data, payload, payload_full_size);
    strncat(data, checkS, checkS_size);

    return data;
}
void processClient(struct sockaddr_in cliaddrSocket, int n, unsigned int len, char* buffer, const int payload_size, const int data_size)
{
	int sequenceNumber = 1;

	char* resultado = createData(payload_size, data_size, buffer, sequenceNumber);
	cout << resultado << endl;
	const char* ack = createACK(payload_size, data_size, sequenceNumber);
	
	sendto(sockfd, ack, strlen(ack), MSG_CONFIRM, (const struct sockaddr *)&cliaddrSocket, len);
	sequenceNumber++;

	if (resultado[15] == 'C' || resultado[15] == 'c' || resultado[15] == 'R' || resultado[15] == 'r' ||
		resultado[15] == 'U' || resultado[15] == 'u' || resultado[15] == 'D' || resultado[15] == 'd')
	{
		buffer = resultado;
		cout << endl << buffer << endl;
		return;
		//assas
	}
	else
	{
		sendto(sockfd, resultado, strlen(resultado), MSG_CONFIRM, (const struct sockaddr *)&cliaddrSocket, len);
		sequenceNumber++;
	
		n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddrSocket, &len);
		sequenceNumber++;
	}
}
void processSlave(const struct sockaddr_in slaveaddr, int n, unsigned int len, char *buffer, const int payload_size, const int data_size, int sockfdSlave) {
    int sequenceNumber = 1;
	sendto(sockfdSlave, buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&slaveaddr, len);
    
	n = recvfrom(sockfdSlave, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &slaveaddr, &len);

    
}

int main() 
{

	struct sockaddr_in servaddr, cliaddr;
	
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);

	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	//---------------
	int sockfdSlave;
	struct hostent *host;
	struct sockaddr_in servSlaveaddr;

	host = (struct hostent *)gethostbyname((char *)"127.0.0.1");

	if ( (sockfdSlave = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
			perror("socket creation failed");
			exit(EXIT_FAILURE);
	}

	memset(&servSlaveaddr, 0, sizeof(servSlaveaddr));

	servSlaveaddr.sin_family = AF_INET;
	servSlaveaddr.sin_port = htons(PORTSLAVE);
	servSlaveaddr.sin_addr = *((struct in_addr *)host->h_addr);
	//-------------------

	char buffer[MAXLINE];

	int  n;

	unsigned int len = sizeof(cliaddr);

	const int payload_size = 975;
    const int data_size = 1000;

	while(1)
	{
		n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
		buffer[n] = '\0';
		thread (processClient, cliaddr, n, len, buffer, payload_size, data_size).detach();

		if (buffer[15] == 'C' || buffer[15] == 'c' || buffer[15] == 'R' || buffer[15] == 'r' ||
		buffer[15] == 'U' || buffer[15] == 'u' || buffer[15] == 'D' || buffer[15] == 'd')
		{
            thread(processSlave, servSlaveaddr, ref(n), ref(len), buffer, payload_size, data_size, sockfdSlave).detach();
        }
	}
	return 0;
}

//C P1 P2
//R P NUMERO
//U PO PN
//D P

//