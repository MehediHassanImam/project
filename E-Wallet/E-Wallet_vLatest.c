// Header Files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#include <ctype.h>
#include <time.h>

// Macros
#define MAX_LENGTH 100
#define MAX_TRANSACTIONS 1000
#define MAX_REQUESTS 10
#define MAX_DIGITS 12
#define DATE_SIZE 11
#define TIME_SIZE 12
#define MAX_USERS 1000

#define RESET "\033[0m" // Reset color
#define BOLD "\033[1m"  // Bold text
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

// Structures

typedef struct transaction
{
    char type[MAX_LENGTH];
    char to[MAX_LENGTH];
    char from[MAX_LENGTH];
    char date[DATE_SIZE];
    char time[TIME_SIZE];
    double amount;

} tran;

typedef struct transactionNode
{
    tran transaction;
    struct transactionNode *next;

} transactionNode;

typedef struct request
{
    char from[MAX_LENGTH];
    double amount;

} req;

typedef struct requestNode
{
    req request;
    struct requestNode *next;

} requestNode;

typedef struct user
{
    char username[MAX_LENGTH];
    char password1[MAX_LENGTH];
    char password2[MAX_LENGTH];
    char phn_nmbr[MAX_DIGITS];
    char security_question1[2 * MAX_LENGTH];
    char security_answer1[MAX_LENGTH];
    char security_question2[2 * MAX_LENGTH];
    char security_answer2[MAX_LENGTH];

    transactionNode *transaction_head;
    requestNode *request_head;

    int transaction_count;
    int request_count;
    double balance;
    double loan_amount;

} user;

// Global Variables
user users[MAX_USERS];
int user_count = 0;
const char *filename = "wallet_data.dat";

// Function Prototypes (31)

// Utility Functions
void clear_input_buffer();
int confirm_action(const char *message);
void free_transaction_list();
transactionNode *reverse_transaction_list(transactionNode *original_head);
requestNode *reverse_request_list(requestNode *original_head);
void free_request_list();
void cleanup_users();
void timeNdate(tran *transaction);
void clear_screen();
// File Operations
void load_user_data();
void save_user_data();
void delete_user_data();
// User Registration, Login and Credentials
int find_user_index(char *username);
void set_security_QnA(user *new_user);
void reset_password(int index);
void forget_password();
int is_number_valid(const char *number);
void Register();
int login();
int all_accounts();
// Transactions
void add_money(user *current_user);
void cash_out(user *current_user);
void send_money(user *current_user);
void request_money(user *current_user);
void provide_requested_money(user *current_user);
void mobile_recharge(user *current_user);
void view_transaction_history(user *current_user);
void view_balance(user *current_user);
void pay_bill(user *current_user);
void donate_money(user *current_user);
void loan_money(user *current_user);
void repay_money(user *current_user);
// Menu
void dashboard(user *current_user);
void main_menu();
void intro();

int main()
{

    intro();

    load_user_data();
    main_menu();
    save_user_data(1);

    return 0;
}

// USER-DEFINED FUNCTIONs [utility functions]

void clear_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

int confirm_action(const char *message)
{
    char confirm;
    printf(CYAN "%s (y/n): " RESET, message);
    scanf(" %c", &confirm); // Leading space to consume any leftover newline
    clear_input_buffer();   // Clear leftover input
    return (confirm == 'y' || confirm == 'Y');
}

// Dynamic Memory Cleanup
void free_transaction_list(transactionNode *head)
{
    while (head != NULL)
    {
        transactionNode *temp = head;
        head = head->next;
        free(temp);
    }
}

void free_request_list(requestNode *head)
{
    while (head != NULL)
    {
        requestNode *temp = head;
        head = head->next;
        free(temp);
    }
}

transactionNode *reverse_transaction_list(transactionNode *original_head)
{
    transactionNode *reversed_head = NULL; // Head of the reversed list
    transactionNode *temp = NULL;          // New node for the reversed list

    while (original_head != NULL)
    {
        temp = (transactionNode *)malloc(sizeof(transactionNode));
        if (temp == NULL)
        {
            printf(RED "Error: Memory allocation failed during list reversal.\n" RESET);
            return NULL;
        }
        // Copy transaction data
        temp->transaction = original_head->transaction;

        // Insert the new node at the beginning of the reversed list
        temp->next = reversed_head;
        reversed_head = temp;

        // Move to the next node in the original list
        original_head = original_head->next;
    }
    return reversed_head; // Return the head of the reversed list
}

requestNode *reverse_request_list(requestNode *original_head)
{
    requestNode *reversed_head = NULL; // Head of the reversed list
    requestNode *temp = NULL;          // New node for the reversed list

    while (original_head != NULL)
    {
        temp = (requestNode *)malloc(sizeof(requestNode));
        if (temp == NULL)
        {
            printf(RED "Error: Memory allocation failed during list reversal.\n" RESET);
            return NULL;
        }
        // Copy request data
        temp->request = original_head->request;

        // Insert the new node at the beginning of the reversed list
        temp->next = reversed_head;
        reversed_head = temp;

        // Move to the next node in the original list
        original_head = original_head->next;
    }
    return reversed_head; // Return the head of the reversed list
}

void cleanup_users()
{
    for (int i = 0; i < user_count; i++)
    {
        free_transaction_list(users[i].transaction_head);
        free_request_list(users[i].request_head);
    }
}

void timeNdate(tran *transaction)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(transaction->date, sizeof(transaction->date), "%d-%m-%Y", t);
    strftime(transaction->time, sizeof(transaction->time), "%I:%M:%S %p", t); // %I for 12-hour format, %p for AM/PM
}


void clear_screen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif // _WIN32
}

