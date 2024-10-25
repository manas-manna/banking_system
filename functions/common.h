#ifndef COMMON_FUNCTIONS
#define COMMON_FUNCTIONS

#include <stdio.h>     // Import for `printf` & `perror`
#include <unistd.h>    // Import for `read`, `write & `lseek`
#include <string.h>    // Import for string functions
#include <stdbool.h>   // Import for `bool` data type
#include <sys/types.h> // Import for `open`, `lseek`
#include <sys/stat.h>  // Import for `open`
#include <fcntl.h>     // Import for `open`
#include <stdlib.h>    // Import for `atoi`
#include <errno.h>     // Import for `errno`
#include <crypt.h>

#include "../record-struct/account.h"
#include "../record-struct/customer.h"
#include "../record-struct/transaction.h"
#include "../record-struct/emp.h"

#include "./admin-credentials.h"
#include "./server-constants.h"

// Semaphores are necessary joint account due the design choice I've made
#include <sys/ipc.h>
#include <sys/sem.h>
int semIdentifier;

// Function Prototypes =================================

bool login_handler(int role, int connFD, void** ptr);
bool change_password(int role, int connFD,void** ptr);
bool lock_critical_section(struct sembuf *semOp);
bool unlock_critical_section(struct sembuf *sem_op);

// =====================================================

// Function Definition =================================
bool login_handler(int role, int connFD, void** ptrToUserID)
{
    ssize_t readBytes, writeBytes;            // Number of bytes written to / read from the socket
    char readBuffer[1000], writeBuffer[1000]; // Buffer for reading from / writing to the client
    char tempBuffer[1000];

    int ID;

    bzero(readBuffer, sizeof(readBuffer));
    memset(writeBuffer, 0, sizeof(writeBuffer));

    struct Customer *customer = malloc(sizeof(struct Customer)); // Dynamically allocate memory for Customer
    struct Employee *emp = malloc(sizeof(struct Employee));      // Dynamically allocated Employee already

    // Get login message for respective user type
    if(role == 1) {
        strcpy(writeBuffer, CUSTOMER_LOGIN_WELCOME);
    }
    else if (role == 2) {
        strcpy(writeBuffer, ADMIN_LOGIN_WELCOME);
    }
    else if(role == 3) {
        strcpy(writeBuffer, EMP_LOGIN_WELCOME);
    }
    else {
        //strcpy(writeBuffer, MAN_LOGIN_WELCOME);
    }

    // Append the request for LOGIN ID message
    strcat(writeBuffer, "\n");
    strcat(writeBuffer, LOGIN_ID);

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing WELCOME & LOGIN_ID message to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading login ID from client!");
        return false;
    }

    bool userFound = false;

    if (role == 2)
    {
        if (strcmp(readBuffer, ADMIN_LOGIN_ID) == 0)
            userFound = true;
    }
    else if (role == 1)
    {
        bzero(tempBuffer, sizeof(tempBuffer));
        strcpy(tempBuffer, readBuffer);
        strtok(tempBuffer, "-");
        ID = atoi(strtok(NULL, "-"));

        int customerFileFD = open(CUSTOMER_FILE, O_RDONLY);
        if (customerFileFD == -1)
        {
            perror("Error opening customer file in read mode!");
            return false;
        }

        off_t offset = lseek(customerFileFD, ID * sizeof(struct Customer), SEEK_SET);
        if (offset >= 0)
        {
            struct flock lock = {F_RDLCK, SEEK_SET, ID * sizeof(struct Customer), sizeof(struct Customer), getpid()};

            int lockingStatus = fcntl(customerFileFD, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error obtaining read lock on customer record!");
                return false;
            }

            readBytes = read(customerFileFD, customer, sizeof(struct Customer));  // Read directly into allocated memory
            if (readBytes == -1)
            {
                perror("Error reading customer record from file!");
            }

            lock.l_type = F_UNLCK;
            fcntl(customerFileFD, F_SETLK, &lock);

            if (strcmp(customer->login, readBuffer) == 0)
                userFound = true;

            close(customerFileFD);
        }
        else
        {
            writeBytes = write(connFD, CUSTOMER_LOGIN_ID_DOESNT_EXIST, strlen(CUSTOMER_LOGIN_ID_DOESNT_EXIST));
        }
    }
    else if (role == 3)
    {
        bzero(tempBuffer, sizeof(tempBuffer));
        strcpy(tempBuffer, readBuffer);
        strtok(tempBuffer, "-");
        ID = atoi(strtok(NULL, "-"));

        int empFileFD = open(EMP_FILE, O_RDONLY);
        if (empFileFD == -1)
        {
            perror("Error opening emp file in read mode!");
            return false;
        }
        off_t offset = lseek(empFileFD, ID * sizeof(struct Employee), SEEK_SET);
        if (offset >= 0)
        {
            struct flock lock = {F_RDLCK, SEEK_SET, ID * sizeof(struct Employee), sizeof(struct Employee), getpid()};

            int lockingStatus = fcntl(empFileFD, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error obtaining read lock on emp record!");
                return false;
            }

            readBytes = read(empFileFD, emp, sizeof(struct Employee));
            if (readBytes == -1)
            {
                perror("Error reading emp record from file!");
            }

            lock.l_type = F_UNLCK;
            fcntl(empFileFD, F_SETLK, &lock);

            if (strcmp(emp->login, readBuffer) == 0)
                userFound = true;

            close(empFileFD);
        }
        else
        {
            writeBytes = write(connFD, EMP_LOGIN_ID_DOESNT_EXIST, strlen(EMP_LOGIN_ID_DOESNT_EXIST));
        }
    }

    if (userFound)
    {
        memset(writeBuffer, 0, sizeof(writeBuffer));
        writeBytes = write(connFD, PASSWORD, strlen(PASSWORD));
        if (writeBytes == -1)
        {
            perror("Error writing PASSWORD message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error reading password from the client!");
            return false;
        }

        char hashedPassword[1000];
        strcpy(hashedPassword, readBuffer);  // Assuming plain text password for now
        
        if (role == 2)
        {
            if (strcmp(hashedPassword, ADMIN_PASSWORD) == 0)
                return true;
        }
        else if (role == 1)
        {
            if (strcmp(hashedPassword, customer->password) == 0)
            {
                *ptrToUserID = (void*) customer;  // Store pointer to customer struct
                return true;
            }
        }
        else if (role == 3)
        {
            if (strcmp(hashedPassword, emp->password) == 0)
            {
                *ptrToUserID = (void*) emp;  // Store pointer to employee struct
                return true;
            }
        }

        memset(writeBuffer, 0, sizeof(writeBuffer));
        writeBytes = write(connFD, INVALID_PASSWORD, strlen(INVALID_PASSWORD));
    }
    else
    {
        memset(writeBuffer, 0, sizeof(writeBuffer));
        writeBytes = write(connFD, INVALID_LOGIN, strlen(INVALID_LOGIN));
    }

    return false;
}


