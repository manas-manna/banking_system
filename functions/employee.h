#ifndef EMP_FUNCTIONS
#define EMP_FUNCTIONS

#include <sys/ipc.h>
#include <sys/sem.h>

#include "common.h"

struct Employee* loggedInEmployee = NULL;
int semIdentifier;

// Function Prototypes =================================

bool emp_operation_handler(int connFD);
bool add_customer(int connFD);
bool modify_customer_info(int connFD);
bool process_loan(int connFD);
bool view_assignedloan(int connFD);

// Function Definitions =================================


bool emp_operation_handler(int connFD)
{

    if (login_handler(3, connFD, (void**)&loggedInEmployee))
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000]; // A buffer used for reading & writing to the client
        memset(writeBuffer, 0, sizeof(writeBuffer));
        strcpy(writeBuffer, EMP_LOGIN_SUCCESS);
        
        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, EMP_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing EMP_MENU to client!");
                return false;
            }
            memset(writeBuffer, 0, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for EMP_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            switch (choice)
            {
            case 1:
                add_customer(connFD);
                break;
            case 2:
                modify_customer_info(connFD);
                break;
            case 3:
                process_loan(connFD);
            case 4:
                view_assignedloan(connFD);
            case 5:
                change_password(3,connFD,(void **)&loggedInEmployee);
                break;
            default:
                writeBytes = write(connFD, EMP_LOGOUT, strlen(EMP_LOGOUT));
                return false;
            }
        }
    }
    else
    {
        //LOGIN FAILED
        return false;
    }
    return true;
}




