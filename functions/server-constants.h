#ifndef SERVER_CONSTANTS
#define SERVER_CONSTANTS

#define INITIAL_PROMPT "👋 Hello User!\nPlease select your role:\n1. Customer\n2. Admin\n3. Employee\n4. Manager\n\nPress any other number to exit..."

// ========== COMMON TEXT =============================

// LOGIN
#define LOGIN_ID "🔑 Please enter your login ID:"
#define PASSWORD "🔒 Enter your password:"
#define INVALID_LOGIN "🚫 The login ID specified doesn't exist!$"
#define INVALID_PASSWORD "❌ The password provided doesn't match!$"
//#define SALT_BAE "666"

// GET ACCOUNT DETAILS
#define GET_ACCOUNT_NUMBER "📄 Please enter the account number:"
#define GET_CUSTOMER_ID "👤 Enter the customer ID:"

#define ERRON_INPUT_FOR_NUMBER "⚠️ Invalid number! You'll be redirected to the main menu!^"
#define INVALID_MENU_CHOICE "⚠️ Invalid choice! You'll be redirected to the main menu!^"

#define CUSTOMER_ID_DOESNT_EXIST "😞 No customer found for this ID!"
#define CUSTOMER_LOGIN_ID_DOESNT_EXIST "😞 No customer found for this login ID!$"

#define EMP_ID_DOESNT_EXIST "😞 No employee found for this ID!"
#define EMP_LOGIN_ID_DOESNT_EXIST "😞 No employee found for this login ID!$"

#define ACCOUNT_ID_DOES_NOT_EXIST "😞 No account found for this account number!"

#define TRANSACTIONS_NOT_FOUND "📉 No transactions found for this account!^"


// ========== ADMIN SPECIFIC TEXT======================

// LOGIN WELCOME
#define ADMIN_LOGIN_WELCOME "🔐 Please enter your admin credentials:"
#define ADMIN_LOGIN_SUCCESS "🎉 Welcome ADMIN!"

// ADMIN MENU
#define ADMIN_MENU "1. Add Employee\n2. Modify Employee Details\n3. Manage User Roles\n4. Change Password\nPress any other key to logout:"

// ADD EMPLOYEE
#define ADMIN_ADD_EMPLOYEE "✍️ Enter details for the employee:\n"
#define ADMIN_ADD_EMPLOYEE_NAME "👤 Employee Name?"
#define ADMIN_ADD_EMPLOYEE_PASSWORD "🔑 Enter Password:"
#define ADMIN_ADD_EMPLOYEE_GENDER "🧑‍🤝‍🧑 Gender?\nEnter M for male, F for female, and O for others:"
#define ADMIN_ADD_EMPLOYEE_ROLE "🛠️ Role?\n(E for Employee or M for Manager):"
#define ADMIN_ADD_EMPLOYEE_AGE "🎂 Age?"
#define ADMIN_ADD_EMPLOYEE_AUTOGEN_LOGIN "🆔 The auto-generated Login ID for the employee is: "
#define EMP_AUTOGEN_PASSWORD "epass" // append to end of next string macro
#define ADMIN_ADD_EMPLOYEE_AUTOGEN_PASSWORD "🔑 The auto-generated Password for the employee is: "
#define ADMIN_ADD_EMPLOYEE_WRONG_GENDER "🚫 Invalid choice! Redirecting to the main menu!^"
#define ADMIN_ADD_EMPLOYEE_WRONG_ROLE "🚫 Invalid choice! Redirecting to the main menu!^"


// MODIFY EMPLOYEE INFO
#define ADMIN_MOD_EMPLOYEE_ID "🆔 Enter the Employee ID to edit:"
#define ADMIN_MOD_EMPLOYEE_MENU "🔧 What would you like to modify?\nPlease select: 1. Name 2. Age 3. Gender\nPress any other key to cancel:"
#define ADMIN_MOD_EMPLOYEE_NEW_NAME "✍️ New name?"
#define ADMIN_MOD_EMPLOYEE_NEW_GENDER "🧑‍🤝‍🧑 New gender?"
#define ADMIN_MOD_EMPLOYEE_NEW_AGE "🎂 New age?"
#define ADMIN_MOD_EMPLOYEE_SUCCESS "✅ Successfully modified!\nRedirecting to the main menu!^"
#define ADMIN_MOD_EMPLOYEE_ROLE "🛠️ New Role?\n(E for Employee or M for Manager):"

