#ifndef EMP_RECORD
#define EMP_RECORD

struct Employee
{
    int id; // 0, 1, 2 ....
    char name[25];
    char gender; // M -> Male, F -> Female, O -> Other
    int age;
    char role; // e->employee, m->manager   
    // Login Credentials
    char login[30]; // Format : name-id
    char password[30];
};

#endif