bool add_customer(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Customer newCustomer, previousCustomer;

    int customerFileDescriptor = open(CUSTOMER_FILE, O_RDONLY);
    struct stat fileStat;
    if (fstat(customerFileDescriptor, &fileStat) == -1)
    {
        perror("Error obtaining file statistics!");
        close(customerFileDescriptor);
        return false;
    }

    if (fileStat.st_size == 0)
    {
        // File is empty, this will be the first customer record
        newCustomer.id = 0;
        close(customerFileDescriptor); // Close the file since we're done with it
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
    newCustomer.account=newCustomer.id;
    newCustomer.active=1;
    newCustomer.balance=1000;
    memset(newCustomer.transactions, -1, MAX_TRANSACTIONS * sizeof(int));

    sprintf(writeBuffer, "%s%s", EMP_ADD_CUSTOMER, EMP_ADD_CUSTOMER_NAME);

    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing EMP_ADD_CUSTOMER_NAME message to client!");
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

    //taking password
    writeBytes = write(connFD, EMP_ADD_CUSTOMER_PASSWORD, strlen(EMP_ADD_CUSTOMER_PASSWORD));
    if (writeBytes == -1)
    {
        perror("Error writing EMP_ADD_CUSTOMER_PASSWORD message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading customer name response from client!");
        return false;
    }
    strcpy(newCustomer.password, readBuffer);
    /////////

    writeBytes = write(connFD, EMP_ADD_CUSTOMER_GENDER, strlen(EMP_ADD_CUSTOMER_GENDER));
    if (writeBytes == -1)
    {
        perror("Error writing EMP_ADD_CUSTOMER_GENDER message to client!");
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
        writeBytes = write(connFD, EMP_ADD_CUSTOMER_WRONG_GENDER, strlen(EMP_ADD_CUSTOMER_WRONG_GENDER));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, EMP_ADD_CUSTOMER_AGE);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing EMP_ADD_CUSTOMER_AGE message to client!");
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


    strcpy(newCustomer.login, newCustomer.name);
    strcat(newCustomer.login, "-");
    sprintf(writeBuffer, "%d", newCustomer.id);
    strcat(newCustomer.login, writeBuffer);


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
    sprintf(writeBuffer, "%s%s-%d\n%s%s", EMP_ADD_CUSTOMER_AUTOGEN_LOGIN, newCustomer.name, newCustomer.id, EMP_ADD_CUSTOMER_AUTOGEN_PASSWORD, newCustomer.password);
    strcat(writeBuffer, "\nRedirecting you to the main menu ...^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending customer loginID and password to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
}




bool modify_customer_info(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Customer customer;

    int customerID;

    off_t offset;
    int lockingStatus;

    writeBytes = write(connFD, EMP_MOD_CUSTOMER_ID, strlen(EMP_MOD_CUSTOMER_ID));
    if (writeBytes == -1)
    {
        perror("Error while writing EMP_MOD_CUSTOMER_ID message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading customer ID from client!");
        return false;
    }

    customerID = atoi(readBuffer);

    int customerFileDescriptor = open(CUSTOMER_FILE, O_RDONLY);
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
    
    offset = lseek(customerFileDescriptor, customerID * sizeof(struct Customer), SEEK_SET);
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

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Customer), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on customer record!");
        return false;
    }

    readBytes = read(customerFileDescriptor, &customer, sizeof(struct Customer));
    if (readBytes == -1)
    {
        perror("Error while reading customer record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(customerFileDescriptor, F_SETLK, &lock);

    close(customerFileDescriptor);

    writeBytes = write(connFD, EMP_MOD_CUSTOMER_MENU, strlen(EMP_MOD_CUSTOMER_MENU));
    if (writeBytes == -1)
    {
        perror("Error while writing EMP_MOD_CUSTOMER_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting customer modification menu choice from client!");
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

    bzero(readBuffer, sizeof(readBuffer));
    switch (choice)
    {
    case 1:
        writeBytes = write(connFD, EMP_MOD_CUSTOMER_NEW_NAME, strlen(EMP_MOD_CUSTOMER_NEW_NAME));
        if (writeBytes == -1)
        {
            perror("Error while writing EMP_MOD_CUSTOMER_NEW_NAME message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for customer's new name from client!");
            return false;
        }
        strcpy(customer.name, readBuffer);
        break;
    case 2:
        writeBytes = write(connFD, EMP_MOD_CUSTOMER_NEW_AGE, strlen(EMP_MOD_CUSTOMER_NEW_AGE));
        if (writeBytes == -1)
        {
            perror("Error while writing EMP_MOD_CUSTOMER_NEW_AGE message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for customer's new age from client!");
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
        customer.age = updatedAge;
        break;
    case 3:
        writeBytes = write(connFD, EMP_MOD_CUSTOMER_NEW_GENDER, strlen(EMP_MOD_CUSTOMER_NEW_GENDER));
        if (writeBytes == -1)
        {
            perror("Error while writing EMP_MOD_CUSTOMER_NEW_GENDER message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for customer's new gender from client!");
            return false;
        }
        customer.gender = readBuffer[0];
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

    customerFileDescriptor = open(CUSTOMER_FILE, O_WRONLY);
    if (customerFileDescriptor == -1)
    {
        perror("Error while opening customer file");
        return false;
    }
    offset = lseek(customerFileDescriptor, customerID * sizeof(struct Customer), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required customer record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on customer record!");
        return false;
    }

    writeBytes = write(customerFileDescriptor, &customer, sizeof(struct Customer));
    if (writeBytes == -1)
    {
        perror("Error while writing update customer info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(customerFileDescriptor, F_SETLKW, &lock);

    close(customerFileDescriptor);

    writeBytes = write(connFD, EMP_MOD_CUSTOMER_SUCCESS, strlen(EMP_MOD_CUSTOMER_SUCCESS));
    if (writeBytes == -1)
    {
        perror("Error while writing EMP_MOD_CUSTOMER_SUCCESS message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
}



bool process_loan(int connFD){
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];
    struct Loan loan;
    struct Employee emp;

    int loanFileFD = open(LOAN_FILE, O_RDWR);
    if (loanFileFD == -1) {
        perror("Error opening loan file");
        return false;
    }

    // Display all unassigned loans (status 0)
    strcpy(writeBuffer, LOAN_ASSIGN_MSG);
    write(connFD, writeBuffer, strlen(writeBuffer));

    // Lock the file to read the loans
    struct flock lock = {F_RDLCK, SEEK_SET, 0, 0, getpid()};
    if (fcntl(loanFileFD, F_SETLKW, &lock) == -1) {
        perror("Error obtaining read lock on loan file");
        close(loanFileFD);
        return false;
    }

    // Read through the file and display all unassigned loans
    while (read(loanFileFD, &loan, sizeof(struct Loan)) == sizeof(struct Loan)) {
        if (loan.status == 0) { // Only show unassigned loans
            snprintf(writeBuffer, sizeof(writeBuffer), "Loan ID: %d, Customer ID: %d, Amount: %ld\n",
                     loan.id, loan.custID, loan.amount);
            write(connFD, writeBuffer, strlen(writeBuffer));
        }
    }

    // Release the read lock and close the file temporarily
    lock.l_type = F_UNLCK;
    fcntl(loanFileFD, F_SETLK, &lock);
    close(loanFileFD);

    // Ask the manager to select a loan by its ID
    strcpy(writeBuffer, SELECT_LOAN_MSG);
    write(connFD, writeBuffer, strlen(writeBuffer));

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading selected loan ID from client");
        return false;
    }

    int selectedLoanID = atoi(readBuffer);
    if (selectedLoanID < 0) {
        strcpy(writeBuffer, INVALID_LOAN_ID);
        write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }

    // Ask the manager to select an employee by their ID
    strcpy(writeBuffer, SELECT_EMP_MSG);
    write(connFD, writeBuffer, strlen(writeBuffer));

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading selected employee ID from client");
        return false;
    }

    int selectedEmpID = atoi(readBuffer);
    if (selectedEmpID < 0) {
        strcpy(writeBuffer, INVALID_EMP_ID);
        write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }

    // Lock the loan file to update the selected loan
    loanFileFD = open(LOAN_FILE, O_RDWR);
    if (loanFileFD == -1) {
        perror("Error reopening loan file for writing");
        return false;
    }

    lock.l_type = F_WRLCK;
    if (fcntl(loanFileFD, F_SETLKW, &lock) == -1) {
        perror("Error obtaining write lock on loan file");
        close(loanFileFD);
        return false;
    }

    // Search for the selected loan and update it
    bool loanFound = false;
    while (read(loanFileFD, &loan, sizeof(struct Loan)) == sizeof(struct Loan)) {
        if (loan.id == selectedLoanID) {
            loan.empID = selectedEmpID;
            loan.status = 1; // Assigned to an employee
            lseek(loanFileFD, -sizeof(struct Loan), SEEK_CUR); // Move file pointer back to the beginning of this record
            if (write(loanFileFD, &loan, sizeof(struct Loan)) == -1) {
                perror("Error writing updated loan record to file");
                close(loanFileFD);
                return false;
            }
            loanFound = true;
            break;
        }
    }

    // Release the lock and close the file
    lock.l_type = F_UNLCK;
    fcntl(loanFileFD, F_SETLK, &lock);
    close(loanFileFD);

    // Notify the client of success or failure
    if (loanFound) {
        strcpy(writeBuffer, LOAN_ASSIGN_SUCCESS);
        write(connFD, writeBuffer, strlen(writeBuffer));
        return true;
    } else {
        strcpy(writeBuffer, INVALID_LOAN_ID);
        write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }
}



bool view_assignedloan(int connFD){
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];
    struct Loan loan;

    int loanFileFD = open(LOAN_FILE, O_RDWR);
    if (loanFileFD == -1) {
        perror("Error opening loan file");
        return false;
    }

    // Display all loans assigned to the logged-in employee
    strcpy(writeBuffer, LOAN_VIEW_MSG);
    write(connFD, writeBuffer, strlen(writeBuffer));

    // Lock the file to read the loans
    struct flock lock = {F_RDLCK, SEEK_SET, 0, 0, getpid()};
    if (fcntl(loanFileFD, F_SETLKW, &lock) == -1) {
        perror("Error obtaining read lock on loan file");
        close(loanFileFD);
        return false;
    }

    // Read through the file and display all loans assigned to this employee
    while (read(loanFileFD, &loan, sizeof(struct Loan)) == sizeof(struct Loan)) {
        if (loan.empID == loggedInEmployee->id && loan.status == 1) { // Only show loans assigned to this employee and with status 1
            snprintf(writeBuffer, sizeof(writeBuffer), "Loan ID: %d, Customer ID: %d, Amount: %ld, Status: %d\n",
                     loan.id, loan.custID, loan.amount, loan.status);
            write(connFD, writeBuffer, strlen(writeBuffer));
        }
    }

    // Release the read lock and close the file temporarily
    lock.l_type = F_UNLCK;
    fcntl(loanFileFD, F_SETLK, &lock);
    close(loanFileFD);

    // Ask the employee to select a loan by its ID
    strcpy(writeBuffer, SELECT_LOAN_TO_UPDATE_MSG);
    write(connFD, writeBuffer, strlen(writeBuffer));

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading selected loan ID from client");
        return false;
    }

    int selectedLoanID = atoi(readBuffer);
    if (selectedLoanID < 0) {
        strcpy(writeBuffer, INVALID_LOAN_SELECTION);
        write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }

    // Ask the employee to enter the new status (2 for Accept, 3 for Reject)
    strcpy(writeBuffer, LOAN_STATUS_PROMPT);
    write(connFD, writeBuffer, strlen(writeBuffer));

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading new loan status from client");
        return false;
    }

    int newStatus = atoi(readBuffer);
    if (newStatus != 2 && newStatus != 3) {
        strcpy(writeBuffer, INVALID_LOAN_STATUS);
        write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }

    // Lock the loan file to update the selected loan's status
    loanFileFD = open(LOAN_FILE, O_RDWR);
    if (loanFileFD == -1) {
        perror("Error reopening loan file for writing");
        return false;
    }

    lock.l_type = F_WRLCK;
    if (fcntl(loanFileFD, F_SETLKW, &lock) == -1) {
        perror("Error obtaining write lock on loan file");
        close(loanFileFD);
        return false;
    }

    // Search for the selected loan and update its status
    bool loanFound = false;
    while (read(loanFileFD, &loan, sizeof(struct Loan)) == sizeof(struct Loan)) {
        if (loan.id == selectedLoanID && loan.empID == loggedInEmployee->id && loan.status == 1) {
            loan.status = newStatus;
            lseek(loanFileFD, -sizeof(struct Loan), SEEK_CUR); // Move file pointer back to the beginning of this record
            if (write(loanFileFD, &loan, sizeof(struct Loan)) == -1) {
                perror("Error writing updated loan record to file");
                close(loanFileFD);
                return false;
            }
            loanFound = true;
            break;
        }
    }

    // Release the lock and close the file
    lock.l_type = F_UNLCK;
    fcntl(loanFileFD, F_SETLK, &lock);
    close(loanFileFD);

    // Notify the client of success or failure
    if (loanFound) {
        strcpy(writeBuffer, LOAN_UPDATE_SUCCESS);
        write(connFD, writeBuffer, strlen(writeBuffer));
        return true;
    } else {
        strcpy(writeBuffer, INVALID_LOAN_SELECTION);
        write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }
}

#endif