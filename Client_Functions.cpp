#include <iostream>
#include <cstring>
#include <iomanip>
#include <sstream>

using namespace std;

void initializePayload(char* payload, int size) 
{
    memset(payload, '_', size - 1);
}

const char* createPayload(int payload_full_size, const char* query) 
{
    char* payload = new char[payload_full_size];
    initializePayload(payload, payload_full_size);
    strcpy(payload, query);
    payload[payload_full_size - 1] = '\0';
    for (int i = 0; i < payload_full_size; ++i) {
        if (payload[i] == '\0') {
            payload[i] = '_';
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
    strncpy(numeroStr, Data + 11, 4);
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
    const int payload_size = 975;
    const int data_size = 1000;
    char query[payload_size];
    cin.getline(query, sizeof(query));

    int sequenceNumber = 0;
    const char* resultado = createData(payload_size, data_size, query, sequenceNumber);
    cout << endl << resultado << endl;
    cout << getPayload(resultado) << endl;
    const char* ack = createACK(payload_size, data_size, sequenceNumber);
    cout << ack << endl;
    return 0;
}