// File Operations
void load_user_data()
{
    clear_screen();

    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        printf(YELLOW "No saved data found. Starting fresh.\n" RESET);
        Sleep(1000);
        return;
    }

    fread(&user_count, sizeof(int), 1, file);

    for (int i = 0; i < user_count; i++)
    {
        if (fread(&users[i], sizeof(user), 1, file) != 1)
        {
            printf(RED "Error loading user data. File may be corrupted.\n" RESET);
            fclose(file);
            Sleep(1000);
            return;
        }

        users[i].transaction_head = NULL;
        users[i].request_head = NULL;

        // Load transactions
        transactionNode *last_transaction = NULL;
        int transaction_count;
        if (fread(&transaction_count, sizeof(int), 1, file) != 1)
            break;

        for (int j = 0; j < transaction_count; j++)
        {
            tran t;
            if (fread(&t, sizeof(tran), 1, file) != 1)
                break;

            transactionNode *new_transaction = (transactionNode *)malloc(sizeof(transactionNode));
            new_transaction->transaction = t;
            new_transaction->next = NULL;

            if (!last_transaction)
            {
                users[i].transaction_head = new_transaction;
            }
            else
            {
                last_transaction->next = new_transaction;
            }
            last_transaction = new_transaction;
        }

        // Load requests
        requestNode *last_request = NULL;
        int request_count;
        if (fread(&request_count, sizeof(int), 1, file) != 1)
            break;

        for (int j = 0; j < request_count; j++)
        {
            req r;
            if (fread(&r, sizeof(req), 1, file) != 1)
                break;

            requestNode *new_request = (requestNode *)malloc(sizeof(requestNode));
            new_request->request = r;
            new_request->next = NULL;

            if (!last_request)
            {
                users[i].request_head = new_request;
            }
            else
            {
                last_request->next = new_request;
            }
            last_request = new_request;
        }
    }

    fclose(file);
    printf(CYAN "Data loaded successfully from - %s!\n" RESET, filename);
    Sleep(2000);
}

void save_user_data(int overwrite)
{
    FILE *file = fopen(filename, overwrite ? "wb" : "ab");
    if (!file)
    {
        printf(RED "Error opening file for saving data!\n" RESET);
        Sleep(3000);
        return;
    }

    if (overwrite)
    {
        fwrite(&user_count, sizeof(int), 1, file);
    }

    // Save new users or all users depending on the mode
    for (int i = 0; i < user_count; i++)
    {
        // Write user data
        if (fwrite(&users[i], sizeof(user), 1, file) != 1)
        {
            printf(RED "Error writing user data!\n" RESET);
            fclose(file);
            return;
        }
        // Save transactions
        transactionNode *current = users[i].transaction_head;
        int transaction_count = 0;

        while (current)
        {
            transaction_count++;
            current = current->next;
        }
        fwrite(&transaction_count, sizeof(int), 1, file);

        current = users[i].transaction_head;
        while (current)
        {
            fwrite(&current->transaction, sizeof(tran), 1, file);
            current = current->next;
        }

        // Save requests
        requestNode *current_request = users[i].request_head;
        int request_count = 0;

        while (current_request)
        {
            request_count++;
            current_request = current_request->next;
        }
        fwrite(&request_count, sizeof(int), 1, file);

        current_request = users[i].request_head;
        while (current_request)
        {
            fwrite(&current_request->request, sizeof(req), 1, file);
            current_request = current_request->next;
        }
    }

    fclose(file);

    // Provide feedback to the user
    printf(overwrite ? CYAN "Data fully overwritten to %s!\n" RESET : CYAN "New data appended to %s!\n" RESET, filename);

    Sleep(250);
}

void delete_user_data()
{
    if (remove(filename) == 0)
    {
        printf(GREEN "File deleted successfully.\n" RESET);
        user_count = 0;  // Reset user count after deletion
        cleanup_users(); // Free dynamically allocated memory
    }
    else
    {
        perror(RED "Error deleting file" RESET);
    }
    Sleep(1000);
}

int find_user_index(char *username)
{
    for (int i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            return i; // User found
        }
    }
    return -1; // User not found
}

void set_security_QnA(user *new_user)
{
    printf(CYAN "Set security question 1: " RESET);
    fgets(new_user->security_question1, sizeof(new_user->security_question1), stdin);
    strtok(new_user->security_question1, "\n");

    printf(CYAN "Answer: " RESET);
    fgets(new_user->security_answer1, MAX_LENGTH, stdin);
    strtok(new_user->security_answer1, "\n");

    printf(CYAN "Set security question 2: " RESET);
    fgets(new_user->security_question2, sizeof(new_user->security_question2), stdin);
    strtok(new_user->security_question2, "\n");

    printf(CYAN "Answer: " RESET);
    fgets(new_user->security_answer2, MAX_LENGTH, stdin);
    strtok(new_user->security_answer2, "\n");
}

void forget_password()
{
    clear_screen();

    char username[MAX_LENGTH];
    printf(CYAN "Enter your username: " RESET);
    fgets(username, MAX_LENGTH, stdin);
    strtok(username, "\n");

    int index = find_user_index(username);
    if (index == -1)
    {
        printf(RED "User not found!\n" RESET);
        Sleep(2000);
        return;
    }

    char answer1[MAX_LENGTH];
    printf(CYAN "\nQuestion 1. %s\n" RESET, users[index].security_question1);
    printf(CYAN "Answer: " RESET);
    fgets(answer1, MAX_LENGTH, stdin);
    strtok(answer1, "\n");

    char answer2[MAX_LENGTH];
    printf(CYAN "\nQuestion 2. %s\n" RESET, users[index].security_question2);
    printf(CYAN "Answer: " RESET);
    fgets(answer2, MAX_LENGTH, stdin);
    strtok(answer2, "\n");

    if (strcmp(users[index].security_answer1, answer1) == 0 &&
        strcmp(users[index].security_answer2, answer2) == 0)
    {
        printf(GREEN "Answers verified! You can now reset your password.\n" RESET);
        Sleep(1000);
        clear_screen();
        reset_password(index);
    }
    else
    {
        printf(RED "Incorrect answers. Cannot reset password.\n" RESET);
        Sleep(2000);
    }
}

void reset_password(int index)
{
    do
    {
        printf(CYAN "Create new password: " RESET);
        fgets(users[index].password1, MAX_LENGTH, stdin);
        strtok(users[index].password1, "\n");

        printf(CYAN "Confirm new password: " RESET);
        fgets(users[index].password2, MAX_LENGTH, stdin);
        strtok(users[index].password2, "\n");

        if (strcmp(users[index].password1, users[index].password2) != 0)
        {
            printf(RED "Passwords don't match! Try again.\n" RESET);
        }
    } while (strcmp(users[index].password1, users[index].password2) != 0);

    printf(GREEN "\nPassword Reset Successful!\n" RESET);
    Sleep(2000);
}

int is_number_valid(const char *number)
{
    int length = strlen(number);
    if (length != MAX_DIGITS - 1)
    {
        return 0; // Length Mismatch
    }

    for (int i = 0; i < length; i++)
    {
        if (isdigit(number[i]) == 0)
        {
            return 0; // Non-digit character found
        }
    }

    return 1; // Valid number
}

