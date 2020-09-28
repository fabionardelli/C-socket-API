/* Simple cross-platform client program to show C Socket API operation using TCP protocol.
 * It asks the user for an operation code (A = Addition, S = Subtraction, M = Multiplication, D = Division)
 * and two numbers, then sends them to the server and prints the result computed by the server.
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

#define OPSIZE 20 // max operand dim
#define MSGSIZE 43 // max message dim
#define BUFFERSIZE 1024 // receive buffer dim
#define PORT 5193 // server port

void clearwinsock();

int main(void)
{
#if defined WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (iResult != 0) {
        fprintf(stderr, "Error in WSAStartup().\n");
        return -1;
    }
#endif

    // creates a TCP socket
    int c_socket;
    c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (c_socket < 0) {
        fprintf(stderr, "Socket creation failed.\n");
        closesocket(c_socket);
        clearwinsock();
        return -1;
    }

    // builds server address
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost
    sad.sin_port = htons(PORT);

    // asks the server for connection
    if (connect(c_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
        fprintf(stderr, "connect() failed.\n");
        closesocket(c_socket);
        clearwinsock();
        return -1;
    }


    // receives data from the server
    int bytesRcvd = 0;
    char buf[BUFFERSIZE];

    if ((bytesRcvd = recv(c_socket, buf, BUFFERSIZE - 1, 0)) <= 0) {
        fprintf(stderr, "recv() failed or connection terminated.");
        closesocket(c_socket);
        clearwinsock();
        return -1;
    }
    buf[bytesRcvd] = '\0';
    printf("%s\n\n", buf); // prints the message received from the server

    char opcode; // contains an operation code (A = Addition, S = Subtraction, M = Multiplication, D = Division)
    int op1; // 1st operand
    int op2; // 2nd operand

    // temporary buffers to convert operands into strings
    char op1_str[OPSIZE] = "";
    char op2_str[OPSIZE] = "";

    char msg[MSGSIZE + 1] = ""; // message for the server

    puts("Insert an operation code (A, S, M o D): ");
    scanf("%c", &opcode);

    puts("Insert the first operand: ");
    scanf("%d", &op1);
    snprintf(op1_str, OPSIZE, "%d", op1);

    puts("Insert the second operand: ");
    scanf("%d", &op2);
    snprintf(op2_str, OPSIZE, "%d", op2);

    getchar(); // avoids terminal closing

    // builds the message for the server
    msg[0] = opcode; // writes the operation code
    msg[1] = '|'; // separator character

    strcat(msg, op1_str); // copies the first operand
    msg[strlen(msg)] = '|';

    strcat(msg, op2_str); // copies the second operand
    msg[strlen(msg)] = '\0'; // adds the termination character

    // sends the message to the server
    int msglen = strlen(msg);
    if (send(c_socket, msg, msglen, 0) != msglen) {
        fprintf(stderr, "send() sent a different number of bytes than expected.");
        closesocket(c_socket);
        clearwinsock();
        return -1;
    }


    // receives a string from the server
    if ((bytesRcvd = recv(c_socket, buf, BUFFERSIZE - 1, 0)) <= 0) {
        fprintf(stderr, "recv() failed or connection terminated.");
        closesocket(c_socket);
        clearwinsock();
        return -1;
    }
    buf[bytesRcvd] = '\0';

    // prints an error message if the string received from
    // the server is "END CLIENT PROCESS"
    // otherwise prints the string received from the server
    puts("");
    if (strcmp(buf, "END CLIENT PROCESS") == 0) {
        puts("closing connection");
    } else {
        puts(buf);
    }

    // closes connection
    closesocket(c_socket);
    clearwinsock();

    printf("\nPress enter to quit...");
    getchar();

    return 0;
}

void clearwinsock()
{
#if defined WIN32
    WSACleanup();
#endif
}
