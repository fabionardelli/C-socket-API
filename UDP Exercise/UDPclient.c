/* Simple cross-platform client program to show C Socket API operation using UDP protocol
 * and DNS name resolution.
 * It asks the user for a string, than sends the vowels contained in it to the server,
 * which converts them in upper case, and finally prints the result.
 */
 
#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFERSIZE 1024

int isVowel(char c); // checks if a character is a vowel

void ErrorHandler(char * errorMessage)
{
    printf("%s", errorMessage);
}


void ClearWinSock()
{
#if defined WIN32
    WSACleanup();
#endif
}

int main(void)
{
#if defined WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (iResult != 0) {
        printf("error at WSAStartup\n");
        return -1;
    }
#endif

    char serverName[BUFFERSIZE] = ""; // server host name
    short port; // server port
    struct sockaddr_in fromAddr;
    unsigned int fromSize;
    char tmp[BUFFERSIZE] = ""; // buffer to receive a string
    char inputString[BUFFERSIZE] = "";
    int inputStringLen = 0;
    char *helloMsg = "Hi, I'm the client!";
    int msgLen;
    char echoBuffer[BUFFERSIZE] = "";
    int respStringLen;

    // asks the user for the server host name
    puts("Insert server name: ");

    fgets(tmp, BUFFERSIZE, stdin);
    sscanf(tmp, "%[^\n]", serverName);

    // asks the user for the server port number
    do {
        puts("Insert the server's port number: ");

        fgets(tmp, BUFFERSIZE, stdin);
        sscanf(tmp, "%hi", &port);

        if (port < 5000) {
            printf("Invalid port number!\nPort number must be greater than 5000\n");
        }

    } while (port < 5000);


    struct hostent *host;

    if ((host = gethostbyname(serverName)) == NULL) {
        ErrorHandler("gethostbyname() failed.\n");
        return -1;
    }

    struct in_addr *ina = (struct in_addr*) host->h_addr_list[0];
    printf("Server IP: %s\n\n", inet_ntoa(*ina));

    // creates an UDP socket
    int cSocket;
    cSocket = socket(PF_INET, SOCK_DGRAM, 0);

    if (cSocket < 0) {
        ErrorHandler("Socket creation failed.\n");
        closesocket(cSocket);
        ClearWinSock();
        return -1;
    }

    // builds the server address
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = ina->s_addr;
    //sad.sin_addr.s_addr = inet_addr("127.0.0.1");
    sad.sin_port = htons(port);

    msgLen = strlen(helloMsg);
    // sends a message to the server
    if (sendto(cSocket, helloMsg, msgLen, 0, (struct sockaddr*) &sad, sizeof(sad)) != msgLen) {
        ErrorHandler("sendto() sent different number of bytes than expected");
    }

    fromSize = sizeof(fromAddr);
    respStringLen = recvfrom(cSocket, echoBuffer, BUFFERSIZE, 0, (struct sockaddr*) &fromAddr, &fromSize);

    if (sad.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
        ErrorHandler("Error: received a packet from unknown source.\n");
        closesocket(cSocket);
        ClearWinSock();
        return -1;
    }

    echoBuffer[respStringLen] = '\0';
    printf("Received: %s\n", echoBuffer);

    // reads a string from stdin
    do {
        puts("Insert a string: ");
        fgets(tmp, BUFFERSIZE, stdin);
        sscanf(tmp, "%[^\n]", inputString);
        inputStringLen = strlen(inputString);
    } while (inputStringLen == 0);


    // passes through the string and, for each vowel...
    int i;
    char currentVowel[2] = "\0"; // string to contain a vowel
    int vowelLen = 1; // length of a vowel

    for (i = 0; i < inputStringLen; ++i) {
        if (isVowel(inputString[i]) == 1) {
            // sends it to the server...
            currentVowel[0] = inputString[i];
            if (sendto(cSocket, currentVowel, vowelLen, 0, (struct sockaddr*) &sad, sizeof(sad)) != vowelLen) {
                ErrorHandler("sendto() sent different number of bytes than expected");
                closesocket(cSocket);
                ClearWinSock();
                return -1;
            }
            // and receives it back converted in upper case
            respStringLen = recvfrom(cSocket, echoBuffer, vowelLen, 0, (struct sockaddr*) &fromAddr, &fromSize);
            printf("Vowel in upper case: %c\n", echoBuffer[0]);
        }
    }

    // sends a message to signal that there are no more vowels
    char *endMsg = "|";
    int endMsgLen = 1;
    if (sendto(cSocket, endMsg, endMsgLen, 0, (struct sockaddr*) &sad, sizeof(sad)) != endMsgLen) {
        ErrorHandler("sendto() sent different number of bytes than expected");
        closesocket(cSocket);
        ClearWinSock();
        return -1;
    }

    // closes socket
    closesocket(cSocket);
    ClearWinSock();
    printf("\nPress enter to quit...");
    getchar(); // avoids terminal window closing

    return 0;
}

/////////////////////// functions definitions ///////////////////////

// Checks if the character passed as input parameter is a vowel.
// Returns 1 if YES, 0 if NO
int isVowel(char c)
{
    int ans = 0;

    switch (c) {
    case 'A':
    case 'a':

    case 'E':
    case 'e':

    case 'I':
    case 'i':

    case 'O':
    case 'o':

    case 'U':
    case 'u':
        ans = 1;
        break;
    default:
        ;
    }

    return ans;
}