void Register()
{
    clear_screen();

    if (user_count >= MAX_USERS)
    {
        printf(RED "User limit reached! Cannot register more users.\n" RESET);
        Sleep(2000);
        return;
    }

    user new_user;
    printf(CYAN "Create username: " RESET);
    fgets(new_user.username, MAX_LENGTH, stdin);
    strtok(new_user.username, "\n"); // Remove trailing newline

    // Check if username already exists
    if (find_user_index(new_user.username) != -1)
    {
        printf(RED "Username already exists! Please choose another.\n" RESET);
        Sleep(2000);
        return;
    }

    do
    {
        printf(CYAN "Create password: " RESET);
        fgets(new_user.password1, MAX_LENGTH, stdin);
        strtok(new_user.password1, "\n");

        printf(CYAN "Confirm password: " RESET);
        fgets(new_user.password2, MAX_LENGTH, stdin);
        strtok(new_user.password2, "\n");

        if (strcmp(new_user.password1, new_user.password2) != 0)
        {
            printf(RED "Passwords don't match! Try again.\n" RESET);
        }
    } while (strcmp(new_user.password1, new_user.password2) != 0);

    // Phone number validation
    do
    {
        printf(CYAN "Add phone number (11 digits): " RESET);
        fgets(new_user.phn_nmbr, MAX_DIGITS, stdin);
        strtok(new_user.phn_nmbr, "\n");

        if (!is_number_valid(new_user.phn_nmbr))
        {
            printf(RED "\nInvalid phone number format. Try again.\n" RESET);
        }

    } while (!is_number_valid(new_user.phn_nmbr));

    clear_input_buffer();

    set_security_QnA(&new_user);

    // Initialize user data
    new_user.transaction_head = NULL;
    new_user.request_head = NULL;
    new_user.transaction_count = 0;
    new_user.request_count = 0;
    new_user.balance = 0.0;
    new_user.loan_amount = 0.0;

    // Add the user to the list
    users[user_count++] = new_user;
    printf(GREEN "Registration successful! Welcome, %s.\n" RESET, new_user.username);
    save_user_data(0); // Append the new user data to the file
    Sleep(2000);
}

int login()
{
    clear_screen();

    char username[MAX_LENGTH];
    printf(CYAN "Enter your username: " RESET);
    fgets(username, MAX_LENGTH, stdin);
    strtok(username, "\n");

    char password[MAX_LENGTH];
    printf(CYAN "Enter your password: " RESET);
    fgets(password, MAX_LENGTH, stdin);
    strtok(password, "\n");

    int index = find_user_index(username);
    if (index != -1 && strcmp(users[index].password1, password) == 0)
    {
        return index;
    }

    printf(RED "Login Failed! Invalid Credentials!\n" RESET);
    Sleep(2000);
    return -1;
}

int all_accounts()
{
    clear_screen();

    if (user_count == 0)
    {
        printf(YELLOW "No registered accounts available. Please register first.\n" RESET);
        printf(CYAN "Press any key to return to the main menu..." RESET);
        _getch();
        return -1;
    }

    printf(BLUE "REGISTERED ACCOUNTS:\n" RESET);
    printf("--------------------\n");
    for (int i = 0; i < user_count; i++)
    {
        printf("%d. %s\n", i + 1, users[i].username);
    }
    printf("--------------------\n");

    int choice;
    printf(CYAN "Choose an account to login (0 to exit): " RESET);
    while (scanf("%d", &choice) != 1 || choice < 0 || choice > user_count)
    {
        clear_input_buffer();
        printf(RED "Invalid choice! Please enter a valid number (1-%d, or 0 to exit): " RESET, user_count);
    }

    if (choice == 0)
    {
        printf(YELLOW "\nReturning to the main menu...\n" RESET);
        Sleep(1000);
        return -1;
    }

    // Adjust for zero-based index
    int account_index = choice - 1;

    ////////
    char password[MAX_LENGTH];
    printf(CYAN "\nEnter password for '%s': " RESET, users[account_index].username);
    clear_input_buffer();
    fgets(password, MAX_LENGTH, stdin);
    strtok(password, "\n");

    if (strcmp(users[account_index].password1, password) == 0)
    {
        // Return the logged-in user's index
        return account_index;
    }

    else
    {
        printf(RED "Incorrect password. Login failed.\n" RESET);
        printf("Press any key to return to the main menu...");
        _getch();
        return -1; // Indicate login failure
    }
}

void add_money(user *current_user) ///
{
    clear_screen();

    double amount;
    printf(CYAN "Enter an amount to add (0 to exit): " RESET);
    while (scanf("%lf", &amount) != 1 || amount <= 0)
    {
        clear_input_buffer();

        if (amount == 0)
        {
            printf(YELLOW "\nReturning to dashboard..." RESET);
            Sleep(1000);
            return;
        }

        else if (amount < 0)
        {
            printf(RED "\nInvalid input! Please enter a positive amount: " RESET);
        }
    }

    current_user->balance += amount;
    printf(GREEN "\nAdded %.2lf BDT to your account successfully!\n" RESET, amount);
    printf(GREEN "New Balance: %.2lf BDT\n" RESET, current_user->balance);

    // Add transaction log
    if (current_user->transaction_count < MAX_TRANSACTIONS)
    {
        transactionNode *new_transaction = (transactionNode *)malloc(sizeof(transactionNode));
        if (new_transaction == NULL)
        {
            printf(RED "Memory allocation failed for transaction log.\n" RESET);
            return;
        }

        // Populate transaction details
        strcpy(new_transaction->transaction.type, "Add Money");
        new_transaction->transaction.amount = amount;
        timeNdate(&new_transaction->transaction);

        // Add to transaction linked list
        new_transaction->next = current_user->transaction_head;
        current_user->transaction_head = new_transaction;
        current_user->transaction_count++;
    }
    else
    {
        printf(RED "Transaction limit exceeded!\n" RESET);
        Sleep(1000);
    }

    printf(CYAN "Press any key to continue..." RESET);
    _getch();
}

