#include <iostream>
#include <cstring>
#include <iomanip>
#include <sstream>

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
    cout << endl << totalLength << endl;
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
            sscanf(query_payload, "%s %s %s", a, b);
            

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
int getSequenceNumber(const char* input)
{
    char sequenceNumberStr[10];
    strncpy(sequenceNumberStr, input + 1, 9);
    sequenceNumberStr[9] = '\0';

    int sequenceNumber = atoi(sequenceNumberStr);
    sequenceNumber++;
    return sequenceNumber;
}
const char* createData(int payload_full_size, int data_size, const char* query, int &sequenceNumber)
{
    const char* payload = createPayload(payload_full_size, query);

    sequenceNumber = getSequenceNumber(query);
    cout << sequenceNumber << endl;
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

int main() 
{
    const int payload_size = 975;
    const int data_size = 1000;
    char query[data_size];
    cin.getline(query, sizeof(query));

    int sequenceNumber;
    const char* resultado = createData(payload_size, data_size, query, sequenceNumber);
    cout << endl << resultado << endl;
    return 0;
}