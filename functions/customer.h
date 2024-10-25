#ifndef CUSTOMER_FUNCTIONS
#define CUSTOMER_FUNCTIONS

#include <sys/ipc.h>
#include <sys/sem.h>
#include "common.h"
#include <fcntl.h>  // For open(), fcntl()
#include <unistd.h> // For read(), write(), close()
#include <stdio.h>  // For perror()
#include <string.h> // For memset()
#include <stdlib.h> // For atol()
#include <time.h>   // For time()

struct Customer *loggedInCustomer = NULL;
int semIdentifier;

// Function Prototypes =================================

bool customer_operation_handler(int connFD);
bool deposit(int connFD);
bool withdraw(int connFD);
bool get_balance(int connFD);
bool applyloan(int connFD);
// bool get_account_details(int connFD, struct Account *account);
bool get_customer_details(int connFD);
bool get_transaction_details(int connFD);
bool transfer(int connFD);
bool add_transaction(int accountNumber, bool operation, long int oldBalance, long int newBalance);
bool loanstatus(int connFD);
// =====================================================

// Function Definition =================================

// bool transfer(int sock, int from_acc, int to_acc){

// }

bool customer_operation_handler(int connFD)
{
    if (login_handler(1, connFD, (void **)&loggedInCustomer))
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
            // memset(readBuffer, 0, sizeof(readBuffer));
            memset(writeBuffer, 0, sizeof(readBuffer));
            // bzero(readBuffer, sizeof(readBuffer));
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
                get_customer_details(connFD);
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
                transfer(connFD);
                break;
            case 6:
                applyloan(connFD);
                break;
            case 7:
                change_password(1, connFD, (void **)&loggedInCustomer);
                break;
            case 8:
                loanstatus(connFD);
                break;
            case 9:
                get_transaction_details(connFD);
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

bool get_customer_details(int connFD)
{
    ssize_t readBytes, writeBytes;             // Number of bytes read from / written to the socket
    char readBuffer[1000], writeBuffer[10000]; // A buffer for reading from / writing to the socket
    char tempBuffer[1000];

    // struct Customer customer;
    struct Customer customer;

    memset(writeBuffer, 0, sizeof(writeBuffer));
    sprintf(writeBuffer, "Customer Details - \nName : %s\nGender : %c\nAge: %d\nAccount Number : %d\nLoginID : %s", loggedInCustomer->name, loggedInCustomer->gender, loggedInCustomer->age, loggedInCustomer->account, loggedInCustomer->login);

    strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing customer info to client!");
        // close(customerFileDescriptor);
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
    // close(customerFileDescriptor);
    return true;
}

bool transfer(int connFD)
{
    char readBuffer[1000];
    ssize_t readBytes, writeBytes;
    int customerFileDescriptor = open(CUSTOMER_FILE, O_RDWR);
    if (customerFileDescriptor < 0)
    {
        perror("Error opening customer file");
        return false;
    }

    struct Customer account, receiverAccount;
    long int transferAmount = 0;
    int receiverAccountNumber;

    // Retrieve sender's account record
    off_t offsetSender = lseek(customerFileDescriptor, loggedInCustomer->account * sizeof(struct Customer), SEEK_SET);
    if (offsetSender < 0)
    {
        perror("Error seeking sender's account record");
        close(customerFileDescriptor);
        return false;
    }
    readBytes = read(customerFileDescriptor, &account, sizeof(struct Customer));
    if (readBytes != sizeof(struct Customer) || !account.active)
    {
        write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        close(customerFileDescriptor);
        return false;
    }

    // Ask for the receiver's account number
    writeBytes = write(connFD, TRANSFER_ACCOUNT, strlen(TRANSFER_ACCOUNT));
    if (writeBytes == -1)
    {
        perror("Error writing enter account message to client!");
        close(customerFileDescriptor);
        return false;
    }

    memset(readBuffer, 0, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading receiver's account number from client!");
        close(customerFileDescriptor);
        return false;
    }
    receiverAccountNumber = atoi(readBuffer);

    // Retrieve receiver's account record
    off_t offsetReceiver = lseek(customerFileDescriptor, receiverAccountNumber * sizeof(struct Customer), SEEK_SET);
    if (offsetReceiver < 0)
    {
        perror("Error seeking receiver's account record");
        close(customerFileDescriptor);
        return false;
    }
    readBytes = read(customerFileDescriptor, &receiverAccount, sizeof(struct Customer));
    if (readBytes != sizeof(struct Customer) || !receiverAccount.active)
    {
        write(connFD, "Receiver account is deactivated or not found!^", strlen("Receiver account is deactivated or not found!^"));
        close(customerFileDescriptor);
        return false;
    }

    // Ask for the transfer amount
    writeBytes = write(connFD, TRANSFER_AMOUNT, strlen(TRANSFER_AMOUNT));
    if (writeBytes == -1)
    {
        perror("Error writing enter amount message to client!");
        close(customerFileDescriptor);
        return false;
    }

    memset(readBuffer, 0, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading amount from client!");
        close(customerFileDescriptor);
        return false;
    }
    transferAmount = atol(readBuffer);

    // Check if transfer is valid
    if (transferAmount <= 0 || account.balance < transferAmount)
    {
        write(connFD, DEPOSIT_AMOUNT_INVALID, strlen(DEPOSIT_AMOUNT_INVALID));
        close(customerFileDescriptor);
        return false;
    }

    // Lock both records for writing
    struct flock lockSender = {F_WRLCK, SEEK_SET, offsetSender, sizeof(struct Customer), getpid()};
    struct flock lockReceiver = {F_WRLCK, SEEK_SET, offsetReceiver, sizeof(struct Customer), getpid()};
    if (fcntl(customerFileDescriptor, F_SETLKW, &lockSender) == -1 || fcntl(customerFileDescriptor, F_SETLKW, &lockReceiver) == -1)
    {
        perror("Error obtaining write lock on account records!");
        close(customerFileDescriptor);
        return false;
    }

    add_transaction(account.account, 0, account.balance, account.balance - transferAmount);
    add_transaction(receiverAccount.account, 1, receiverAccount.balance, receiverAccount.balance + transferAmount);
    // Update balances
    long int oldSenderBalance = account.balance;
    long int oldReceiverBalance = receiverAccount.balance;
    account.balance -= transferAmount;
    receiverAccount.balance += transferAmount;


    // Write updated sender record back to the file
    lseek(customerFileDescriptor, offsetSender, SEEK_SET); // Reposition the file offset
    writeBytes = write(customerFileDescriptor, &account, sizeof(struct Customer));
    if (writeBytes != sizeof(struct Customer))
    {
        perror("Error writing updated sender record to file!");
        close(customerFileDescriptor);
        return false;
    }

    // Write updated receiver record back to the file
    lseek(customerFileDescriptor, offsetReceiver, SEEK_SET); // Reposition the file offset
    writeBytes = write(customerFileDescriptor, &receiverAccount, sizeof(struct Customer));
    if (writeBytes != sizeof(struct Customer))
    {
        perror("Error writing updated receiver record to file!");
        close(customerFileDescriptor);
        return false;
    }

    // Unlock both records
    lockSender.l_type = F_UNLCK;
    lockReceiver.l_type = F_UNLCK;
    fcntl(customerFileDescriptor, F_SETLK, &lockSender);
    fcntl(customerFileDescriptor, F_SETLK, &lockReceiver);

    // Inform client of successful transfer
    write(connFD, TRANSFER_AMOUNT_SUCCESS, strlen(TRANSFER_AMOUNT_SUCCESS));
    close(customerFileDescriptor);
    return true;
}

bool deposit(int connFD)
{
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    struct Customer account;
    int customerFileDescriptor = open(CUSTOMER_FILE, O_RDWR);
    if (customerFileDescriptor < 0)
    {
        perror("Error opening customer file!");
        return false;
    }

    // Calculate the offset for the customer account
    off_t offset = loggedInCustomer->account * sizeof(struct Customer);
    struct flock lock;

    // Lock the customer record
    lock.l_type = F_WRLCK; // Write lock
    lock.l_whence = SEEK_SET;
    lock.l_start = offset;                // Start of the record
    lock.l_len = sizeof(struct Customer); // Length of the record

    // Attempt to lock the record
    if (fcntl(customerFileDescriptor, F_SETLKW, &lock) == -1)
    {
        perror("Error obtaining write lock on account file!");
        close(customerFileDescriptor);
        return false;
    }

    // Read the existing account information
    readBytes = read(customerFileDescriptor, &account, sizeof(struct Customer));
    if (readBytes == -1)
    {
        perror("Error reading customer account!");
        fcntl(customerFileDescriptor, F_UNLCK, &lock); // Unlock before return
        close(customerFileDescriptor);
        return false;
    }

    // Verify if the account is active
    if (!account.active)
    {
        write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        fcntl(customerFileDescriptor, F_UNLCK, &lock); // Unlock before return
        close(customerFileDescriptor);
        return false;
    }

    // Prompt for deposit amount
    writeBytes = write(connFD, DEPOSIT_AMOUNT, strlen(DEPOSIT_AMOUNT));
    if (writeBytes == -1)
    {
        perror("Error writing DEPOSIT_AMOUNT to client!");
        fcntl(customerFileDescriptor, F_UNLCK, &lock); // Unlock before return
        close(customerFileDescriptor);
        return false;
    }

    // Read deposit amount from client
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading deposit money from client!");
        fcntl(customerFileDescriptor, F_UNLCK, &lock); // Unlock before return
        close(customerFileDescriptor);
        return false;
    }

    // Convert the amount and update the balance
    long int depositAmount = atol(readBuffer);
    if (depositAmount > 0)
    {
        add_transaction(account.account, 1, account.balance, account.balance + depositAmount);
        account.balance += depositAmount;            // Update balance
        loggedInCustomer->balance = account.balance; // Update logged-in customer's balance

        // Move file pointer back to the beginning of the record before writing
        lseek(customerFileDescriptor, offset, SEEK_SET);

        // Write the updated account back to the file
        writeBytes = write(customerFileDescriptor, &account, sizeof(struct Customer));
        if (writeBytes == -1)
        {
            perror("Error storing updated deposit money in account record!");
            fcntl(customerFileDescriptor, F_UNLCK, &lock); // Unlock before return
            close(customerFileDescriptor);
            return false;
        }

        // Unlock the record after writing
        lock.l_type = F_UNLCK; // Set lock type to unlock
        fcntl(customerFileDescriptor, F_SETLK, &lock);

        // Notify the client of success
        write(connFD, DEPOSIT_AMOUNT_SUCCESS, strlen(DEPOSIT_AMOUNT_SUCCESS)-1);
    }
    else
    {
        write(connFD, DEPOSIT_AMOUNT_INVALID, strlen(DEPOSIT_AMOUNT_INVALID));
    }

    // Cleanup
    close(customerFileDescriptor);
    return true;
}

