#include <stdio.h> 
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>


#include "./functions/server-constants.h"
#include "./functions/admin.h"
#include "./functions/customer.h"
#include "./functions/employee.h"

int connection_handler(int connFD); // Handles client connection

int main()
{
    int socketFileDescriptor, socketBindStatus, socketListenStatus, connectionFileDescriptor;
    struct sockaddr_in serverAddress, clientAddress;

    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor == -1)
    {
        perror("Error while creating server socket!");
        exit(0);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    socketBindStatus = bind(socketFileDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (socketBindStatus == -1)
    {
        perror("Error while binding to server socket!");
        exit(0);
    }

    socketListenStatus = listen(socketFileDescriptor, 10);
    if (socketListenStatus == -1)
    {
        perror("Error while listening for connections on the server socket!");
        close(socketFileDescriptor);
        exit(0);
    }
    printf("Server listening at socket 8080...\n");

    int clientSize;
    while (1)
    {
        clientSize = (int)sizeof(clientAddress);
        connectionFileDescriptor = accept(socketFileDescriptor, (struct sockaddr *)&clientAddress, &clientSize);
        if (connectionFileDescriptor == -1)
        {
            perror("Error while connecting to client!");
            close(socketFileDescriptor);
        }
        else
        {            
            if (!fork())
            {
                // Child will enter this branch
                connection_handler(connectionFileDescriptor);
                close(connectionFileDescriptor);
                exit(0);
            }
        }
    }

    close(socketFileDescriptor);
    return 0;
}

int connection_handler(int connectionFileDescriptor)
{
    printf("Client connected to server!\n");

    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;
    int userChoice, home=1;
    do{
        writeBytes = write(connectionFileDescriptor, INITIAL_PROMPT, strlen(INITIAL_PROMPT));
        if (writeBytes == -1){
            perror("Error while sending first prompt to the user!");
        }
        else
        {
            memset(readBuffer, 0, sizeof(readBuffer));
            readBytes = read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
                perror("Error while reading from client");
            else if (readBytes == 0)
                printf("No data was sent by the client\n");
            else
            {
                userChoice = atoi(readBuffer);
                switch (userChoice)
                {
            case 1:
                // Customer Handler
                customer_operation_handler(connectionFileDescriptor);
                break;
            case 2:
                // Admin Handler
                admin_operation_handler(connectionFileDescriptor);
                break;
            case 3:
                // Employee Handler
                emp_operation_handler(connectionFileDescriptor);
                break;
            case 4:
                // Manager Handler
                //man_operation_handler(connectionFileDescriptor);
                break;
            default:
                // Exiting
                home=0;
                writeBytes = write(connectionFileDescriptor, "*", strlen("*"));
                if (writeBytes == -1)
                {
                     perror("Error while writing to client socket!");
                     break;
                }
            }
                read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
            }
        }
    }while(home);
    
    printf("Terminating connection to client!\n");
    return 0;
}
