#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include "stdio.h"
#include <chrono> 
#include <thread> 
#include <vector>

#include <cstring>
#include <iomanip>
#include <sstream>
#include <fstream>

#define PORT    8081
#define MAXLINE 1000

using namespace std;
void initializePayload(char* payload, int size) 
{
    memset(payload, '?', size - 1);
}

const char* createPayload(int payload_full_size, const char* query) 
{
    char* payload = new char[payload_full_size];
    initializePayload(payload, payload_full_size);
    strcpy(payload, query);
    payload[payload_full_size - 1] = '\0';
    for (int i = 0; i < payload_full_size; ++i) {
        if (payload[i] == '\0') {
            payload[i] = '?';
            break;
        }
    }

    return payload;
}

void formatNumber(int number, int size, char* Num) {
    ostringstream ss;
    ss << setw(size) << setfill('0') << number;
    string result = ss.str();

    strcpy(Num, result.c_str());
}

int calculateCheckSum(const char* str) 
{
    int sum = 0;
    int count = 0;
    while (*str != '\0') {
        sum += static_cast<int>(*str);
        ++str;
        ++count;
    }
    return sum;
}

const char* createData(int payload_full_size, int data_size, const char* query, int sequenceNumber)
{
    const char* payload = createPayload(payload_full_size, query);

    int seqN_size = 10;
    char* seqN = new char[seqN_size];
    formatNumber(sequenceNumber, seqN_size, seqN);

    int payloadS_size = 4;
    int payloadSize = strlen(query);
    char* payloadS = new char[payloadS_size];
    formatNumber(payloadSize, payloadS_size, payloadS);

    int checkS_size = 10;
    int checkSumValue = calculateCheckSum(query);
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

const char* getPayload(const char* Data)
{
    char numeroStr[5];
    strncpy(numeroStr, Data + 11, 4); //copia payloadsize
    numeroStr[4] = '\0';

    int numeroCaracteres = atoi(numeroStr);

    const char* inicioPayload = Data + 15;

    char* payload = new char[numeroCaracteres + 1];

    strncpy(payload, inicioPayload, numeroCaracteres);

    payload[numeroCaracteres] = '\0';

    return payload;
}

bool VerifyCheckSum(const char* Payload, const char* Data)
{
    const char* ultimosDigitos = Data + strlen(Data) - 10;

    int checksumData = atoi(ultimosDigitos);

    int checksumPayload = calculateCheckSum(Payload);

    return (checksumData == checksumPayload);
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

    // cout << "A|" << seqN << "|" << payloadS << "|" << payload << "|" << checkS << endl;

    return data;
}
int main() 
{
	int sockfd;
	char buffer[MAXLINE];
	struct hostent *host;
	struct sockaddr_in servaddr;

	host = (struct hostent *)gethostbyname((char *)"127.0.0.1");

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr = *((struct in_addr *)host->h_addr);

	int n;

	unsigned int len = sizeof(servaddr);

	int sequenceNumber = 0;
	const int payload_size = 975;
    const int data_size = 1000;
	cout << "\nCommands:\nCreate: C <<Participant 1>> <<Participant 2>>\nRead:   R <<Participant>>\nUpdate: U <<Participant OLD>> <<Participant NEW>>\nDelete: D <<Participant>>\nMenu:   M\nQuit:   Q\n";

    ifstream inputFile("Flavor_network2.txt");

    if (!inputFile.is_open()) 
    {
        cerr << "Error al abrir el archivo de comandos." << endl;
        return 1;
    }

    string line;
    // while (getline(inputFile, line)) 
    // {
    //     const char* resultado = createData(payload_size, data_size, line.c_str(), sequenceNumber);

    //     sendto(sockfd, resultado, strlen(resultado), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    //     n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    //     sequenceNumber++;

    //     n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    //     sequenceNumber++;

    //     const char* ack = createACK(payload_size, data_size, sequenceNumber);
    // }

    // Cierra el archivo

	while (1) {

        fgets(buffer, MAXLINE, stdin);
        
        if (buffer[0] == 'i')
        {

            while (getline(inputFile, line)) 
            {

                const char* resultado = createData(payload_size, data_size, line.c_str(), sequenceNumber);

                sendto(sockfd, resultado, strlen(resultado), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));

                n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
                sequenceNumber++;

                n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
                sequenceNumber++;

                const char* ack = createACK(payload_size, data_size, sequenceNumber);
            }
            inputFile.close();
        }
        else
        {
            const char* resultado = createData(payload_size, data_size, buffer, sequenceNumber);

            sendto(sockfd, resultado, strlen(resultado), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
            //  cout << "1. Sending query to server." << endl;
            //cout << resultado;

            n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
            // cout << "2. Receiving ACK from server." << endl;
            sequenceNumber++;
            
            n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
            // cout << "7. Receiving response from server." << endl;
            sequenceNumber++;
            
            //cout << getPayload(buffer) << endl;
            //const char* ack = createACK(payload_size, data_size, sequenceNumber);

            //sendto(sockfd, ack, strlen(ack), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
            //cout << "8. Sending ACK to server." << endl;
            //sequenceNumber++;

            cout << getPayload(buffer) << endl;
            if (getPayload(buffer)[0] == 'Q')
            {
                break;
            }
        }
    }
	close(sockfd);
	return 0;
}