void cash_out(user *current_user) ///
{
    clear_screen();

    double amount;
    printf(CYAN "Enter an amount to cash-out (0 to exit): " RESET);
    while (scanf("%lf", &amount) != 1 || amount <= 0 || amount > current_user->balance)
    {
        clear_input_buffer();

        if (amount == 0)
        {
            printf(YELLOW "\nReturning to dashboard..." RESET);
            Sleep(1000);
            return;
        }

        else if (amount < 0)
        {
            printf(RED "Invalid input! Please enter a positive amount: " RESET);
        }

        else if (amount > current_user->balance)
        {
            printf(RED "Insufficient balance! Current balance: %.2lf BDT. Try again: " RESET, current_user->balance);
        }
    }

    current_user->balance -= amount;
    printf(GREEN "Cashed out %.2lf BDT successfully!\n" RESET, amount);
    printf(GREEN "New Balance: %.2lf BDT\n" RESET, current_user->balance);

    // Add transaction log
    if (current_user->transaction_count < MAX_TRANSACTIONS)
    {
        transactionNode *new_transaction = (transactionNode *)malloc(sizeof(transactionNode));
        if (new_transaction == NULL)
        {
            printf(RED "Memory allocation failed for transaction log.\n" RESET);
            return;
        }

        // Populate transaction details
        strcpy(new_transaction->transaction.type, "Cash out");
        new_transaction->transaction.amount = amount;
        timeNdate(&new_transaction->transaction);

        // Add to transaction linked list
        new_transaction->next = current_user->transaction_head;
        current_user->transaction_head = new_transaction;
        current_user->transaction_count++;
    }
    else
    {
        printf(RED "Transaction limit exceeded!\n" RESET);
        Sleep(1000);
    }

    printf(CYAN "Press any key to continue..." RESET);
    _getch();
}

void send_money(user *current_user) ///
{
    clear_screen();

    char recipient[MAX_LENGTH];
    printf(CYAN "Enter recipient's username (0 to exit): " RESET);
    clear_input_buffer();
    fgets(recipient, MAX_LENGTH, stdin);
    strtok(recipient, "\n");

    // Return to dashboard
    if (strcmp(recipient, "0") == 0)
    {
        printf(YELLOW "\nReturning to dashboard...\n" RESET);
        Sleep(1000);
        return;
    }

    // Prevent self-transfer
    if (strcmp(recipient, current_user->username) == 0)
    {
        printf(RED "Error: Can't send money to own account.\n" RESET);
        printf(CYAN "Press any key to continue..." RESET);
        _getch();
        return;
    }

    int recipient_index = find_user_index(recipient);
    if (recipient_index == -1)
    {
        printf(RED "Error: Recipient not found. Please check the username.\n" RESET);
        printf(CYAN "Press any key to continue..." RESET);
        _getch();
        return;
    }

    double amount;
    printf(CYAN "Enter amount to send (0 to exit): " RESET);
    while (scanf("%lf", &amount) != 1 || amount <= 0 || amount > current_user->balance)
    {
        clear_input_buffer();

        if (amount == 0)
        {
            printf(YELLOW "\nReturning to dashboard..." RESET);
            Sleep(1000);
            return;
        }

        else if (amount < 0)
        {
            printf(RED "Invalid input! Please enter a positive amount: " RESET);
        }

        else if (amount > current_user->balance)
        {
            printf(RED "Insufficient balance! Current balance: %.2lf BDT. Try again: " RESET, current_user->balance);
        }
    }

    current_user->balance -= amount;
    users[recipient_index].balance += amount;

    printf(GREEN "Successfully sent %.2lf BDT to %s.\n" RESET, amount, recipient);
    printf(GREEN "New Balance: %.2lf BDT\n" RESET, current_user->balance);

    // Log sender's transaction
    if (current_user->transaction_count < MAX_TRANSACTIONS)
    {
        transactionNode *sender_transaction = (transactionNode *)malloc(sizeof(transactionNode));
        if (sender_transaction == NULL)
        {
            printf(RED "Error: Memory allocation failed for sender's transaction.\n" RESET);
            return;
        }

        strcpy(sender_transaction->transaction.type, "Send Money");
        strcpy(sender_transaction->transaction.to, recipient);
        sender_transaction->transaction.amount = amount;
        timeNdate(&sender_transaction->transaction);

        sender_transaction->next = current_user->transaction_head;
        current_user->transaction_head = sender_transaction;
        current_user->transaction_count++;
    }

    // Log recipient's transaction
    if (users[recipient_index].transaction_count < MAX_TRANSACTIONS)
    {
        transactionNode *recipient_transaction = (transactionNode *)malloc(sizeof(transactionNode));
        if (recipient_transaction == NULL)
        {
            printf(RED "Error: Memory allocation failed for recipient's transaction.\n" RESET);
            return;
        }

        strcpy(recipient_transaction->transaction.type, "Received Money");
        strcpy(recipient_transaction->transaction.from, current_user->username);
        recipient_transaction->transaction.amount = amount;
        timeNdate(&recipient_transaction->transaction);

        recipient_transaction->next = users[recipient_index].transaction_head;
        users[recipient_index].transaction_head = recipient_transaction;
        users[recipient_index].transaction_count++;
    }

    printf(GREEN "Transaction recorded successfully.\n" RESET);
    printf(CYAN "Press any key to continue..." RESET);
    _getch();
}

void request_money(user *current_user) ///
{
    clear_screen();

    char recipient[MAX_LENGTH];
    printf(CYAN "Request money from (0 to exit): " RESET);
    clear_input_buffer();
    fgets(recipient, MAX_LENGTH, stdin);
    strtok(recipient, "\n");

    // Return to dashboard
    if (strcmp(recipient, "0") == 0)
    {
        printf(YELLOW "\nReturning to dashboard...\n" RESET);
        Sleep(1000);
        return;
    }

    // Validate recipient's username
    if (strcmp(recipient, current_user->username) == 0)
    {
        printf(RED "Error: You cannot request money from yourself.\n" RESET);
        printf(CYAN "Press any key to continue..." RESET);
        _getch();
        return;
    }

    int recipient_index = find_user_index(recipient);
    if (recipient_index == -1)
    {
        printf(RED "Error: Recipient username not found.\n" RESET);
        printf(CYAN "Press any key to continue..." RESET);
        _getch();
        return;
    }

    double request_amount;
    printf(CYAN "Enter amount to request (0 to exit): " RESET);
    while (scanf("%lf", &request_amount) != 1 || request_amount <= 0)
    {
        clear_input_buffer();

        if (request_amount == 0)
        {
            printf(YELLOW "\nReturning to dashboard..." RESET);
            Sleep(1000);
            return;
        }

        else if (request_amount < 0)
        {
            printf(RED "\nInvalid input! Please enter a positive amount: " RESET);
        }
    }

    // Check if the recipient has space for more requests
    if (users[recipient_index].request_count >= MAX_REQUESTS)
    {
        printf(RED "Error: Recipient's request queue is full. Cannot send request.\n" RESET);
        printf(CYAN "Press any key to continue..." RESET);
        _getch();
        return;
    }

    // Add the request to the recipient's request list
    requestNode *new_request = (requestNode *)malloc(sizeof(requestNode));
    if (new_request == NULL)
    {
        printf(RED "Error: Memory allocation failed for the request.\n" RESET);
        printf(CYAN "Press any key to continue..." RESET);
        _getch();
        return;
    }

    strcpy(new_request->request.from, current_user->username);
    new_request->request.amount = request_amount;
    new_request->next = NULL;

    /*  // 1. (PREPENDING new request) faster runtime, but the list will be fliped
        new_request->next = users[recipient_index].request_head;
        users[recipient_index].request_head =  new_request;
    */

    // 2. (APPENDING new request) slower runtime, but the list will be in order
    if (users[recipient_index].request_head == NULL)
    {
        // If the list is empty, set the new request as the head
        users[recipient_index].request_head = new_request;
    }
    else
    {
        // Traverse to the end of the list
        requestNode *ptr = users[recipient_index].request_head;
        while (ptr->next != NULL)
        {
            ptr = ptr->next;
        }
        ptr->next = new_request; // Append the new request
    }

    users[recipient_index].request_count++;

    printf(GREEN "Request for %.2lf BDT was sent to: %s successfully.\n" RESET, request_amount, recipient);
    printf(CYAN "Press any key to continue..." RESET);
    _getch();
}