bool withdraw(int connFD)
{
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    struct Customer account;
    int customerFileDescriptor = open(CUSTOMER_FILE, O_RDWR);
    if (customerFileDescriptor < 0)
    {
        perror("Error opening customer file!");
        return false;
    }

    // Calculate the offset for the customer account
    off_t offset = loggedInCustomer->account * sizeof(struct Customer);
    struct flock lock;

    // Lock the customer record
    lock.l_type = F_WRLCK; // Write lock
    lock.l_whence = SEEK_SET;
    lock.l_start = offset;                // Start of the record
    lock.l_len = sizeof(struct Customer); // Length of the record

    // Attempt to lock the record
    if (fcntl(customerFileDescriptor, F_SETLKW, &lock) == -1)
    {
        perror("Error obtaining write lock on account file!");
        close(customerFileDescriptor);
        return false;
    }

    // Read the existing account information
    readBytes = read(customerFileDescriptor, &account, sizeof(struct Customer));
    if (readBytes == -1)
    {
        perror("Error reading customer account!");
        fcntl(customerFileDescriptor, F_UNLCK, &lock); // Unlock before return
        close(customerFileDescriptor);
        return false;
    }

    // Verify if the account is active
    if (!account.active)
    {
        write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        fcntl(customerFileDescriptor, F_UNLCK, &lock); // Unlock before return
        close(customerFileDescriptor);
        return false;
    }

    // Prompt for withdraw amount
    writeBytes = write(connFD, WITHDRAW_AMOUNT, strlen(WITHDRAW_AMOUNT)-1);
    if (writeBytes == -1)
    {
        perror("Error writing DEPOSIT_AMOUNT to client!");
        fcntl(customerFileDescriptor, F_UNLCK, &lock); // Unlock before return
        close(customerFileDescriptor);
        return false;
    }

    // Read deposit amount from client
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading deposit money from client!");
        fcntl(customerFileDescriptor, F_UNLCK, &lock); // Unlock before return
        close(customerFileDescriptor);
        return false;
    }

    // Convert the amount and update the balance
    long int withdrawAmount = atol(readBuffer);
    if (withdrawAmount > 0)
    {
        add_transaction(account.account, 0, account.balance, account.balance - withdrawAmount);
        account.balance -= withdrawAmount;           // Update balance
        loggedInCustomer->balance = account.balance; // Update logged-in customer's balance

        // Move file pointer back to the beginning of the record before writing
        lseek(customerFileDescriptor, offset, SEEK_SET);

        // Write the updated account back to the file
        writeBytes = write(customerFileDescriptor, &account, sizeof(struct Customer));
        if (writeBytes == -1)
        {
            perror("Error storing updated deposit money in account record!");
            fcntl(customerFileDescriptor, F_UNLCK, &lock); // Unlock before return
            close(customerFileDescriptor);
            return false;
        }

        // Unlock the record after writing
        lock.l_type = F_UNLCK; // Set lock type to unlock
        fcntl(customerFileDescriptor, F_SETLK, &lock);

        // Notify the client of success
        write(connFD, WITHDRAW_AMOUNT_SUCCESS, strlen(WITHDRAW_AMOUNT_SUCCESS));
    }
    else
    {
        write(connFD, WITHDRAW_AMOUNT_INVALID, strlen(WITHDRAW_AMOUNT_INVALID));
    }

    // Cleanup
    close(customerFileDescriptor);
    return true;
}

