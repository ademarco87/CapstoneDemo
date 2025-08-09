#include <iostream>
#include <string>
#include <fstream> // For file I/O operations
#include <sstream> // for string parsing
#include <limits> // Needed for cin.ignore
using namespace std;
#ifdef _WIN32
#include <conio.h>
#endif
// Password hiding function using _getch() for Windows systems ONLY. It allows the user to enter a password without displaying it on the screen.
string GetHiddenPassword() {
    string password;
    char ch;

    cout << "Enter your password: ";
    while (true) {
        ch = _getch();  // Windows-specific function

        if (ch == 13) { // Enter key
            break;
        }
        else if (ch == 8 && !password.empty()) { // Backspace
            cout << "\b \b";
            password.pop_back();
        }
        else if (isprint(ch)) {
            cout << '*';
            password += ch;
        }
    }
    cout << endl;
    return password;
}
// Global client data found in readable strings from objdump.
string clients[5] = {
  "Bob Jones",
  "Sara Davis",
  "Amy Friendly",
  "Johnny Smith",
  "Carol Spears"
};

// Mapping service selection for each client: 1 = Brokerage, 2 = Retirement
int services[5] = { 1, 2, 1, 1, 2 };

void DisplayInfo() {
    cout << "\nClient's Name\t\tService Selected (1 = Brokerage, 2 = Retirement)\n";
    for (int i = 0; i < 5; ++i) {
        cout << i + 1 << ". " << clients[i] << "\t\t" << services[i] << endl;
    }
}

// Function to update client service choice
void ChangeCustomerChoice() {
    int clientNum, newService;

    cout << "Enter the number of the client that you wish to change (1-5):\n";
    cin >> clientNum;

    // FIXED: Input validation and stream failure handling for clientNum
    if (cin.fail() || clientNum < 1 || clientNum > 5) {
        cin.clear(); // Clear the error flags
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
        cout << "Invalid client number.\n";
        return;
    }

    cout << "Please enter the client's new service choice (1 = Brokerage, 2 = Retirement):\n";
    cin >> newService;

    // FIXED: Input validation and stream failure handling for newService
    if (cin.fail() || (newService != 1 && newService != 2)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid service selection.\n";
        return;
    }

    services[clientNum - 1] = newService;
}

int CheckUserPermissionAccess() {
    string inputUsername;
    string inputPassword;
    string storedUsername;
    string storedPassword;

    // Load credentials from .env
    ifstream envFile(".env");
    if (!envFile.is_open()) {
        cout << "Error: Could not open .env file. Access denied.\n";
        exit(1);
    }

    string line;
    while (getline(envFile, line)) {
        size_t delimPos = line.find('=');
        if (delimPos != string::npos) {
            string key = line.substr(0, delimPos);
            string value = line.substr(delimPos + 1);
            if (key == "username") storedUsername = value;
            if (key == "password") storedPassword = value;
        }
    }
    envFile.close();

    // Username attempt logic
    const int maxAttempts = 3;
    int userAttempts = 0;
    bool usernameAccepted = false;

    while (userAttempts < maxAttempts) {
        cout << "Enter your username:\n";
        cin >> inputUsername;

        if (inputUsername.empty()) {
            cout << "Username cannot be empty.\n";
        }
        else if (inputUsername == storedUsername) {
            usernameAccepted = true;
            break;
        }
        else {
            cout << "Username not recognized. Attempt " << userAttempts + 1 << " of " << maxAttempts << ".\n";
        }

        userAttempts++;
    }

    if (!usernameAccepted) {
        cout << "Access denied. Too many failed username attempts.\n";
        exit(1);
    }

    // Password attempt logic
    int passAttempts = 0;
    while (passAttempts < maxAttempts) {
        string inputPassword = GetHiddenPassword();

        if (inputPassword == storedPassword) {
            return 1; // Success
        }
        else {
            passAttempts++;
            cout << "Incorrect password. Attempt " << passAttempts << " of " << maxAttempts << ".\n";
        }
    }

    cout << "Access denied. Too many failed password attempts.\n";
    exit(1);
}

int main() {
    cout << "Created by Alexander DeMarco for SNHU CS-410, Reverse Engineering, Project 2" << endl;
    cout << "Hello! Welcome to our Investment Company\n";

    int permission = 0;
    while (permission != 1) {
        permission = CheckUserPermissionAccess();
    }

    int choice = 0;

    do {
        cout << "\nWhat would you like to do?\n";
        cout << "1. DISPLAY the client list\n";
        cout << "2. CHANGE a client's choice\n";
        cout << "3. Exit the program\n";
        cout << "Enter your choice: ";

        cin >> choice;

        // FIXED: Input validation for choice using stream check and range enforcement
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number between 1 and 3.\n";
            continue;
        }

        if (choice == 1) {
            DisplayInfo();
        }
        else if (choice == 2) {
            ChangeCustomerChoice();
        }
        else if (choice != 3) {
            cout << "Invalid choice. Please select 1, 2, or 3.\n";
        }

    } while (choice != 3);

    cout << "Goodbye!\n";
    return 0;
}