void provide_requested_money(user *current_user)
{
    clear_screen();

    if (current_user->request_head == NULL)
    {
        printf(YELLOW "No pending money requests available.\n" RESET);
        printf(CYAN "Press any key to continue..." RESET);
        _getch();
        return;
    }

    // Display the list of requests
    printf(CYAN "PENDING MONEY REQUESTS:\n" RESET);
    printf("----------------------------\n");

    requestNode *temp = current_user->request_head;
    for (int index = 1; temp; index++)
    {
        printf("%d. From: %s, Amount: %.2lf BDT\n", index, temp->request.from, temp->request.amount);
        temp = temp->next;
    }

    printf("----------------------------\n");

    // Select a request to fulfill
    int choice;
    printf(CYAN "Select a request to fulfill (0 to exit): " RESET);
    while (scanf("%d", &choice) != 1 || choice < 0 || choice > current_user->request_count)
    {
        clear_input_buffer();
        printf(RED "Invalid input! Please select a valid request (0 to exit): " RESET);
    }

    if (choice == 0)
    {
        printf(YELLOW "Returning to dashboard...\n" RESET);
        Sleep(1500);
        return;
    }

    // Locate the selected request
    requestNode *prev = NULL;
    temp = current_user->request_head;
    for (int i = 1; i < choice; i++)
    {
        prev = temp;
        temp = temp->next;
    }

    double requested_amount = temp->request.amount;
    int recipient_index = find_user_index(temp->request.from);

    // Check for sufficient balance
    if (current_user->balance < requested_amount)
    {
        printf(RED "Error: Insufficient balance to fulfill the request.\n" RESET);
        printf(CYAN "Press any key to continue..." RESET);
        _getch();
        return;
    }

    // Fulfill the request
    current_user->balance -= requested_amount;
    users[recipient_index].balance += requested_amount;

    printf(GREEN "Successfully sent Requested %.2lf BDT to %s.\n" RESET, requested_amount, users[recipient_index].username);
    printf(GREEN "New Balance: %.2lf BDT\n" RESET, current_user->balance);

    // Log transaction for recipient
    transactionNode *recipient_transaction = (transactionNode *)malloc(sizeof(transactionNode));
    if (!recipient_transaction)
    {
        printf(RED "Error: Memory allocation failed. Press any key to continue..." RESET);
        _getch();
        return;
    }

    strcpy(recipient_transaction->transaction.type, "Received Money");
    strcpy(recipient_transaction->transaction.from, current_user->username);
    recipient_transaction->transaction.amount = requested_amount;
    timeNdate(&recipient_transaction->transaction);

    recipient_transaction->next = users[recipient_index].transaction_head;
    users[recipient_index].transaction_head = recipient_transaction;
    users[recipient_index].transaction_count++;

    // Log transaction for sender
    transactionNode *sender_transaction = (transactionNode *)malloc(sizeof(transactionNode));
    if (!sender_transaction)
    {
        printf(RED "Error: Memory allocation failed. Press any key to continue..." RESET);
        _getch();
        return;
    }

    strcpy(sender_transaction->transaction.type, "Send Money");
    strcpy(sender_transaction->transaction.to, users[recipient_index].username);
    sender_transaction->transaction.amount = requested_amount;
    timeNdate(&sender_transaction->transaction);

    sender_transaction->next = current_user->transaction_head;
    current_user->transaction_head = sender_transaction;
    current_user->transaction_count++;

    // Remove the fulfilled request
    if (prev)
    {
        prev->next = temp->next;
    }
    else
    {
        current_user->request_head = temp->next;
    }
    free(temp);
    current_user->request_count--;

    printf(GREEN "Request fulfilled and transaction logged successfully.\n" RESET);
    printf(CYAN "Press any key to continue..." RESET);
    _getch();
}

