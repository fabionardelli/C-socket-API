/* Simple cross-platform server program to show C Socket API operation using TCP protocol.
 * It computes an operation (addition, subtraction, multiplication or division) as requested from
 * the client and sends back the result.
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
#include <ctype.h>
#define PORT 5193 // port
#define QLEN 6 // request queue
#define BUFFERSIZE 1024 // receive buffer dim
#define ANSSIZE 80 // max size for the answer to send to the client
#define OPSIZE 20 // max operand dim


void clearwinsock();
void compute_result(char *answermsg, const char *buf);

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
    int server_socket;
    server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0) {
        fprintf(stderr, "Socket creation failed.\n");
        clearwinsock();
        return -1;
    }

    // assigns an address to the socket
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost
    sad.sin_port = htons(PORT);

    // binds IP address and port number to the socket
    if (bind(server_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
        fprintf(stderr, "bind() failed.\n");
        closesocket(server_socket);
        clearwinsock();
        return -1;
    }

    // sets the socket to listen
    if (listen (server_socket, QLEN) < 0) {
        fprintf(stderr, "listen() failed.\n");
        closesocket(server_socket);
        clearwinsock();
        return -1;
    }

    // accepts a new connection request from a client
    struct sockaddr_in cad;
    int client_socket; // temporary socket
    int client_len;
    char *welcomemsg = "Connection established.";
    puts("Waiting for connection...");

    int bytes_rcvd = 0;
    char buf[BUFFERSIZE + 1];
    char answermsg[ANSSIZE + 1] = ""; // answer for the client
    int str_len;

    while(1) {
        memset(buf, 0, BUFFERSIZE); // initializes buffer
        client_len = sizeof(cad);

        if ((client_socket = accept(server_socket,
                (struct sockaddr*) &cad, &client_len)) < 0) {
            fprintf(stderr, "accept() failed.\n");
            closesocket(server_socket);
            clearwinsock();
            return 0;
        }
        printf("\nClient connected: %s\n", inet_ntoa(cad.sin_addr));

        // sends a confirmation message to the client
        str_len = strlen(welcomemsg);
        if(send(client_socket, welcomemsg, str_len, 0) != str_len) {
            fprintf(stderr, "send() sent a different number of bytes than expected.");
            closesocket(client_socket);
            clearwinsock();
            return -1;
        }

        // receives a string from the client
        if ((bytes_rcvd = recv(client_socket, buf, BUFFERSIZE, 0)) <= 0) {
            fprintf(stderr, "recv() failed or connection terminated.");
            closesocket(client_socket);
            clearwinsock();
            return -1;
        }

        // computes the result and builds a message for the client
        compute_result(answermsg, buf);

        // sends the message to the client
        str_len = strlen(answermsg);
        if(send(client_socket, answermsg, str_len, 0) != str_len) {
            fprintf(stderr, "send() sent a different number of bytes than expected.");
            closesocket(client_socket);
            clearwinsock();
            return -1;
        }
        puts("");
    }

    return 0;
}

/////////////////////// functions definitions ///////////////////////
void clearwinsock()
{
#if defined WIN32
    WSACleanup();
#endif
}

void compute_result(char *answermsg, const char *buf)
{
    // gets data from the received string
    char opcode = buf[0];

    // temporary buffers to convert the operands to integers
    char op1_str[OPSIZE] = "";
    char op2_str[OPSIZE] = "";

    int op1;
    int op2;

    int i, j;

    // gets the substring containing the first operand
    for (i = 0, j = 2; buf[j] != '|'; ++i, ++j) {
        op1_str[i] = buf[j];
    }

    // gets the substring containing the second operand
    for (i = 0, j = j + 1; buf[j] != '\0'; ++i, ++j) {
        op2_str[i] = buf[j];
    }

    // converts the above strings to integers
    op1 = strtol(op1_str, NULL, 0);
    op2 = strtol(op2_str, NULL, 0);

    char tmp[OPSIZE + 1] = "";

    // computes the requested operation
    switch(opcode) {
    case 'A':
    case 'a':
        memcpy(answermsg, "Addition result: ", 18);
        snprintf(tmp, OPSIZE,"%d", op1 + op2);
        strcat(answermsg, tmp);
        break;
    case 'S':
    case 's':
        memcpy(answermsg, "Subtraction result: ", 21);
        snprintf(tmp, OPSIZE,"%d", op1 - op2);
        strcat(answermsg, tmp);
        break;
    case 'M':
    case 'm':
        memcpy(answermsg, "Multiplication result: ", 24);
        snprintf(tmp, OPSIZE,"%d", op1 * op2);
        strcat(answermsg, tmp);
        break;
    case 'D':
    case 'd':
        if (op2 != 0) {
            memcpy(answermsg, "Division result: ", 18);
            snprintf(tmp, OPSIZE,"%.2f", (float)op1 / op2);
            strcat(answermsg, tmp);
        } else {
            memcpy(answermsg, "Error: attempt to divide by 0!", 31);
        }
        break;
    default:
        memcpy(answermsg, "END CLIENT PROCESS", 19);
        break;
    }
}
