# CS513 - System Software - Mini Project

## Title - Design and Development of online banking management system

### Description

The Banking Management System aims to simulate the core functionalities of a bank, including customer account management, transactions, loan processing, and administrative oversight. The system features secure login mechanisms and role-based access for customers, employees, managers, and administrators. The focus is on ensuring data consistency, security, and correct handling of race conditions using file management, locking mechanisms, and process synchronization.
Roles:
1.	Customer: Customers log in using their credentials and have access to banking operations such as viewing account balances, depositing or withdrawing money, transferring funds, and applying for loans. The system ensures proper locking mechanisms to avoid race conditions when performing concurrent operations.
o	Login System (one session per user)
o	View Account Balance
o	Deposit Money
o	Withdraw Money
o	Transfer Funds
o	Apply for a Loan
o	Change Password
o	Adding Feedback
o	View Transaction History
o	Logout
o	Exit
2.	Bank Employee: Bank employees manage customer accounts, including opening, modifying, or closing accounts, as well as processing loans.
o	Login System (one session per user)
o	Add New Customer
o	Modify Customer Details
o	Process Loan Applications
o	Approve/Reject Loans
o	View Assigned Loan Applications
    View Customer Transactions( Passbook Kind of feature)
o	Change Password
o	Logout
o	Exit
3.	Manager:
o	Login System (one session per user)
o	Activate/Deactivate Customer Accounts
o	Assign Loan Application Processes to Employees
o	Review Customer Feedback
o	Change Password
o	Logout
o	Exit
4.	Administrator: The Administrator has the highest level of access and manages both customer and employee accounts.
o	Login System (one session per user)
o	Add New Bank Employee
o	Modify Customer/Employee Details
o	Manage User Roles
o	Change Password
o	Logout
o	Exit


## How to execute?

Step 1: Get the server up and running   
- Compile the server program  
    ```
        gcc -o server server.c -I./functions -I./record-struct -lcrypt
    ```
- Run the server program
    ```
        ./server
    ```

Step 2: Run the client program
- Compile the client program
    ```bash
        gcc -o client client.c -I./functions -I./re
cord-struct
    ```
- Run the client program for every user
    ```bash
        ./client
    ```
- Credentials to login as the admin  
    - Login ID : `admin`
    - Password : `admin1234`

- The credentials to login as the bank's user will be displayed during each new user's account creation.  