void mobile_recharge(user *current_user) ///
{
    clear_screen();

    char phone_number[MAX_DIGITS + 1];
    printf(CYAN "Enter Phone Number to recharge (0 to exit): " RESET);
    clear_input_buffer();
    fgets(phone_number, sizeof(phone_number), stdin);
    strtok(phone_number, "\n");

    // Return to dashboard
    if (strcmp(phone_number, "0") == 0)
    {
        printf(YELLOW "\nReturning to dashboard...\n" RESET);
        Sleep(1000);
        return;
    }

    if (!is_number_valid(phone_number))
    {
        printf(RED "Error: Invalid phone number format. Please try again.\n" RESET);
        printf(CYAN "Press any key to continue..." RESET);
        _getch();
        return;
    }

    double recharge_amount;
    printf(CYAN "Enter Recharge amount (0 to exit): " RESET);

    while (scanf("%lf", &recharge_amount) != 1 || recharge_amount <= 0 || recharge_amount > current_user->balance)
    {
        clear_input_buffer();

        if (recharge_amount == 0)
        {
            printf(YELLOW "\nReturning to dashboard..." RESET);
            Sleep(1000);
            return;
        }

        else if (recharge_amount < 0)
        {
            printf(RED "Invalid amount! Please enter a positive amount: " RESET);
        }

        else if (recharge_amount > current_user->balance)
        {
            printf(RED "Insufficient balance! Current balance: %.2lf BDT. Try again: " RESET, current_user->balance);
        }
    }

    current_user->balance -= recharge_amount;
    printf(GREEN "Recharged %.2lf BDT to %s successfully!\n" RESET, recharge_amount, phone_number);
    printf(GREEN "New Balance: %.2lf BDT\n" RESET, current_user->balance);

    // Log transaction
    if (current_user->transaction_count < MAX_TRANSACTIONS)
    {
        transactionNode *new_transaction = (transactionNode *)malloc(sizeof(transactionNode));
        if (new_transaction == NULL)
        {
            printf(RED "Error: Memory allocation failed for transaction log.\n" RESET);
            return;
        }

        strcpy(new_transaction->transaction.type, "Mobile Recharge");
        strcpy(new_transaction->transaction.to, phone_number);
        new_transaction->transaction.amount = recharge_amount;
        timeNdate(&new_transaction->transaction);

        new_transaction->next = current_user->transaction_head;
        current_user->transaction_head = new_transaction;
        current_user->transaction_count++;
    }

    printf(GREEN "Transaction recorded successfully.\n" RESET);
    printf(CYAN "Press any key to continue..." RESET);
    _getch();
}

void view_transaction_history(user *current_user)
{
    clear_screen();

    // Check if there are any transactions
    if (current_user->transaction_count == 0)
    {
        printf(YELLOW "No transactions found!\n" RESET);
        printf(CYAN "Press any key to continue..." RESET);
        _getch();
        return;
    }

    // Display Transaction History header
    printf(CYAN "TRANSACTION HISTORY:\n\n" RESET);
    printf("------------------------------\n");

    // Reverse the list
    transactionNode *view_transaction = reverse_transaction_list(current_user->transaction_head);
    if (view_transaction == NULL)
    {
        printf(RED "No transactions found!\n" RESET);
        printf(CYAN "Press any key to continue..." RESET);
        _getch();
        return;
    }

    // Iterate over each transaction and display details
    for (int i = 1; view_transaction; i++)
    {
        printf("#%d\tType: %s\n", i, view_transaction->transaction.type);

        // Display based on transaction type
        if (strcmp(view_transaction->transaction.type, "Send Money") == 0 ||
            strcmp(view_transaction->transaction.type, "Donation") == 0 ||
            strcmp(view_transaction->transaction.type, "Pay Bill") == 0 ||
            strcmp(view_transaction->transaction.type, "Mobile Recharge") == 0)
        {
            printf("\tTo: %s\n", view_transaction->transaction.to);
        }
        else if (strcmp(view_transaction->transaction.type, "Received Money") == 0)
        {
            printf("\tFrom: %s\n", view_transaction->transaction.from);
        }

        // Display the transaction amount, date, and time
        printf("\tAmount: %.2lf BDT\n", view_transaction->transaction.amount);
        printf("\tDate: %s\n", view_transaction->transaction.date);
        printf("\tTime: %s\n", view_transaction->transaction.time);
        printf("------------------------------\n");

        // Move to the next transaction in the list
        view_transaction = view_transaction->next;
    }

    // Wait for user input before returning
    printf(CYAN "Press any key to continue..." RESET);
    _getch();
}

void view_balance(user *current_user)
{
    clear_screen();

    printf(CYAN "Current Balance: %.2lf BDT\n" RESET, current_user->balance);
    printf(CYAN "Press any key to continue..." RESET);
    _getch();
}

void pay_bill(user *current_user)
{
    clear_screen();

    const char *bill_types[] = {
        "Electricity",
        "Water",
        "Internet",
        "Gas",
        "Telephone",
        "Cable TV",
        "Rent",
        "Garbage Collection",
        "Property Tax",
        "Streaming Service Fees",
        "Health Insurance",
        "College Fees",
        "Maintenance Fees",
        "Gym Membership Fees",
        "Subscription Service Fees"};

    int num_of_bills = sizeof(bill_types) / sizeof(bill_types[0]);

    printf(CYAN "Select Bill Type:\n" RESET);
    printf("--------------------\n");
    for (int i = 0; i < num_of_bills; i++)
    {
        printf("%d. %s\n", i + 1, bill_types[i]);
    }
    printf("--------------------\n");

    int choice;
    printf(CYAN "Enter your choice (0 to exit): " RESET);
    while (scanf("%d", &choice) != 1 || choice < 0 || choice > num_of_bills)
    {
        clear_input_buffer();

        if (choice == 0)
        {
            printf(YELLOW "\nReturning to dashboard..." RESET);
            Sleep(1000);
            return;
        }

        printf(RED "Invalid choice! Please enter a number between 1 and %d: " RESET, num_of_bills);
    }

    clear_screen();
    printf(CYAN "Bill Type : %s\n" RESET, bill_types[choice - 1]);

    double bill_amount;
    printf(CYAN "Enter amount to pay : " RESET);
    while (scanf("%lf", &bill_amount) != 1 || bill_amount <= 0 || bill_amount > current_user->balance)
    {
        clear_input_buffer();
        if (bill_amount == 0)
        {
            printf(YELLOW "\nReturning to dashboard..." RESET);
            Sleep(1000);
            return;
        }
        else if (bill_amount < 0)
        {
            printf(RED "Invalid amount! Please enter a positive amount: " RESET);
        }
        else if (bill_amount > current_user->balance)
        {
            printf(RED "Insufficient balance! Current balance: %.2lf BDT. Try again: " RESET, current_user->balance);
        }
    }

    current_user->balance -= bill_amount;
    printf(GREEN "Successfully paid %.2lf BDT for \"%s\".\n" RESET, bill_amount, bill_types[choice - 1]);
    printf(GREEN "Your new balance: %.2lf BDT\n" RESET, current_user->balance);

    // Log transaction
    if (current_user->transaction_count < MAX_TRANSACTIONS)
    {
        transactionNode *new_transaction = (transactionNode *)malloc(sizeof(transactionNode));
        if (new_transaction == NULL)
        {
            printf(RED "Error: Memory allocation failed for transaction log.\n" RESET);
            return;
        }

        strcpy(new_transaction->transaction.type, "Pay Bill");
        strcpy(new_transaction->transaction.to, bill_types[choice - 1]);
        new_transaction->transaction.amount = bill_amount;
        timeNdate(&new_transaction->transaction);

        new_transaction->next = current_user->transaction_head;
        current_user->transaction_head = new_transaction;
        current_user->transaction_count++;
    }

    printf(GREEN "Transaction recorded successfully.\n" RESET);
    printf(CYAN "Press any key to continue..." RESET);
    _getch();
}