#define ADMIN_LOGOUT "🚪 Logging you out ADMIN...!$"


// ========== CUSTOMER SPECIFIC TEXT===================

// LOGIN WELCOME
#define CUSTOMER_LOGIN_WELCOME "👋 Welcome customer! Please enter your credentials!"
#define CUSTOMER_LOGIN_SUCCESS "🎉 Welcome! Please select an option:"

#define CUSTOMER_LOGOUT "🚪 Logging you out CUSTOMER...!$"

// CUSTOMER MENU
#define CUSTOMER_MENU "1. View Account Details\n2. View Balance\n3. Deposit Money\n4. Withdraw Money\n5. Transfer Money\n6. Apply for a Loan\n7. Change Password\n8. Check loan status\n9. View Transactions\nPress any other key to logout:"

#define ACCOUNT_DEACTIVATED "🚫 Your account is deactivated!^"

#define DEPOSIT_AMOUNT "💰 Amount to deposit?"
#define DEPOSIT_AMOUNT_INVALID "⚠️ Invalid amount!^"
#define DEPOSIT_AMOUNT_SUCCESS "✅ Money successfully deposited!^"

#define WITHDRAW_AMOUNT "💸 Amount to withdraw?"
#define WITHDRAW_AMOUNT_INVALID "🚫 You entered an invalid amount^"
#define WITHDRAW_AMOUNT_SUCCESS "✅ Money successfully withdrawn!^"

#define TRANSFER_AMOUNT "💵 Amount to transfer?"
#define TRANSFER_ACCOUNT "🏦 Account to transfer to?"
#define TRANSFER_AMOUNT_INVALID "⚠️ Invalid amount!^"
#define TRANSFER_AMOUNT_SUCCESS "✅ Money successfully transferred!^"

#define PASSWORD_CHANGE_OLD_PASS "🔑 Enter old password:"
#define PASSWORD_CHANGE_OLD_PASS_INVALID "❌ The entered password doesn't match the old password."
#define PASSWORD_CHANGE_NEW_PASS "🔑 Enter new password:"
#define PASSWORD_CHANGE_NEW_PASS_RE "🔑 Re-enter new password:"
#define PASSWORD_CHANGE_NEW_PASS_INVALID "🚫 New password and re-entered passwords don't match!^"
#define PASSWORD_CHANGE_SUCCESS "✅ Password successfully changed!^"


// ====================================================

// ========== EMP SPECIFIC TEXT======================

// LOGIN WELCOME
#define EMP_LOGIN_WELCOME "🔐 Please enter your EMP credentials:"
#define EMP_LOGIN_SUCCESS "🎉 Welcome! Please select an option:"

// EMP MENU
#define EMP_MENU "1. Add Customer\n2. Modify Customer Details\n3. Process Loan\n4. View Assigned Loan\n5. Change Password\nPress any other key to logout:"

// ADD CUSTOMER
#define EMP_ADD_CUSTOMER "✍️ Enter the details of the customer:\n"
#define EMP_ADD_CUSTOMER_NAME "👤 Name:"
#define EMP_ADD_CUSTOMER_PASSWORD "🔑 Enter the password:"
#define EMP_ADD_CUSTOMER_GENDER "🧑‍🤝‍🧑 Enter Gender?\nEnter M for male, F for female, and O for others:"
#define EMP_ADD_CUSTOMER_AGE "🎂 Age?"
#define EMP_ADD_CUSTOMER_AUTOGEN_LOGIN "🆔 The auto-generated Login ID for the customer is: "
#define CUST_AUTOGEN_PASSWORD "cpass" // append to end of next string macro
#define EMP_ADD_CUSTOMER_AUTOGEN_PASSWORD "🔑 The auto-generated Password for the customer is: "
#define EMP_ADD_CUSTOMER_WRONG_GENDER "🚫 Invalid choice! Redirecting to the main menu!^"

