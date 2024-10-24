#ifndef CUSTOMER_FUNCTIONS
#define CUSTOMER_FUNCTIONS


#include <sys/ipc.h>
#include <sys/sem.h>
#include "common.h"

struct Customer loggedInCustomer;
int semIdentifier;

// Function Prototypes =================================

bool customer_operation_handler(int connFD);
bool deposit(int connFD);
bool withdraw(int connFD);
bool get_balance(int connFD);
bool get_account_details(int connFD, struct Account* account);
bool get_customer_details(int connFD, int customerID);
bool get_transaction_details(int connFD, int accountNumber);
bool get_transaction_details(int connFD, int accountNumber);
void write_transaction_to_array(int *transactionArray, int ID);
int write_transaction_to_file(int accountNumber, long int oldBalance, long int newBalance, bool operation);

// =====================================================

// Function Definition =================================

bool customer_operation_handler(int connFD)
{
    if (login_handler(1, connFD, &loggedInCustomer))
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000]; // A buffer used for reading & writing to the client

        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, CUSTOMER_LOGIN_SUCCESS);

        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, CUSTOMER_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing CUSTOMER_MENU to client!");
                return false;
            }
            memset(readBuffer, 0, sizeof(readBuffer));
            //bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for CUSTOMER_MENU");
                return false;
            }
            
            // printf("READ BUFFER : %s\n", readBuffer);
            int choice = atoi(readBuffer);
            // printf("CHOICE : %d\n", choice);
            switch (choice)
            {
            case 1:
                get_customer_details(connFD, loggedInCustomer.id);
                break;
            case 2:
                get_balance(connFD);
                break;
            case 3:
                deposit(connFD);
                break;
            case 4:
                withdraw(connFD);
                break;
            case 5:
                //transfer(connFD);
                break;
            case 6:
                //apply_loan(connFD);
                break;
            case 7:
                change_password(1,connFD,&loggedInCustomer);
                break;
            case 8:
                //add_feedback(connFD);
                break;
            case 9:
                get_transaction_details(connFD,loggedInCustomer.account);
                break;
            
            default:
                writeBytes = write(connFD, CUSTOMER_LOGOUT, strlen(CUSTOMER_LOGOUT));
                return false;
            }
        }
    }
    else
    {
        // CUSTOMER LOGIN FAILED
      return false;
    }
   return true;
}