bool get_balance(int connFD)
{
    char buffer[1000];
    // struct Customer account;
    // account.accountNumber = loggedInCustomer->account;
    // if (get_account_details(connFD, &account))
    // {
    bzero(buffer, sizeof(buffer));
    if (loggedInCustomer->active)
    {
        sprintf(buffer, "You have ₹ %ld money in our bank!^", loggedInCustomer->balance);
        write(connFD, buffer, strlen(buffer));
    }
    else
        write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
    read(connFD, buffer, sizeof(buffer)); // Dummy read
    // }
    // else
    // {
    //     // ERROR while getting balance
    //     return false;
    // }
}

bool add_transaction(int accountNumber, bool operation, long int oldBalance, long int newBalance)
{
    int fd = open(TRANSACTION_FILE, O_RDWR | O_CREAT, 0666);
    if (fd == -1)
    {
        perror("Error opening transaction file");
        return false;
    }

    // Lock the transaction file
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = 0; // Lock the whole file
    lock.l_pid = getpid();

    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        perror("Error locking transaction file");
        close(fd);
        return false;
    }

    // Calculate the transaction ID by counting the number of existing records
    off_t fileSize = lseek(fd, 0, SEEK_END);
    int transactionID = fileSize / sizeof(struct Transaction);

    // Prepare the new transaction record
    struct Transaction newTransaction;
    newTransaction.transactionID = transactionID;
    newTransaction.accountNumber = accountNumber;
    newTransaction.operation = operation;
    newTransaction.oldBalance = oldBalance;
    newTransaction.newBalance = newBalance;
    newTransaction.transactionTime = time(NULL); // Record the current time

    // Write the new transaction record to the file
    ssize_t writeBytes = write(fd, &newTransaction, sizeof(newTransaction));
    if (writeBytes == -1)
    {
        perror("Error writing to transaction file");
        close(fd);
        return false;
    }

    // Unlock the transaction file
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) == -1)
    {
        perror("Error unlocking transaction file");
        close(fd);
        return false;
    }

    // Close the file
    close(fd);
    return true;
}