// ADD ACCOUNT
#define EMP_ADD_ACCOUNT_NUMBER "📄 The newly created account's number is:"
// DELETE ACCOUNT
#define EMP_DEL_ACCOUNT_NO "❌ Account number to delete?"
#define EMP_DEL_ACCOUNT_SUCCESS "✅ Account has been successfully deleted!\nRedirecting to the main menu...^"
#define EMP_DEL_ACCOUNT_FAILURE "⚠️ Account cannot be deleted as it still has a balance. Redirecting to the main menu...^"

// MODIFY CUSTOMER INFO
#define EMP_MOD_CUSTOMER_ID "🆔 Enter customer ID to edit:"
#define EMP_MOD_CUSTOMER_MENU "🔧 What would you like to modify?\n1. Name 2. Age 3. Gender\nPress any other key to cancel:"
#define EMP_MOD_CUSTOMER_NEW_NAME "✍️ New name?"
#define EMP_MOD_CUSTOMER_NEW_GENDER "🧑‍🤝‍🧑 New gender?"
#define EMP_MOD_CUSTOMER_NEW_AGE "🎂 New age?"

#define EMP_MOD_CUSTOMER_SUCCESS "✅ Successfully modified customer!\nRedirecting to the main menu...!^"

#define EMP_LOGOUT "🚪 Logging you out EMPLOYEE...!$"



//LOAN APPLY
#define LOAN_FILE "./records/loan.dat"
#define LOAN_APPLY_MSG "Enter the amount you want to apply for the loan: "
#define LOAN_APPLY_SUCCESS "Loan applied successfully and sent to manager for approval.\n"
#define LOAN_APPLY_FAILURE "Failed to apply for the loan."
#define INVALID_AMOUNT_MSG "Invalid amount entered. Please enter a positive value.\n"

//LOAN ASSIGN
#define LOAN_ASSIGN_MSG "Unassigned Loans:\n"
#define SELECT_LOAN_MSG "Enter the Loan ID you want to process: "
#define SELECT_EMP_MSG "Enter the Employee ID to assign the loan: "
#define LOAN_ASSIGN_SUCCESS "Loan assigned successfully."
#define LOAN_ASSIGN_FAILURE "Failed to assign the loan."
#define INVALID_LOAN_ID "Invalid Loan ID entered.\n"
#define INVALID_EMP_ID "Invalid Employee ID entered.\n"
#define LOAN_ASSIGNMENT_IN_PROGRESS "Processing the loan assignment...\n"

//ASSIGNMENT LOAN VIEW
#define LOAN_VIEW_MSG "Assigned Loans:\n"
#define SELECT_LOAN_TO_UPDATE_MSG "Enter the Loan ID to update: "
#define LOAN_STATUS_PROMPT "Enter the new status for the loan (2 -> Accept, 3 -> Reject): "
#define LOAN_UPDATE_SUCCESS "Loan status updated successfully.\n"
#define LOAN_UPDATE_FAILURE "Failed to update the loan status.\n"
#define INVALID_LOAN_SELECTION "Invalid Loan ID entered.\n"
#define INVALID_LOAN_STATUS "Invalid status entered. Please enter 2 (Accept) or 3 (Reject).\n"

//customer loan view
#define LOAN_STATUS_MSG "Your Loans:\n"
#define NO_LOANS_MSG "You have no active loans.\n"

// ====================================================



#define EMP_FILE "./records/emp.dat"
#define ACCOUNT_FILE "./records/account.dat"
#define CUSTOMER_FILE "./records/customer.dat"
#define TRANSACTION_FILE "./records/transactions.dat"
#define LOAN_FILE "./records/loan.dat"

#endif