bool change_password(int role, int connFD, void** ptr)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], newPassword[1000];
    struct Customer *customer = NULL;
    struct Employee *emp = NULL;

    // Pointer to user object based on the role
    if (role == 1) {
        customer = (struct Customer*) *ptr;
    } else if (role == 3) {
        emp = (struct Employee*) *ptr;
    } else {
        return false; // Invalid role
    }


    // Request old password
    writeBytes = write(connFD, PASSWORD_CHANGE_OLD_PASS, strlen(PASSWORD_CHANGE_OLD_PASS));
    if (writeBytes == -1) {
        perror("Error writing PASSWORD_CHANGE_OLD_PASS message to client!");
        return false;
    }

    // Read old password response
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    readBuffer[strcspn(readBuffer, "\n")] = 0;
    if (readBytes == -1) {
        perror("Error reading old password response from client");
        return false;
    }

    // Check old password
    if ((role == 1 && strcmp(readBuffer, customer->password) == 0) || 
        (role == 3 && strcmp(readBuffer, emp->password) == 0)) {

        // Request new password
        writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS, strlen(PASSWORD_CHANGE_NEW_PASS));
        if (writeBytes == -1) {
            perror("Error writing PASSWORD_CHANGE_NEW_PASS message to client!");
            return false;
        }

        // Read new password response
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error reading new password response from client");
            return false;
        }
        strcpy(newPassword, readBuffer);

        // Reconfirm new password
        writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS_RE, strlen(PASSWORD_CHANGE_NEW_PASS_RE));
        if (writeBytes == -1) {
            perror("Error writing PASSWORD_CHANGE_NEW_PASS_RE message to client!");
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error reading new password re-enter response from client");
            return false;
        }

        // Verify new passwords match
        if (strcmp(readBuffer, newPassword) == 0) {
            // Update password in memory
            if (role == 1) {
                strcpy(customer->password, newPassword);
            } else if (role == 3) {
                strcpy(emp->password, newPassword);
            }

            // Open respective file for updating the record
            int fileDescriptor = (role == 1) ? open(CUSTOMER_FILE, O_WRONLY) : open(EMP_FILE, O_WRONLY);
            if (fileDescriptor == -1) {
                perror("Error opening file for updating password!");
                return false;
            }

            // Calculate offset based on ID
            off_t offset = lseek(fileDescriptor, (role == 1 ? customer->id : emp->id) * (role == 1 ? sizeof(struct Customer) : sizeof(struct Employee)), SEEK_SET);
            if (offset == -1) {
                perror("Error seeking to the record for password update!");
                return false;
            }

            // Lock the record
            struct flock lock = {F_WRLCK, SEEK_SET, offset, (role == 1 ? sizeof(struct Customer) : sizeof(struct Employee)), getpid()};
            int lockingStatus = fcntl(fileDescriptor, F_SETLKW, &lock);
            if (lockingStatus == -1) {
                perror("Error obtaining write lock on the record!");
                return false;
            }

            // Write the updated record back
            writeBytes = write(fileDescriptor, (role == 1 ? (void*)customer : (void*)emp), (role == 1 ? sizeof(struct Customer) : sizeof(struct Employee)));
            if (writeBytes == -1) {
                perror("Error writing updated record back to the file!");
                return false;
            }

            // Unlock the record
            lock.l_type = F_UNLCK;
            fcntl(fileDescriptor, F_SETLK, &lock);
            close(fileDescriptor);

            // Send success message
            writeBytes = write(connFD, PASSWORD_CHANGE_SUCCESS, strlen(PASSWORD_CHANGE_SUCCESS));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

            return true;
        } else {
            // New & reentered passwords don't match
            writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS_INVALID, strlen(PASSWORD_CHANGE_NEW_PASS_INVALID));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        }
    } else {
        // Old password doesn't match
        writeBytes = write(connFD, PASSWORD_CHANGE_OLD_PASS_INVALID, strlen(PASSWORD_CHANGE_OLD_PASS_INVALID));
    }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return false;
}


// =====================================================

#endif