bool get_transaction_details(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[10000], tempBuffer[1000];

    struct Customer account;

    // If no logged-in customer exists, prompt the client for account number
    if (loggedInCustomer->account == -1)
    {
        // Request account number from client
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

        account.account = atoi(readBuffer);
    }
    else
    {
        account.account = loggedInCustomer->account;
    }

    int transactionFileDescriptor = open(TRANSACTION_FILE, O_RDONLY);
    if (transactionFileDescriptor == -1)
    {
        perror("Error while opening transaction file!");
        write(connFD, TRANSACTIONS_NOT_FOUND, strlen(TRANSACTIONS_NOT_FOUND));
        read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    struct Transaction transaction;
    struct tm transactionTime;

    // Loop through the transaction file to find matching account number
    while (read(transactionFileDescriptor, &transaction, sizeof(transaction)) > 0)
    {
        // Check if the transaction belongs to the current customer
        if (transaction.accountNumber == account.account)
        {
            transactionTime = *localtime(&(transaction.transactionTime));

            bzero(tempBuffer, sizeof(tempBuffer));
            sprintf(tempBuffer, 
                    "Details of transaction %d - \n\tDate: %02d:%02d %02d/%02d/%d\n\tOperation: %s\n\tBalance - \n\t\tBefore: %ld\n\t\tAfter: %ld\n\t\tDifference: %ld\n", 
                    transaction.transactionID, 
                    transactionTime.tm_hour, transactionTime.tm_min, 
                    transactionTime.tm_mday, transactionTime.tm_mon + 1, 
                    transactionTime.tm_year + 1900, 
                    (transaction.operation ? "Deposit" : "Withdraw"), 
                    transaction.oldBalance, 
                    transaction.newBalance, 
                    transaction.newBalance - transaction.oldBalance);

            // Append transaction details to the write buffer
            strcat(writeBuffer, tempBuffer);
        }
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

    return true;
}

bool applyloan(int connFD) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];
    struct Loan newLoan;
    int loanFileFD;

    // Ask for the loan amount
    strcpy(writeBuffer, LOAN_APPLY_MSG);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing loan apply message to the client");
        return false;
    }

    // Read the loan amount from the client
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading loan amount from client");
        return false;
    }

    long int loanAmount = atol(readBuffer);
    if (loanAmount <= 0) {
        strcpy(writeBuffer, INVALID_AMOUNT_MSG);
        write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }

    // Initialize the new loan record
    newLoan.amount = loanAmount;
    newLoan.custID = loggedInCustomer->id;
    newLoan.status = 0; // Assigned to manager by default
    newLoan.empID = -1; // Not assigned to any employee initially

    // Open the loan file to write the new loan entry
    loanFileFD = open(LOAN_FILE, O_RDWR | O_CREAT, 0644);
    if (loanFileFD == -1) {
        perror("Error opening loan file");
        return false;
    }

    // Lock the file for writing
    struct flock lock = {F_WRLCK, SEEK_SET, 0, 0, getpid()};
    if (fcntl(loanFileFD, F_SETLKW, &lock) == -1) {
        perror("Error obtaining write lock on loan file");
        close(loanFileFD);
        return false;
    }

    // Find the next available loan ID by reading the last entry in the file
    off_t offset = lseek(loanFileFD, -sizeof(struct Loan), SEEK_END);
    if (offset == -1 && errno != EINVAL) {
        perror("Error finding end of loan file");
        close(loanFileFD);
        return false;
    }

    struct Loan lastLoan;
    if (offset >= 0) {
        // Read the last loan entry to get the ID
        readBytes = read(loanFileFD, &lastLoan, sizeof(struct Loan));
        if (readBytes == -1) {
            perror("Error reading last loan entry");
            close(loanFileFD);
            return false;
        }
        newLoan.id = lastLoan.id + 1; // Increment the loan ID
    } else {
        // If no entries exist, set the first loan ID to 0
        newLoan.id = 0;
    }

    // Write the new loan entry to the file
    if (write(loanFileFD, &newLoan, sizeof(struct Loan)) != sizeof(struct Loan)) {
        perror("Error writing new loan entry to loan file");
        close(loanFileFD);
        return false;
    }

    // Unlock the file and close it
    lock.l_type = F_UNLCK;
    fcntl(loanFileFD, F_SETLK, &lock);
    close(loanFileFD);

    // Notify the client of successful loan application
    strcpy(writeBuffer, LOAN_APPLY_SUCCESS);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer)-1);
    if (writeBytes == -1) {
        perror("Error writing loan apply success message to the client");
        return false;
    }

    return true;
}