void donate_money(user *current_user)
{
    clear_screen();

    const char *organizations[] = {
        "Hope Foundation",
        "Care & Share Initiative",
        "Bright Futures Trust",
        "Global Kindness Network",
        "Helping Hands Alliance",
        "Smile Makers Foundation",
        "Beacon of Hope",
        "Green Earth Initiative",
        "Hearts United Charity",
        "Children's Dreams Fund",
        "Lifeline Support Center",
        "Empower Change Organization",
        "Community Builders",
        "Food for All Program",
        "Safe Haven Shelter"};

    int num_of_org = sizeof(organizations) / sizeof(organizations[0]);

    printf(CYAN "Select Organization/Cause to Donate:\n" RESET);
    printf("--------------------------------\n");
    for (int i = 0; i < 10; i++)
    {
        printf("%d. %s\n", i + 1, organizations[i]);
    }
    printf("--------------------------------\n");

    int choice;
    printf(CYAN "Enter your choice (0 to exit): " RESET);
    while (scanf("%d", &choice) != 1 || choice < 0 || choice > num_of_org)
    {
        clear_input_buffer();

        if (choice == 0)
        {
            printf(YELLOW "\nReturning to dashboard..." RESET);
            Sleep(1000);
            return;
        }

        printf(RED "Invalid choice! Please enter a number between 1 and %d: " RESET, num_of_org);
    }

    double donation_amount;
    printf(CYAN "Enter donation amount (0 to exit): " RESET);
    while (scanf("%lf", &donation_amount) != 1 || donation_amount <= 0 || donation_amount > current_user->balance)
    {
        clear_input_buffer();
        if (donation_amount == 0)
        {
            printf(YELLOW "\nReturning to dashboard..." RESET);
            Sleep(1000);
            return;
        }
        else if (donation_amount <= 0)
        {
            printf(RED "Invalid amount! Please enter a positive amount: " RESET);
        }
        else if (donation_amount > current_user->balance)
        {
            printf(RED "Insufficient balance! Current balance: %.2lf BDT. Try again: " RESET, current_user->balance);
        }
    }

    current_user->balance -= donation_amount;
    printf(GREEN "Successfully donated %.2lf BDT to \"%s\"\n" RESET, donation_amount, organizations[choice - 1]);
    printf(GREEN "New Balance: %.2lf BDT\n" RESET, current_user->balance);

    // Log transaction
    if (current_user->transaction_count < MAX_TRANSACTIONS)
    {
        transactionNode *new_transaction = (transactionNode *)malloc(sizeof(transactionNode));
        if (new_transaction == NULL)
        {
            printf(RED "Error: Memory allocation failed for transaction log.\n" RESET);
            return;
        }

        strcpy(new_transaction->transaction.type, "Donation");
        strcpy(new_transaction->transaction.to, organizations[choice - 1]);
        new_transaction->transaction.amount = donation_amount;
        timeNdate(&new_transaction->transaction);

        new_transaction->next = current_user->transaction_head;
        current_user->transaction_head = new_transaction;
        current_user->transaction_count++;
    }

    printf(GREEN "Transaction recorded successfully.\n" RESET);
    printf(CYAN "Press any key to continue..." RESET);
    _getch();
}

void loan_money(user *current_user)
{
    clear_screen();

    double requested_loan_amount;
    printf(CYAN "Enter loan amount (0 to exit): " RESET);

    while (scanf("%lf", &requested_loan_amount) != 1 || requested_loan_amount <= 0 || requested_loan_amount > 0.5 * current_user->balance)
    {
        clear_input_buffer();

        if (requested_loan_amount == 0)
        {
            printf(YELLOW "\nReturning to dashboard..." RESET);
            Sleep(1000);
            return;
        }
        else if (requested_loan_amount < 0)
        {
            printf(RED "Please enter a positive amount.\n" RESET);
        }
        else if (requested_loan_amount > 0.5 * current_user->balance)
        {
            printf(RED "Loan Limit Exceeded! (Max Loan: 50%% of user balance), Current balance: %.2lf BDT.\nTry again: " RESET, current_user->balance);
        }
    }

    // Update user balance and loan amount
    current_user->balance += requested_loan_amount;
    current_user->loan_amount += requested_loan_amount;

    printf(GREEN "\nNew Balance: %.2lf BDT\n" RESET, current_user->balance);
    printf(GREEN "New Loan Amount: %.2lf BDT\n" RESET, current_user->loan_amount);

    // Create new transaction for loan request
    if (current_user->transaction_count < MAX_TRANSACTIONS)
    {
        transactionNode *new_transaction = (transactionNode *)malloc(sizeof(transactionNode));
        if (new_transaction == NULL)
        {
            printf(RED "Error: Memory allocation failed for Loan!\n" RESET);
            return;
        }

        // Populate transaction details
        strcpy(new_transaction->transaction.type, "Loan");
        new_transaction->transaction.amount = requested_loan_amount;
        timeNdate(&new_transaction->transaction);

        // Add transaction to the list
        new_transaction->next = current_user->transaction_head;
        current_user->transaction_head = new_transaction;
        current_user->transaction_count++;
    }

    printf(GREEN "Transaction recorded successfully.\n" RESET);
    printf(CYAN "Press any key to continue..." RESET);
    _getch();
}