bool get_account_details(int connFD, struct Account* customerAccount)
{
    ssize_t readBytes, writeBytes;            // Number of bytes read from / written to the socket
    char readBuffer[1000], writeBuffer[1000]; // A buffer for reading from / writing to the socket
    char tempBuffer[1000];

    int accountNumber;
    struct Account account;
    int accountFileDescriptor;

    if (customerAccount == NULL)
    {

        writeBytes = write(connFD, GET_ACCOUNT_NUMBER, strlen(GET_ACCOUNT_NUMBER));
        if (writeBytes == -1)
        {
            perror("Error writing GET_ACCOUNT_NUMBER message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error reading account number response from client!");
            return false;
        }

        accountNumber = atoi(readBuffer);
    }
    else
        accountNumber = customerAccount->accountNumber;

    accountFileDescriptor = open(ACCOUNT_FILE, O_RDONLY);
    if (accountFileDescriptor == -1)
    {
        // Account record doesn't exist
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIST);
        strcat(writeBuffer, "^");
        perror("Error opening account file in get_account_details!");
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
    if (offset == -1 && errno == EINVAL)
    {
        // Account record doesn't exist
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIST);
        strcat(writeBuffer, "^");
        perror("Error seeking to account record in get_account_details!");
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
        perror("Error obtaining read lock on account record!");
        return false;
    }

    readBytes = read(accountFileDescriptor, &account, sizeof(struct Account));
    if (readBytes == -1)
    {
        perror("Error reading account record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(accountFileDescriptor, F_SETLK, &lock);

    if (customerAccount != NULL)
    {
        *customerAccount = account;
        return true;
    }

    memset(writeBuffer, 0, sizeof(writeBuffer));
    sprintf(writeBuffer, "Account Details - \n\tAccount Number : %d\n\tAccount Status : %s", account.accountNumber, (account.active) ? "Active" : "Deactive");
    if (account.active)
    {
        sprintf(tempBuffer, "\n\tAccount Balance:₹ %ld", account.balance);
        strcat(writeBuffer, tempBuffer);
    }

    strcat(writeBuffer, "\n^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
}
/*
off_t find_customer_by_id(int customerFileDescriptor, int customerID) {
    struct Customer tempCustomer;
    off_t offset = 0;

    // Read through the file, record by record, until we find the customer
    while (read(customerFileDescriptor, &tempCustomer, sizeof(struct Customer)) > 0) {
        if (tempCustomer.id == customerID) {
            // Found the customer, return the current offset
            return offset;
        }
        offset += sizeof(struct Customer);
    }

    // If no customer is found, return -1
    return -1;
}
*/
bool get_customer_details(int connFD, int customerID)
{
    ssize_t readBytes, writeBytes;             // Number of bytes read from / written to the socket
    char readBuffer[1000], writeBuffer[10000]; // A buffer for reading from / writing to the socket
    char tempBuffer[1000];

    struct Customer customer;
    int customerFileDescriptor;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Account), getpid()};

    if (customerID == -1)
    {
        writeBytes = write(connFD, GET_CUSTOMER_ID, strlen(GET_CUSTOMER_ID));
        if (writeBytes == -1)
        {
            perror("Error while writing GET_CUSTOMER_ID message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error getting customer ID from client!");
            ;
            return false;
        }

        customerID = atoi(readBuffer);
    }

    customerFileDescriptor = open(CUSTOMER_FILE, O_RDONLY);
    if (customerFileDescriptor == -1)
    {
        // Customer File doesn't exist
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, CUSTOMER_ID_DOESNT_EXIST);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing CUSTOMER_ID_DOESNT_EXIST message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    //int offset = find_customer_by_id(customerFileDescriptor, customerID);
    
    int offset = lseek(customerFileDescriptor, customerID * sizeof(struct Customer), SEEK_SET);
    if (errno == EINVAL)
    {
        // Customer record doesn't exist
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, CUSTOMER_ID_DOESNT_EXIST);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing CUSTOMER_ID_DOESNT_EXIST message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required customer record!");
        return false;
    }
    lock.l_start = offset;

    int lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the Customer file!");
        return false;
    }

    readBytes = read(customerFileDescriptor, &customer, sizeof(struct Customer));
    if (readBytes == -1)
    {
        perror("Error reading customer record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(customerFileDescriptor, F_SETLK, &lock);

    memset(writeBuffer, 0, sizeof(writeBuffer));
    sprintf(writeBuffer, "Customer Details - \n\tID : %d\n\tName : %s\n\tGender : %c\n\tAge: %d\n\tAccount Number : %d\n\tLoginID : %s", customer.id, customer.name, customer.gender, customer.age, customer.account, customer.login);

    strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing customer info to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
}

bool deposit(int connFD)
{
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;

    long int depositAmount = 0;

    // Lock the critical section
    struct sembuf semOp;
    lock_critical_section(&semOp);

    if (get_account_details(connFD, &account))
    {
        
        if (account.active)
        {

            writeBytes = write(connFD, DEPOSIT_AMOUNT, strlen(DEPOSIT_AMOUNT));
            if (writeBytes == -1)
            {
                perror("Error writing DEPOSIT_AMOUNT to client!");
                unlock_critical_section(&semOp);
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error reading deposit money from client!");
                unlock_critical_section(&semOp);
                return false;
            }

            depositAmount = atol(readBuffer);
            if (depositAmount != 0)
            {

                int newTransactionID = write_transaction_to_file(account.accountNumber, account.balance, account.balance + depositAmount, 1);
                write_transaction_to_array(account.transactions, newTransactionID);

                account.balance += depositAmount;

                int accountFileDescriptor = open(ACCOUNT_FILE, O_WRONLY);
                off_t offset = lseek(accountFileDescriptor, account.accountNumber * sizeof(struct Account), SEEK_SET);

                struct flock lock = {F_WRLCK, SEEK_SET, offset, sizeof(struct Account), getpid()};
                int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
                if (lockingStatus == -1)
                {
                    perror("Error obtaining write lock on account file!");
                    unlock_critical_section(&semOp);
                    return false;
                }

                writeBytes = write(accountFileDescriptor, &account, sizeof(struct Account));
                if (writeBytes == -1)
                {
                    perror("Error storing updated deposit money in account record!");
                    unlock_critical_section(&semOp);
                    return false;
                }

                lock.l_type = F_UNLCK;
                fcntl(accountFileDescriptor, F_SETLK, &lock);

                write(connFD, DEPOSIT_AMOUNT_SUCCESS, strlen(DEPOSIT_AMOUNT_SUCCESS));
                read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

                get_balance(connFD);

                unlock_critical_section(&semOp);

                return true;
            }
            else
                writeBytes = write(connFD, DEPOSIT_AMOUNT_INVALID, strlen(DEPOSIT_AMOUNT_INVALID));
        }
        else
            write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

        unlock_critical_section(&semOp);
    }
    else
    {
        // FAIL
        unlock_critical_section(&semOp);
        return false;
    }
}

bool withdraw(int connFD)
{
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;

    long int withdrawAmount = 0;

    // Lock the critical section
    struct sembuf semOp;
    lock_critical_section(&semOp);

    if (get_account_details(connFD, &account))
    {
        if (account.active)
        {

            writeBytes = write(connFD, WITHDRAW_AMOUNT, strlen(WITHDRAW_AMOUNT));
            if (writeBytes == -1)
            {
                perror("Error writing WITHDRAW_AMOUNT message to client!");
                unlock_critical_section(&semOp);
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error reading withdraw amount from client!");
                unlock_critical_section(&semOp);
                return false;
            }

            withdrawAmount = atol(readBuffer);

            if (withdrawAmount != 0 && account.balance - withdrawAmount >= 0)
            {

                int newTransactionID = write_transaction_to_file(account.accountNumber, account.balance, account.balance - withdrawAmount, 0);
                write_transaction_to_array(account.transactions, newTransactionID);

                account.balance -= withdrawAmount;

                int accountFileDescriptor = open(ACCOUNT_FILE, O_WRONLY);
                off_t offset = lseek(accountFileDescriptor, account.accountNumber * sizeof(struct Account), SEEK_SET);

                struct flock lock = {F_WRLCK, SEEK_SET, offset, sizeof(struct Account), getpid()};
                int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
                if (lockingStatus == -1)
                {
                    perror("Error obtaining write lock on account record!");
                    unlock_critical_section(&semOp);
                    return false;
                }

                writeBytes = write(accountFileDescriptor, &account, sizeof(struct Account));
                if (writeBytes == -1)
                {
                    perror("Error writing updated balance into account file!");
                    unlock_critical_section(&semOp);
                    return false;
                }

                lock.l_type = F_UNLCK;
                fcntl(accountFileDescriptor, F_SETLK, &lock);

                write(connFD, WITHDRAW_AMOUNT_SUCCESS, strlen(WITHDRAW_AMOUNT_SUCCESS));
                read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

                get_balance(connFD);

                unlock_critical_section(&semOp);

                return true;
            }
            else
                writeBytes = write(connFD, WITHDRAW_AMOUNT_INVALID, strlen(WITHDRAW_AMOUNT_INVALID));
        }
        else
            write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

        unlock_critical_section(&semOp);
    }
    else
    {
        // FAILURE while getting account information
        unlock_critical_section(&semOp);
        return false;
    }
}

bool get_balance(int connFD)
{
    char buffer[1000];
    struct Account account;
    account.accountNumber = loggedInCustomer.account;
    if (get_account_details(connFD, &account))
    {
        bzero(buffer, sizeof(buffer));
        if (account.active)
        {
            sprintf(buffer, "You have ₹ %ld imaginary money in our bank!^", account.balance);
            write(connFD, buffer, strlen(buffer));
        }
        else
            write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        read(connFD, buffer, sizeof(buffer)); // Dummy read
    }
    else
    {
        // ERROR while getting balance
        return false;
    }
}
/*
bool change_password(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000], hashedPassword[1000];

    char newPassword[1000];

    // Lock the critical section
    struct sembuf semOp = {0, -1, SEM_UNDO};
    int semopStatus = semop(semIdentifier, &semOp, 1);
    if (semopStatus == -1)
    {
        perror("Error while locking critical section");
        return false;
    }

    writeBytes = write(connFD, PASSWORD_CHANGE_OLD_PASS, strlen(PASSWORD_CHANGE_OLD_PASS));
    if (writeBytes == -1)
    {
        perror("Error writing PASSWORD_CHANGE_OLD_PASS message to client!");
        unlock_critical_section(&semOp);
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading old password response from client");
        unlock_critical_section(&semOp);
        return false;
    }

    if (strcmp(crypt(readBuffer, SALT_BAE), loggedInCustomer.password) == 0)
    if (strcmp(readBuffer, loggedInCustomer.password) == 0)
    {
        // Password matches with old password
        writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS, strlen(PASSWORD_CHANGE_NEW_PASS));
        if (writeBytes == -1)
        {
            perror("Error writing PASSWORD_CHANGE_NEW_PASS message to client!");
            unlock_critical_section(&semOp);
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error reading new password response from client");
            unlock_critical_section(&semOp);
            return false;
        }

        //strcpy(newPassword, crypt(readBuffer, SALT_BAE));
        strcpy(newPassword, readBuffer);

        writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS_RE, strlen(PASSWORD_CHANGE_NEW_PASS_RE));
        if (writeBytes == -1)
        {
            perror("Error writing PASSWORD_CHANGE_NEW_PASS_RE message to client!");
            unlock_critical_section(&semOp);
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error reading new password reenter response from client");
            unlock_critical_section(&semOp);
            return false;
        }

        //if (strcmp(crypt(readBuffer, SALT_BAE), newPassword) == 0)
        if (strcmp(readBuffer, newPassword) == 0)
        {
            // New & reentered passwords match

            strcpy(loggedInCustomer.password, newPassword);

            int customerFileDescriptor = open(CUSTOMER_FILE, O_WRONLY);
            if (customerFileDescriptor == -1)
            {
                perror("Error opening customer file!");
                unlock_critical_section(&semOp);
                return false;
            }

            off_t offset = lseek(customerFileDescriptor, loggedInCustomer.id * sizeof(struct Customer), SEEK_SET);
            if (offset == -1)
            {
                perror("Error seeking to the customer record!");
                unlock_critical_section(&semOp);
                return false;
            }

            struct flock lock = {F_WRLCK, SEEK_SET, offset, sizeof(struct Customer), getpid()};
            int lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error obtaining write lock on customer record!");
                unlock_critical_section(&semOp);
                return false;
            }

            writeBytes = write(customerFileDescriptor, &loggedInCustomer, sizeof(struct Customer));
            if (writeBytes == -1)
            {
                perror("Error storing updated customer password into customer record!");
                unlock_critical_section(&semOp);
                return false;
            }

            lock.l_type = F_UNLCK;
            lockingStatus = fcntl(customerFileDescriptor, F_SETLK, &lock);

            close(customerFileDescriptor);

            writeBytes = write(connFD, PASSWORD_CHANGE_SUCCESS, strlen(PASSWORD_CHANGE_SUCCESS));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

            unlock_critical_section(&semOp);

            return true;
        }
        else
        {
            // New & reentered passwords don't match
            writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS_INVALID, strlen(PASSWORD_CHANGE_NEW_PASS_INVALID));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        }
    }
    else
    {
        // Password doesn't match with old password
        writeBytes = write(connFD, PASSWORD_CHANGE_OLD_PASS_INVALID, strlen(PASSWORD_CHANGE_OLD_PASS_INVALID));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
    }

    unlock_critical_section(&semOp);

    return false;
}
*/
bool lock_critical_section(struct sembuf *semOp)
{
    semOp->sem_flg = SEM_UNDO;
    semOp->sem_op = -1;
    semOp->sem_num = 0;
    int semopStatus = semop(semIdentifier, semOp, 1);
    if (semopStatus == -1)
    {
        perror("Error while locking critical section");
        return false;
    }
    return true;
}

bool unlock_critical_section(struct sembuf *semOp)
{
    semOp->sem_op = 1;
    int semopStatus = semop(semIdentifier, semOp, 1);
    if (semopStatus == -1)
    {
        perror("Error while operating on semaphore!");
        _exit(1);
    }
    return true;
}

void write_transaction_to_array(int *transactionArray, int ID)
{
    // Check if there's any free space in the array to write the new transaction ID
    int iter = 0;
    while (transactionArray[iter] != -1)
        iter++;

    if (iter >= MAX_TRANSACTIONS)
    {
        // No space
        for (iter = 1; iter < MAX_TRANSACTIONS; iter++)
            // Shift elements one step back discarding the oldest transaction
            transactionArray[iter - 1] = transactionArray[iter];
        transactionArray[iter - 1] = ID;
    }
    else
    {
        // Space available
        transactionArray[iter] = ID;
    }
}

int write_transaction_to_file(int accountNumber, long int oldBalance, long int newBalance, bool operation)
{
    struct Transaction newTransaction;
    newTransaction.accountNumber = accountNumber;
    newTransaction.oldBalance = oldBalance;
    newTransaction.newBalance = newBalance;
    newTransaction.operation = operation;
    newTransaction.transactionTime = time(NULL);

    ssize_t readBytes, writeBytes;

    int transactionFileDescriptor = open(TRANSACTION_FILE, O_CREAT | O_APPEND | O_RDWR, S_IRWXU);

    // Get most recent transaction number
    off_t offset = lseek(transactionFileDescriptor, -sizeof(struct Transaction), SEEK_END);
    if (offset >= 0)
    {
        // There exists at least one transaction record
        struct Transaction prevTransaction;
        readBytes = read(transactionFileDescriptor, &prevTransaction, sizeof(struct Transaction));

        newTransaction.transactionID = prevTransaction.transactionID + 1;
    }
    else
        // No transaction records exist
        newTransaction.transactionID = 0;

    writeBytes = write(transactionFileDescriptor, &newTransaction, sizeof(struct Transaction));

    return newTransaction.transactionID;
}

bool get_transaction_details(int connFD, int accountNumber)
{

    ssize_t readBytes, writeBytes;                               // Number of bytes read from / written to the socket
    char readBuffer[1000], writeBuffer[10000], tempBuffer[1000]; // A buffer for reading from / writing to the socket

    struct Account account;

    if (accountNumber == -1)
    {
        // Get the accountNumber
        writeBytes = write(connFD, GET_ACCOUNT_NUMBER, strlen(GET_ACCOUNT_NUMBER));
        if (writeBytes == -1)
        {
            perror("Error writing GET_ACCOUNT_NUMBER message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error reading account number response from client!");
            return false;
        }

        account.accountNumber = atoi(readBuffer);
    }
    else
        account.accountNumber = accountNumber;

    if (get_account_details(connFD, &account))
    {
        int iter;

        struct Transaction transaction;
        struct tm transactionTime;

        bzero(writeBuffer, sizeof(readBuffer));

        int transactionFileDescriptor = open(TRANSACTION_FILE, O_RDONLY);
        if (transactionFileDescriptor == -1)
        {
            perror("Error while opening transaction file!");
            write(connFD, TRANSACTIONS_NOT_FOUND, strlen(TRANSACTIONS_NOT_FOUND));
            read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            return false;
        }

        for (iter = 0; iter < MAX_TRANSACTIONS && account.transactions[iter] != -1; iter++)
        {

            int offset = lseek(transactionFileDescriptor, account.transactions[iter] * sizeof(struct Transaction), SEEK_SET);
            if (offset == -1)
            {
                perror("Error while seeking to required transaction record!");
                return false;
            }

            struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Transaction), getpid()};

            int lockingStatus = fcntl(transactionFileDescriptor, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error obtaining read lock on transaction record!");
                return false;
            }

            readBytes = read(transactionFileDescriptor, &transaction, sizeof(struct Transaction));
            if (readBytes == -1)
            {
                perror("Error reading transaction record from file!");
                return false;
            }

            lock.l_type = F_UNLCK;
            fcntl(transactionFileDescriptor, F_SETLK, &lock);

            transactionTime = *localtime(&(transaction.transactionTime));

            bzero(tempBuffer, sizeof(tempBuffer));
            sprintf(tempBuffer, "Details of transaction %d - \n\t Date : %d:%d %d/%d/%d \n\t Operation : %s \n\t Balance - \n\t\t Before : %ld \n\t\t After : %ld \n\t\t Difference : %ld\n", (iter + 1), transactionTime.tm_hour, transactionTime.tm_min, transactionTime.tm_mday, (transactionTime.tm_mon + 1), (transactionTime.tm_year + 1900), (transaction.operation ? "Deposit" : "Withdraw"), transaction.oldBalance, transaction.newBalance, (transaction.newBalance - transaction.oldBalance));

            if (strlen(writeBuffer) == 0)
                strcpy(writeBuffer, tempBuffer);
            else
                strcat(writeBuffer, tempBuffer);
        }

        close(transactionFileDescriptor);

        if (strlen(writeBuffer) == 0)
        {
            write(connFD, TRANSACTIONS_NOT_FOUND, strlen(TRANSACTIONS_NOT_FOUND));
            read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            return false;
        }
        else
        {
            strcat(writeBuffer, "^");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        }
    }
}

// =====================================================

#endif