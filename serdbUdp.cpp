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

#define PORT     8082
#define MAXLINE  1000
int sockfd;
using namespace std;
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
    void addConnection(const string& node1, const string& node2) {
        nodes[node1].name = node1;  // Crea el nodo si no existe
        nodes[node2].name = node2;  // Crea el nodo si no existe

        nodes[node1].connections.insert(node2);  // Agrega la conexión
        cout << "Connection created between " << node1 << " and " << node2 << "." << endl;
    }

    // Función privada auxiliar para imprimir conexiones recursivamente
    void printConnectionsRecursive(const string& nodeName, int depth, int maxDepth) const {
        auto it = nodes.find(nodeName);
        if (it != nodes.end()) {
            cout << string(depth * 4, ' ') << it->first << endl;
            if (depth < maxDepth) {
                for (const auto& connection : it->second.connections) {
                    printConnectionsRecursive(connection, depth + 1, maxDepth);
                }
            }
        }
    }

    // Función para imprimir las conexiones de un nodo con recursión limitada
    void printConnections(const string& nodeName, int recursionDepth) const {
        printConnectionsRecursive(nodeName, 0, recursionDepth);
    }

    // Función para imprimir las conexiones de un nodo
    void printConnections(const string& nodeName) const {
        printConnections(nodeName, 1);  // Llamada predeterminada sin recursión
    }

    // Función para actualizar el nombre de un nodo
    void updateConnection(const string& oldNode, const string& newNode) {
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

            cout << "Connection updated: " << oldNode << " is now " << newNode << "." << endl;
        } else {
            cout << "The node " << oldNode << " does not exist." << endl;
        }
    }

    // Función para eliminar un nodo y sus conexiones
    void deleteConnection(const string& node) {
        auto it = nodes.find(node);
        if (it != nodes.end()) {
            // Elimina el nodo y sus conexiones en otros nodos
            for (const auto& connection : it->second.connections) {
                nodes[connection].connections.erase(node);
            }
            nodes.erase(it);
            cout << "Node " << node << " and its connections deleted." << endl;
        } else {
            cout << "The node " << node << " does not exist." << endl;
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

int main() {
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
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        const char* ACK = createACK(payload_size, data_size, sequenceNumber);

        sendto(sockfd, ACK, strlen(ACK), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);

        
    }

        return 0;
}

