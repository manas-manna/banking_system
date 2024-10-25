#ifndef LOAN_RECORD
#define LOAN_RECORD



struct Loan
{
    int id; // 0, 1, 2 ....
    int custID;
    int empID;
    int status;           // 0 -> applied & assigned to manager, 1 -> Assigned to employee 2->Accepted 3->Rejected
    long int amount;      // Amount of money in the account
};

#endif