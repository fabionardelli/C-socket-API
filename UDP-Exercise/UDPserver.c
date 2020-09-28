/* Simple cross-platform client program to show C Socket API operation using UDP protocol
 * and DNS name resolution.
 * It receives a number of vowels from the client and sends them back converted in upper case.
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
#include <string.h>
#include <ctype.h>

#define BUFFERSIZE 1024
#define PORT 5193

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
        ErrorHandler("Error at WSAStartup()\n");
        return 0;
    }
#endif

    int sock;
    struct sockaddr_in echoServAddr;
    struct sockaddr_in echoClntAddr;
    unsigned int cliAddrLen;
    char buf[BUFFERSIZE] = "";
    int recvMsgSize;

    char *okMsg = "OK";
    int msgLen;

    // creates an UDP socket
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        ErrorHandler("socket() failed");
        ClearWinSock();
        return 0;
    }

    // builds server address
    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family= AF_INET;
    echoServAddr.sin_port= htons(PORT);
    echoServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // binds the socket
    if ((bind(sock, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr))) < 0) {
        ErrorHandler("bind() failed");
        closesocket(sock);
        ClearWinSock();
        return 0;
    }

    puts("Server started.\n");

    // handles a client at a time
    while(1) {
        memset(buf, 0, BUFFERSIZE); // initializes buffer
        cliAddrLen = sizeof(echoClntAddr);
        recvMsgSize = recvfrom(sock, buf, BUFFERSIZE, 0, (struct sockaddr*) &echoClntAddr, &cliAddrLen);

        // prints client's host name and IP
        struct hostent *host;
        if ((host = gethostbyaddr((char*) &echoClntAddr.sin_addr, 4, AF_INET)) == NULL) {
            ErrorHandler("gethostbyaddr() failed.\n");
            return -1;
        }

        char *clientName = host->h_name;
        puts("\nClient:");
        printf("%s %s\n%s %s\n\n", "Name: ", clientName, "IP address: ", inet_ntoa(echoClntAddr.sin_addr));
        printf("Client message: %s\n", buf);

        // sends a confirmation message to the client
        msgLen = strlen(okMsg);
        if (sendto(sock, okMsg, msgLen, 0, (struct sockaddr*) &echoClntAddr, sizeof(echoClntAddr)) != msgLen) {
            ErrorHandler("sendto() sent different number of bytes than expected");
            closesocket(sock);
            ClearWinSock();
            return 0;
        }

        // receives vowels from the client and converts them in upper case
        char currentVowel = '\0'; // string to hold a vowel
        int vowelLen = 1; // length of a vowel
        recvMsgSize = 0;

        do {

            memset(buf, 0, BUFFERSIZE); // cleans the buffer
            recvMsgSize = recvfrom(sock, buf, BUFFERSIZE, 0, (struct sockaddr*) &echoClntAddr, &cliAddrLen);
            currentVowel = buf[0];

            // exits the loop if the current character is not a vowel
            if (isVowel(currentVowel) == 0) {
                break;
            }

            printf("Vowel received: %c\n", currentVowel);
            currentVowel = toupper(currentVowel);

            char upperedVowelStr[2] = "";
            upperedVowelStr[0] = currentVowel;
            if (sendto(sock, upperedVowelStr, vowelLen, 0, (struct sockaddr*) &echoClntAddr, sizeof(echoClntAddr)) != vowelLen) {
                ErrorHandler("sendto() sent different number of bytes than expected");
                closesocket(sock);
                ClearWinSock();
                return 0;
            }

        } while (isVowel(currentVowel) == 1);


        puts("");
    }

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