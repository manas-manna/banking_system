#ifndef ADMIN_FUNCTIONS
#define ADMIN_FUNCTIONS

#include <sys/ipc.h>
#include <sys/sem.h>
#include "./common.h"

bool admin_operation_handler(int connFD);
bool add_account(int connFD);
int add_employee(int connFD);
bool modify_emp_info(int connFD);
//bool modify_user_role(int connFD);

bool admin_operation_handler(int connFD)
{

    if (login_handler(2, connFD, NULL))
    {
        ssize_t writeBytes, readBytes;
        char readBuffer[1000], writeBuffer[1000];
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, ADMIN_LOGIN_SUCCESS);
        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, ADMIN_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing ADMIN_MENU to client!");
                return false;
            }
            memset(writeBuffer, 0, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for ADMIN_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            switch (choice)
            {
            case 1:
                add_employee(connFD);
                break;
            case 2:
                modify_emp_info(connFD);
                break;
            case 3: 
                //manage_user_roles(connFD);
                break;
            case 4:
                //change_password
                break;
            default:
                writeBytes = write(connFD, ADMIN_LOGOUT, strlen(ADMIN_LOGOUT));
                return false;
            }
        }
    }
    else
    {
        // ADMIN LOGIN FAILED
        return false;
    }
    return true;
}