bool loanstatus(int connFD){
    ssize_t writeBytes;
    char writeBuffer[1000];
    struct Loan loan;

    int loanFileFD = open(LOAN_FILE, O_RDONLY);
    if (loanFileFD == -1) {
        perror("Error opening loan file");
        return false;
    }

    // Lock the file to read the loans
    struct flock lock = {F_RDLCK, SEEK_SET, 0, 0, getpid()};
    if (fcntl(loanFileFD, F_SETLKW, &lock) == -1) {
        perror("Error obtaining read lock on loan file");
        close(loanFileFD);
        return false;
    }

    // Display header message to the customer
    strcpy(writeBuffer, LOAN_STATUS_MSG);
    write(connFD, writeBuffer, strlen(writeBuffer));

    bool hasLoans = false;
    // Read through the file and display all loans for the logged-in customer
    while (read(loanFileFD, &loan, sizeof(struct Loan)) == sizeof(struct Loan)) {
        if (loan.custID == loggedInCustomer->id) {
            hasLoans = true;
            // Display loan details along with status
            const char *statusString;
            switch (loan.status) {
                case 0:
                    statusString = "Applied & Assigned to Manager";
                    break;
                case 1:
                    statusString = "Assigned to Employee";
                    break;
                case 2:
                    statusString = "Accepted";
                    break;
                case 3:
                    statusString = "Rejected";
                    break;
                default:
                    statusString = "Unknown";
                    break;
            }

            snprintf(writeBuffer, sizeof(writeBuffer),
                     "Loan ID: %d, Amount: %ld, Status: %s\n",
                     loan.id, loan.amount, statusString);
            write(connFD, writeBuffer, strlen(writeBuffer));
        }
    }

    // If no loans were found for this customer, send a message indicating this
    if (!hasLoans) {
        strcpy(writeBuffer, NO_LOANS_MSG);
        write(connFD, writeBuffer, strlen(writeBuffer));
    }

    // Release the read lock and close the file
    lock.l_type = F_UNLCK;
    fcntl(loanFileFD, F_SETLK, &lock);
    close(loanFileFD);

    return true;
}
// =====================================================

#endif