void repay_money(user *current_user)
{
    clear_screen();

    if (current_user->loan_amount == 0)
    {
        printf(RED "No loan to repay.\n" RESET);
        printf(CYAN "Press any key to continue..." RESET);
        _getch();
        return;
    }

    printf(CYAN "Loan Amount Pending: %.2lf BDT\n\n" RESET, current_user->loan_amount);

    double repay_amount;
    printf(CYAN "Enter amount to Repay (0 to exit): " RESET);

    while (scanf("%lf", &repay_amount) != 1 || repay_amount <= 0 || repay_amount > current_user->balance || repay_amount > current_user->loan_amount)
    {
        clear_input_buffer();

        if (repay_amount == 0)
        {
            printf(YELLOW "\nReturning to dashboard..." RESET);
            Sleep(1000);
            return;
        }
        else if (repay_amount > current_user->balance)
        {
            printf(RED "Insufficient balance! Current balance: %.2lf BDT. Try again: " RESET, current_user->balance);
        }

        if (repay_amount < 0)
        {
            printf(RED "Please enter a positive amount: " RESET);
        }

        if (repay_amount > current_user->loan_amount)
        {
            printf(RED "Repay amount exceeded the pending loan amount! Try again:" RESET);
        }
    }

    // Update user's balance and loan amount
    current_user->balance -= repay_amount;
    current_user->loan_amount -= repay_amount;

    printf(GREEN "Successfully repaid %.2lf BDT.\n" RESET, repay_amount);
    printf(GREEN "New Loan Amount: %.2lf BDT\n" RESET, current_user->loan_amount);
    printf(GREEN "New Balance: %.2lf BDT\n" RESET, current_user->balance);

    // Create new transaction for loan repayment
    if (current_user->transaction_count < MAX_TRANSACTIONS)
    {
        transactionNode *new_transaction = (transactionNode *)malloc(sizeof(transactionNode));
        if (new_transaction == NULL)
        {
            printf(RED "Error: Memory allocation failed for Repay Loan!\n" RESET);
            return;
        }

        // Populate transaction details
        strcpy(new_transaction->transaction.type, "Repay Loan");
        new_transaction->transaction.amount = repay_amount;
        timeNdate(&new_transaction->transaction);

        // Add transaction to the list
        new_transaction->next = current_user->transaction_head;
        current_user->transaction_head = new_transaction;
        current_user->transaction_count++;
    }

    printf(GREEN "Transaction recorded successfully.\n" RESET);
    printf(CYAN "Press any key to continue..." RESET);
    _getch();
}

void dashboard(user *current_user)
{
    while (1)
    {
        clear_screen();

        printf(YELLOW "DASHBORAD (%s)\n" RESET, current_user->username);
        printf("-----------------------------\n");
        printf("1. Add Money\n");
        printf("2. Cash-Out\n");
        printf("3. Send Money\n");
        printf("4. Request Money\n");
        printf("5. Provide Requested Money\n");
        printf("6. Mobile Recharge\n");
        printf("7. View Transaction History\n");
        printf("8. View Balance\n");
        printf("9. Pay Bill\n");
        printf("10. Donate Money\n");
        printf("11. Loan Money\n");
        printf("12. Repay Loaned Money\n");
        printf("0. Logout\n");
        printf("-----------------------------\n");

        printf(CYAN "Enter your choice: " RESET);

        int choice;
        if (scanf("%d", &choice) != 1 || choice < 0 || choice > 12)
        {
            clear_input_buffer();
            printf(RED "Invalid choice! Please select a valid option.\n" RESET);
            Sleep(1000);
            continue;
        }

        switch (choice)
        {
        case 1:
            add_money(current_user);
            break;
        case 2:
            cash_out(current_user);
            break;
        case 3:
            send_money(current_user);
            break;
        case 4:
            request_money(current_user);
            break;
        case 5:
            provide_requested_money(current_user);
            break;
        case 6:
            mobile_recharge(current_user);
            break;
        case 7:
            view_transaction_history(current_user);
            break;
        case 8:
            view_balance(current_user);
            break;
        case 9:
            pay_bill(current_user);
            break;
        case 10:
            donate_money(current_user);
            break;
        case 11:
            loan_money(current_user);
            break;
        case 12:
            repay_money(current_user);
            break;

        case 0:
            clear_screen();
            printf(GREEN "Logging " RESET);
            Sleep(200);
            printf(GREEN "Out" RESET);
            Sleep(200);
            for (int i = 0; i < 3; i++)
            {
                printf(GREEN "." RESET);
                Sleep(200);
            }
            Sleep(1000);
            return;

        default:
            printf(RED "Invalid choice!\n" RESET);
            Sleep(1000);
            break;
        }
    }
}

void main_menu()
{
    while (1)
    {
        clear_screen();
        printf(YELLOW "MAIN MENU\n" RESET);
        printf("----------------------\n");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Forgot Password\n");
        printf("4. User Accounts\n");
        printf("5. Delete All Accounts\n");
        printf("0. Exit\n");
        printf("----------------------\n");

        printf(CYAN "Enter your choice: " RESET);
        int choice;
        if (scanf("%d", &choice) != 1)
        {
            printf(RED "Invalid input! Please enter a number.\n" RESET);
            clear_input_buffer();
            Sleep(1000);
            continue;
        }
        clear_input_buffer();

        switch (choice)
        {
        case 1:
            Register();
            break;

        case 2:
        {
            int index = login();

            if (index != -1)
            {
                clear_screen();

                printf(GREEN "Logging " RESET);
                Sleep(200);
                printf(GREEN "In" RESET);
                Sleep(200);

                for (int i = 0; i < 3; i++)
                {
                    printf(GREEN "." RESET);
                    Sleep(200);
                }
                Sleep(1000);

                dashboard(&users[index]);
            }
            break;
        }

        case 3:
            forget_password();
            break;

        case 4:
        {
            int index = all_accounts();

            if (index != -1)
            {
                clear_screen();

                printf(GREEN "Logging " RESET);
                Sleep(200);
                printf(GREEN "In" RESET);
                Sleep(200);

                for (int i = 0; i < 3; i++)
                {
                    printf(GREEN "." RESET);
                    Sleep(200);
                }
                Sleep(1000);

                dashboard(&users[index]);
            }
            break;
        }

        case 5:
            if (confirm_action("Are you sure you want to delete user data"))
            {
                delete_user_data();
            }
            else
            {
                printf(YELLOW "Data deletion canceled.\n" RESET);
            }
            break;

        case 0:
            printf(GREEN "\nExiting the program. Goodbye!\n" RESET);
            Sleep(250);
            return;

        default:
            printf(RED "Invalid choice!\n" RESET);
            Sleep(1000);
            break;
        }
    }
}

void intro()
{
    system("color 0F");
    printf(CYAN "\n\tWELCOME " RESET);
    Sleep(150);
    printf(CYAN "TO " RESET);
    Sleep(150);
    printf(MAGENTA "\"BitCash " RESET);
    Sleep(150);
    printf(MAGENTA "E-WALLET " RESET);
    Sleep(150);
    printf(MAGENTA "SERVICE\" " RESET);
    Sleep(1000);
    printf(GREEN "\n\n\t (Secure " RESET);
    Sleep(150);
    printf(GREEN "& " RESET);
    Sleep(150);
    printf(GREEN "Convenient " RESET);
    Sleep(150);
    printf(GREEN "Transactions)" RESET);
    Sleep(2000);
}