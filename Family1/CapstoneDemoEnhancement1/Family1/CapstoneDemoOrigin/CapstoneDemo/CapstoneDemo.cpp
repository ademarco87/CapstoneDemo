#include <iostream>
#include <string>
#include <limits> // Needed for cin.ignore
using namespace std;

// Global client data found in readable strings from objdump.
string clients[5] = {
  "Bob Jones"
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

// WARNING: Security vulnerabilities remain in this function for demonstration purposes.
// These are not fixed in this implementation due to scope and assignment constraints.

int CheckUserPermissionAccess() {
    string username;
    string password;

    cout << "Enter your username:\n";
    cin >> username;

    cout << "Enter your password:\n";
    cin >> password;

    // PARTIALLY FIXED: Empty input is rejected
    if (username.empty() || password.empty()) {
        cout << "Username and password cannot be empty.\n";
        return 2;
    }

    // VULNERABILITY: Hardcoded password remains
    // WHY IT CAN’T BE FIXED: Secure password storage (e.g., hashed passwords in a database or config file)
    // requires external storage, file I/O, or database connectivity, which is beyond the scope of this assignment.
    if (password == "123") {
        return 1;
    }
    else {
        cout << "Invalid Password. Please try again\n";
        return 2;
    }

    // VULNERABILITY: Username is unused
    // WHY IT CAN’T BE FIXED: Implementing proper username validation would require a list of authorized users,
    // which would typically be stored in an external file or database. As this project is limited to static C++ code,
    // full authentication logic cannot be implemented here.

    // VULNERABILITY: Password input is shown in plain text
    // WHY IT CAN’T BE FIXED: Hiding password input from the terminal requires system-specific functions like
    // _getch() (Windows) or disabling terminal echo with termios (Unix/Linux). These require platform-dependent
    // headers and are beyond the scope of basic C++ standard I/O.
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
