#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <iostream>
#include <string.h>
#include "stdio.h"
#include <chrono> 
#include <thread> 
#include <vector> 

#include <map>
#include <set>
#include <string>
#include <cstring>

#include <iomanip>
#include <sstream>


#define PORT     8085
#define MAXLINE  1000
int sockfd;
using namespace std;
void initializePayload(char* payload, int size) 
{
    memset(payload, '?', size);
}

void formatNumber(int number, int size, char* Num) {
    ostringstream ss;
    ss << setw(size) << setfill('0') << number;
    string result = ss.str();

    strcpy(Num, result.c_str());
}

struct Node {
    string name;
    set<string> connections;  // Conjunto de nodos conectados
};

// Clase que representa el grafo
class Graph {
private:
    map<string, Node> nodes;  // Mapa que asocia el nombre de un nodo con su información

public:
    // Función para agregar una conexión entre dos nodos
    string addConnection(const std::string& node1, const std::string& node2) {
        nodes[node1].name = node1;  // Crea el nodo si no existe
        nodes[node2].name = node2;  // Crea el nodo si no existe

        nodes[node1].connections.insert(node2);  // Agrega la conexión

        ostringstream resultStream;
        resultStream << "Connection created between " << node1 << " and " << node2 << ".";
        return resultStream.str();
    }

    // Función privada auxiliar para imprimir conexiones recursivamente
    string printConnectionsRecursive(const string& nodeName, int depth, int maxDepth) const {
        ostringstream resultStream;
        auto it = nodes.find(nodeName);
        if (it != nodes.end()) {
            resultStream << it->first << " ";
            if (depth < maxDepth) {
                for (const auto& connection : it->second.connections) {
                    resultStream << printConnectionsRecursive(connection, depth + 1, maxDepth);
                }
            }
        }
        return resultStream.str();
    }

    // Función para imprimir las conexiones de un nodo con recursión limitada
    string printConnections(const string& nodeName, int recursionDepth) const 
    {
        return printConnectionsRecursive(nodeName, 0, recursionDepth);
    }

    // Función para actualizar el nombre de un nodo
    string updateConnection(const string& oldNode, const string& newNode) {
        auto oldNodeIt = nodes.find(oldNode);
        auto newNodeIt = nodes.find(newNode);

        if (oldNodeIt != nodes.end()) {
            // El nodo antiguo existe
            Node& oldNodeData = oldNodeIt->second;

            if (newNodeIt == nodes.end()) {
                // El nuevo nodo no existe, simplemente actualiza el nombre
                oldNodeData.name = newNode;
                nodes[newNode] = oldNodeData;
            } else {
                // El nuevo nodo ya existe, actualiza conexiones
                Node& newNodeData = newNodeIt->second;

                // Transfiere conexiones desde el antiguo nodo al nuevo nodo
                newNodeData.connections.insert(oldNodeData.connections.begin(), oldNodeData.connections.end());

                // Actualiza conexiones en otros nodos
                for (const auto& connection : oldNodeData.connections) {
                    Node& connectedNode = nodes[connection];
                    connectedNode.connections.erase(oldNode);
                    connectedNode.connections.insert(newNode);
                }

                // Borra el antiguo nodo
                nodes.erase(oldNodeIt);
            }
            ostringstream resultStream;
            resultStream << "Connection updated: " << oldNode << " is now " << newNode << ".";
            return resultStream.str();
        } else {
            ostringstream resultStream;
            resultStream << "The node " << oldNode << " does not exist.";
            return resultStream.str();
        }
    }

    // Función para eliminar un nodo y sus conexiones
    string deleteConnection(const string& node) {
        auto it = nodes.find(node);
        if (it != nodes.end()) {
            // Elimina el nodo y sus conexiones en otros nodos
            for (const auto& connection : it->second.connections) {
                nodes[connection].connections.erase(node);
            }
            nodes.erase(it);
            ostringstream resultStream;
            resultStream << "Node " << node << " and its connections deleted.";
            return resultStream.str();
        } else {
            ostringstream resultStream;
            resultStream << "The node " << node << " does not exist.";
            return resultStream.str();
        }
    }
};

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

string getPayload(const char* Data)
{
    char numeroStr[5];
    strncpy(numeroStr, Data + 11, 4);
    numeroStr[4] = '\0';

    int numeroCaracteres = atoi(numeroStr);

    const char* inicioPayload = Data + 15;

    string payload(inicioPayload, numeroCaracteres);

    return payload;
}

void getValues(const std::string& entrada, std::string& a, std::string& b, std::string& c) {

    // Obtener la primera subcadena (a)
    size_t longitudA = 1;
    a = entrada.substr(0, longitudA);
    // Obtener la segunda subcadena (b)
    size_t longitudB = std::stoi(entrada.substr(1, 3));
    b = entrada.substr(4, longitudB);
    // Obtener la tercera subcadena (c)
    if (a != "R")
    {    
        size_t longitudC = std::stoi(entrada.substr(4 + longitudB, 3));
        c = entrada.substr(4 + longitudB + 3, longitudC);
    }
    else
    {
        size_t longitudC = std::stoi(entrada.substr(4 + longitudB, 1));
        c = entrada.substr(4 + longitudB, longitudC);
    }
}

void getValuesD(const std::string& entrada, std::string& a, std::string& b) {

    // Obtener la primera subcadena (a)
    size_t longitudA = 1;
    a = entrada.substr(0, longitudA);
    // Obtener la segunda subcadena (b)
    size_t longitudB = std::stoi(entrada.substr(1, 3));
    b = entrada.substr(4, longitudB);
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
int main() {
    Graph DB;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
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

    int  n;

    unsigned int len = sizeof(cliaddr);

    int sequenceNumber = 0;
	const int payload_size = 975;
    const int data_size = 1000;

    while(1)
    {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        cout << "3. Receiving query from master." << endl;

        const char* ACK = createACK(payload_size, data_size, sequenceNumber);

        sendto(sockfd, ACK, strlen(ACK), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
        cout << "4. Sending ACK to master." << endl;

        string returnmsg;
        if (getPayload(buffer)[0] != 'D')
        {
            string a, b, c;
            getValues(getPayload(buffer), a, b, c);
            if (a[0] == 'C')
            {
                returnmsg = DB.addConnection(b,c);
            }
            else if (a[0] == 'R')
            {
                cout << "Received 'R' query. Parameters: " << b << " " << c << endl;
                //int d = stoi(c);
                returnmsg = DB.printConnections(b, 1);
                cout << returnmsg<< endl;
            }
            else if (a[0] == 'U')
            {
                returnmsg = DB.updateConnection(b,c);
            }
        }
        else
        {
            string a, b;
            getValuesD(getPayload(buffer), a, b);
            returnmsg = DB.deleteConnection(b);
        }
        const char* response = createData(payload_size, data_size, returnmsg.c_str(), sequenceNumber);
        sendto(sockfd, response, data_size, MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
        cout << "5. Sending response to master." << response <<endl;
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        cout << "6. Receiving ACK from master." << endl;
    }
    return 0;
}