#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define MAX_TRANSACTIONS 10

struct Transaction {
    int transactionID;
    int accountNumber;
    bool operation; // 0 -> Withdraw, 1 -> Deposit
    long int oldBalance;
    long int newBalance;
    time_t transactionTime;
};

struct Employee {
    int id;
    char name[25];
    char gender; // M -> Male, F -> Female, O -> Other
    int age;
    char role; // e -> employee, m -> manager
    char login[30];
    char password[30];
};

struct Customer
{
    int id; // 0, 1, 2 ....
    char name[25];
    char gender; // M -> Male, F -> Female, O -> Other
    int age;
    // Login Credentials
    char login[30]; // Format : name-id (name will the first word in the structure member `name`)
    char password[30];
    // Bank data
    int account; // Account number of the account the customer owns
    bool active;           // 1 -> Active, 0 -> Deactivated (Deleted)
    long int balance;      // Amount of money in the account
    int transactions[MAX_TRANSACTIONS];  // A list of transaction IDs. Used to look up the transactions. // -1 indicates unused space in array
};

void readTransaction(int fd) {
    struct Transaction t;
    ssize_t bytesRead;

    printf("Reading Transactions:\n");
    while ((bytesRead = read(fd, &t, sizeof(struct Transaction))) > 0) {
     char timeStr[26]; // Buffer to hold the formatted time string
        ctime_r(&t.transactionTime, timeStr); // Convert time to string
        timeStr[strcspn(timeStr, "\n")] = '\0'; // Remove newline character

        printf("Transaction ID: %d, Account: %d, Operation: %d, Old Balance: %ld, New Balance: %ld, Time: %s\n",
               t.transactionID, t.accountNumber, t.operation, t.oldBalance, t.newBalance, timeStr);
    }

    if (bytesRead == -1) {
        perror("Error reading transaction");
    }
}



void readEmployee(int fd) {
    struct Employee e;
    ssize_t bytesRead;

    printf("\nReading Employees:\n");
    while ((bytesRead = read(fd, &e, sizeof(struct Employee))) > 0) {
        printf("Employee ID: %d, Name: %s, Gender: %c, Age: %d, Role: %c, Login: %s, Password: %s\n",
               e.id, e.name, e.gender, e.age, e.role,e.login,e.password);
    }

    if (bytesRead == -1) {
        perror("Error reading employee");
    }
}

void readCustomer(int fd) {
    struct Customer c;
    ssize_t bytesRead;

    printf("\nReading Customers:\n");
    while ((bytesRead = read(fd, &c, sizeof(struct Customer))) > 0) {
        printf("Customer ID: %d, Name: %s, Gender: %c, Age: %d, Login: %s, Password: %s, Account: %d, Active: %d, Balance: %ld\n",c.id, c.name, c.gender, c.age,c.login,c.password,c.account,c.active,c.balance);
    }

    if (bytesRead == -1) {
        perror("Error reading customer");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <transaction_file> <employee_file> <customer_file>\n", argv[0]);
        return 1;
    }

    int fd;

    // Read Transactions
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening transaction file");
        return 1;
    }
    readTransaction(fd);
    close(fd);


    // Read Employees
    fd = open(argv[2], O_RDONLY);
    if (fd == -1) {
        perror("Error opening employee file");
        return 1;
    }
    readEmployee(fd);
    close(fd);

    // Read Customers
    fd = open(argv[3], O_RDONLY);
    if (fd == -1) {
        perror("Error opening customer file");
        return 1;
    }
    readCustomer(fd);
    close(fd);

    return 0;
}