int add_employee(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Employee newEmployee, previousEmployee;

    int employeeFileDescriptor = open(EMP_FILE, O_RDONLY);
    if (employeeFileDescriptor == -1 && errno == ENOENT)
    {
        // Employee file was never created
        newEmployee.id = 0;
    }
    else if (employeeFileDescriptor == -1)
    {
        perror("Error while opening employee file");
        return -1;
    }
    else
    {   //Reading last created employee's id so that new_empid= old_empid + 1

        int offset = lseek(employeeFileDescriptor, -sizeof(struct Employee), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last Employee record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Employee), getpid()};
        int lockingStatus = fcntl(employeeFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on Employee record!");
            return false;
        }

        readBytes = read(employeeFileDescriptor, &previousEmployee, sizeof(struct Employee));
        if (readBytes == -1)
        {
            perror("Error while reading Employee record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(employeeFileDescriptor, F_SETLK, &lock);

        close(employeeFileDescriptor);

        newEmployee.id = previousEmployee.id + 1;
    }

    sprintf(writeBuffer, "%s%s", ADMIN_ADD_EMPLOYEE, ADMIN_ADD_EMPLOYEE_NAME);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_EMPLOYEE_NAME message to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading employee name response from client!");
        return false;
    }

    strcpy(newEmployee.name, readBuffer);

    writeBytes = write(connFD, ADMIN_ADD_EMPLOYEE_GENDER, strlen(ADMIN_ADD_EMPLOYEE_GENDER));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_EMPLOYEE_GENDER message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading employee gender response from client!");
        return false;
    }

    if (readBuffer[0] == 'M' || readBuffer[0] == 'F' || readBuffer[0] == 'O')
        newEmployee.gender = readBuffer[0];
    else
    {
        writeBytes = write(connFD, ADMIN_ADD_EMPLOYEE_WRONG_GENDER, strlen(ADMIN_ADD_EMPLOYEE_WRONG_GENDER));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

        writeBytes = write(connFD, ADMIN_ADD_EMPLOYEE_ROLE, strlen(ADMIN_ADD_EMPLOYEE_ROLE));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_EMPLOYEE_GENDER message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading employee role response from client!");
        return false;
    }

    if (readBuffer[0] == 'M' || readBuffer[0] == 'E')
        newEmployee.role = readBuffer[0];
    else
    {
        writeBytes = write(connFD, ADMIN_ADD_EMPLOYEE_WRONG_ROLE, strlen(ADMIN_ADD_EMPLOYEE_WRONG_ROLE));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
        
        
    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, ADMIN_ADD_EMPLOYEE_AGE);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_EMPLOYEE_AGE message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading employee age response from client!");
        return false;
    }

    int employeeAge = atoi(readBuffer);
    if (employeeAge == 0)
    {
        // Either client has sent age as 0 (which is invalid) or has entered a non-numeric string
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    newEmployee.age = employeeAge;

    strcpy(newEmployee.login, newEmployee.name);
    strcat(newEmployee.login, "-");
    sprintf(writeBuffer, "%d", newEmployee.id);
    strcat(newEmployee.login, writeBuffer);

    char hashedPassword[1000];
    //strcpy(hashedPassword, crypt(AUTOGEN_PASSWORD, SALT_BAE));
    strcpy(hashedPassword, EMP_AUTOGEN_PASSWORD);
    char idStr[5]; // Enough space for an integer as a string
    snprintf(idStr, sizeof(idStr), "%d", newEmployee.id); // Convert id to string
    strcat(hashedPassword, idStr);
    strcpy(newEmployee.password, hashedPassword);

    employeeFileDescriptor = open(EMP_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    if (employeeFileDescriptor == -1)
    {
        perror("Error while creating / opening employee file!");
        return false;
    }
    writeBytes = write(employeeFileDescriptor, &newEmployee, sizeof(newEmployee));
    if (writeBytes == -1)
    {
        perror("Error while writing Employee record to file!");
        return false;
    }

    close(employeeFileDescriptor);

    memset(writeBuffer, 0, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%s\n%s%s", ADMIN_ADD_EMPLOYEE_AUTOGEN_LOGIN, newEmployee.login, ADMIN_ADD_EMPLOYEE_AUTOGEN_PASSWORD, newEmployee.password);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending employee loginID and password to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return newEmployee.id;
}

/*
bool add_account(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Account newAccount, prevAccount;

    int accountFileDescriptor = open(ACCOUNT_FILE, O_RDONLY);
    if (accountFileDescriptor == -1 && errno == ENOENT)
    {
        // Account file was never created
        newAccount.accountNumber = 0;
    }
    else if (accountFileDescriptor == -1)
    {
        perror("Error while opening account file");
        return false;
    }
    else
    {
        int offset = lseek(accountFileDescriptor, -sizeof(struct Account), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last Account record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Account), getpid()};
        int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on Account record!");
            return false;
        }

        readBytes = read(accountFileDescriptor, &prevAccount, sizeof(struct Account));
        if (readBytes == -1)
        {
            perror("Error while reading Account record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(accountFileDescriptor, F_SETLK, &lock);

        close(accountFileDescriptor);

        newAccount.accountNumber = prevAccount.accountNumber + 1;
    }
    writeBytes = write(connFD, ADMIN_ADD_ACCOUNT_TYPE, strlen(ADMIN_ADD_ACCOUNT_TYPE));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_ACCOUNT_TYPE message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading account type response from client!");
        return false;
    }

    newAccount.isRegularAccount = atoi(readBuffer) == 1 ? true : false;

    newAccount.owners[0] = add_customer(connFD, true, newAccount.accountNumber);

    if (newAccount.isRegularAccount)
        newAccount.owners[1] = -1;
    else
        newAccount.owners[1] = add_customer(connFD, false, newAccount.accountNumber);

    newAccount.active = true;
    newAccount.balance = 0;

    memset(newAccount.transactions, -1, MAX_TRANSACTIONS * sizeof(int));

    accountFileDescriptor = open(ACCOUNT_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    if (accountFileDescriptor == -1)
    {
        perror("Error while creating / opening account file!");
        return false;
    }

    writeBytes = write(accountFileDescriptor, &newAccount, sizeof(struct Account));
    if (writeBytes == -1)
    {
        perror("Error while writing Account record to file!");
        return false;
    }

    close(accountFileDescriptor);

    memset(writeBuffer, 0, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%d", ADMIN_ADD_ACCOUNT_NUMBER, newAccount.accountNumber);
    strcat(writeBuffer, "\nRedirecting you to the main menu ...^");
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    readBytes = read(connFD, readBuffer, sizeof(read)); // Dummy read
    return true;
}

int add_customer(int connFD, bool isPrimary, int newAccountNumber)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Customer newCustomer, previousCustomer;

    int customerFileDescriptor = open(CUSTOMER_FILE, O_RDONLY);
    if (customerFileDescriptor == -1 && errno == ENOENT)
    {
        // Customer file was never created
        newCustomer.id = 0;
    }
    else if (customerFileDescriptor == -1)
    {
        perror("Error while opening customer file");
        return -1;
    }
    else
    {
        int offset = lseek(customerFileDescriptor, -sizeof(struct Customer), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last Customer record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Customer), getpid()};
        int lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on Customer record!");
            return false;
        }

        readBytes = read(customerFileDescriptor, &previousCustomer, sizeof(struct Customer));
        if (readBytes == -1)
        {
            perror("Error while reading Customer record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(customerFileDescriptor, F_SETLK, &lock);

        close(customerFileDescriptor);

        newCustomer.id = previousCustomer.id + 1;
    }

    if (isPrimary)
        sprintf(writeBuffer, "%s%s", ADMIN_ADD_CUSTOMER_PRIMARY, ADMIN_ADD_CUSTOMER_NAME);
    else
        sprintf(writeBuffer, "%s%s", ADMIN_ADD_CUSTOMER_SECONDARY, ADMIN_ADD_CUSTOMER_NAME);

    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_CUSTOMER_NAME message to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading customer name response from client!");
        ;
        return false;
    }

    strcpy(newCustomer.name, readBuffer);

    writeBytes = write(connFD, ADMIN_ADD_CUSTOMER_GENDER, strlen(ADMIN_ADD_CUSTOMER_GENDER));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_CUSTOMER_GENDER message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading customer gender response from client!");
        return false;
    }

    if (readBuffer[0] == 'M' || readBuffer[0] == 'F' || readBuffer[0] == 'O')
        newCustomer.gender = readBuffer[0];
    else
    {
        writeBytes = write(connFD, ADMIN_ADD_CUSTOMER_WRONG_GENDER, strlen(ADMIN_ADD_CUSTOMER_WRONG_GENDER));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, ADMIN_ADD_CUSTOMER_AGE);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_CUSTOMER_AGE message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading customer age response from client!");
        return false;
    }

    int customerAge = atoi(readBuffer);
    if (customerAge == 0)
    {
        // Either client has sent age as 0 (which is invalid) or has entered a non-numeric string
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    newCustomer.age = customerAge;

    newCustomer.account = newAccountNumber;

    strcpy(newCustomer.login, newCustomer.name);
    strcat(newCustomer.login, "-");
    sprintf(writeBuffer, "%d", newCustomer.id);
    strcat(newCustomer.login, writeBuffer);

    char hashedPassword[1000];
    //strcpy(hashedPassword, crypt(AUTOGEN_PASSWORD, SALT_BAE));
    strcpy(hashedPassword, AUTOGEN_PASSWORD);
    strcpy(newCustomer.password, hashedPassword);

    customerFileDescriptor = open(CUSTOMER_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    if (customerFileDescriptor == -1)
    {
        perror("Error while creating / opening customer file!");
        return false;
    }
    writeBytes = write(customerFileDescriptor, &newCustomer, sizeof(newCustomer));
    if (writeBytes == -1)
    {
        perror("Error while writing Customer record to file!");
        return false;
    }

    close(customerFileDescriptor);

    memset(writeBuffer, 0, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%s-%d\n%s%s", ADMIN_ADD_CUSTOMER_AUTOGEN_LOGIN, newCustomer.name, newCustomer.id, ADMIN_ADD_CUSTOMER_AUTOGEN_PASSWORD, AUTOGEN_PASSWORD);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending customer loginID and password to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return newCustomer.id;
}

bool delete_account(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Account account;

    writeBytes = write(connFD, ADMIN_DEL_ACCOUNT_NO, strlen(ADMIN_DEL_ACCOUNT_NO));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_DEL_ACCOUNT_NO to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading account number response from the client!");
        return false;
    }

    int accountNumber = atoi(readBuffer);

    int accountFileDescriptor = open(ACCOUNT_FILE, O_RDONLY);
    if (accountFileDescriptor == -1)
    {
        // Account record doesn't exist
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIST);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ACCOUNT_ID_DOESNT_EXIST message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }


    int offset = lseek(accountFileDescriptor, accountNumber * sizeof(struct Account), SEEK_SET);
    if (errno == EINVAL)
    {
        // Customer record doesn't exist
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIST);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ACCOUNT_ID_DOESNT_EXIST message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required account record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Account), getpid()};
    int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error obtaining read lock on Account record!");
        return false;
    }

    readBytes = read(accountFileDescriptor, &account, sizeof(struct Account));
    if (readBytes == -1)
    {
        perror("Error while reading Account record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(accountFileDescriptor, F_SETLK, &lock);

    close(accountFileDescriptor);

    memset(writeBuffer, 0, sizeof(writeBuffer));
    if (account.balance == 0)
    {
        // No money, hence can close account
        account.active = false;
        accountFileDescriptor = open(ACCOUNT_FILE, O_WRONLY);
        if (accountFileDescriptor == -1)
        {
            perror("Error opening Account file in write mode!");
            return false;
        }

        offset = lseek(accountFileDescriptor, accountNumber * sizeof(struct Account), SEEK_SET);
        if (offset == -1)
        {
            perror("Error seeking to the Account!");
            return false;
        }

        lock.l_type = F_WRLCK;
        lock.l_start = offset;

        int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining write lock on the Account file!");
            return false;
        }

        writeBytes = write(accountFileDescriptor, &account, sizeof(struct Account));
        if (writeBytes == -1)
        {
            perror("Error deleting account record!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(accountFileDescriptor, F_SETLK, &lock);

        strcpy(writeBuffer, ADMIN_DEL_ACCOUNT_SUCCESS);
    }
    else
        // Account has some money ask customer to withdraw it
        strcpy(writeBuffer, ADMIN_DEL_ACCOUNT_FAILURE);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error while writing final DEL message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
}
*/

bool modify_emp_info(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Employee employee;

    int employeeID;

    off_t offset;
    int lockingStatus;

    writeBytes = write(connFD, ADMIN_MOD_EMPLOYEE_ID, strlen(ADMIN_MOD_EMPLOYEE_ID));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_EMPLOYEE_ID message to client!");
        return false;
    }
    memset(readBuffer, 0, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading employee ID from client!");
        return false;
    }

    employeeID = atoi(readBuffer);

    int employeeFileDescriptor = open(EMP_FILE, O_RDONLY);
    if (employeeFileDescriptor == -1)
    {
        // Employee File doesn't exist
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, EMP_ID_DOESNT_EXIST);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing EMPLOYEE_ID_DOESNT_EXIST message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    
    offset = lseek(employeeFileDescriptor, employeeID * sizeof(struct Employee), SEEK_SET);
    if (errno == EINVAL)
    {
        // Employee record doesn't exist
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, EMP_ID_DOESNT_EXIST);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing EMPLOYEE_ID_DOESNT_EXIST message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required employee record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Employee), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(employeeFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on employee record!");
        return false;
    }

    readBytes = read(employeeFileDescriptor, &employee, sizeof(struct Employee));
    if (readBytes == -1)
    {
        perror("Error while reading employee record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(employeeFileDescriptor, F_SETLK, &lock);

    close(employeeFileDescriptor);

    writeBytes = write(connFD, ADMIN_MOD_EMPLOYEE_MENU, strlen(ADMIN_MOD_EMPLOYEE_MENU));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_EMPLOYEE_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting employee modification menu choice from client!");
        return false;
    }

    int choice = atoi(readBuffer);
    if (choice == 0)
    { // A non-numeric string was passed to atoi
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    memset(readBuffer, 0, sizeof(readBuffer));
    switch (choice)
    {
    case 1:
        writeBytes = write(connFD, ADMIN_MOD_EMPLOYEE_NEW_NAME, strlen(ADMIN_MOD_EMPLOYEE_NEW_NAME));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_EMPLOYEE_NEW_NAME message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for employee's new name from client!");
            return false;
        }
        strcpy(employee.name, readBuffer);
        break;
    case 2:
        writeBytes = write(connFD, ADMIN_MOD_EMPLOYEE_NEW_AGE, strlen(ADMIN_MOD_EMPLOYEE_NEW_AGE));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_EMPLOYEE_NEW_AGE message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for employee's new age from client!");
            return false;
        }
        int updatedAge = atoi(readBuffer);
        if (updatedAge == 0)
        {
            // Either client has sent age as 0 (which is invalid) or has entered a non-numeric string
            memset(writeBuffer, 0, sizeof(writeBuffer));
            strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
                return false;
            }
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            return false;
        }
        employee.age = updatedAge;
        break;
    case 3:
        writeBytes = write(connFD, ADMIN_MOD_EMPLOYEE_NEW_GENDER, strlen(ADMIN_MOD_EMPLOYEE_NEW_GENDER));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_EMPLOYEE_NEW_GENDER message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for employee's new gender from client!");
            return false;
        }
        employee.gender = readBuffer[0];
        break;
    default:
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, INVALID_MENU_CHOICE);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing INVALID_MENU_CHOICE message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    employeeFileDescriptor = open(EMP_FILE, O_WRONLY);
    if (employeeFileDescriptor == -1)
    {
        perror("Error while opening employee file");
        return false;
    }
    offset = lseek(employeeFileDescriptor, employeeID * sizeof(struct Employee), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required employee record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(employeeFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on employee record!");
        return false;
    }

    writeBytes = write(employeeFileDescriptor, &employee, sizeof(struct Employee));
    if (writeBytes == -1)
    {
        perror("Error while writing update employee info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(employeeFileDescriptor, F_SETLKW, &lock);

    close(employeeFileDescriptor);

    writeBytes = write(connFD, ADMIN_MOD_EMPLOYEE_SUCCESS, strlen(ADMIN_MOD_EMPLOYEE_SUCCESS));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_EMPLOYEE_SUCCESS message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
}

#endif