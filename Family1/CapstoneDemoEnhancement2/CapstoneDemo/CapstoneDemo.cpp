#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <iomanip>
#include <fstream>
#include <sstream>
#ifdef _WIN32
#include <conio.h>
#endif
using namespace std;

// Struct for client info
struct Client {
    string name;
    int serviceType; // 1 = Brokerage, 2 = Retirement
};

// Password hiding (Windows only)
string GetHiddenPassword() {
    string password;
    char ch;

    cout << "Enter your password: ";
    while (true) {
        ch = _getch();  // Windows-only

        if (ch == 13) break;              // Enter
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

// Load clients from file
vector<Client> LoadClientsFromFile(const string& filename) {
    vector<Client> clients;
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        size_t comma = line.find(',');
        if (comma != string::npos) {
            string name = line.substr(0, comma);
            int service = stoi(line.substr(comma + 1));
            clients.push_back({ name, service });
        }
    }

    return clients;
}

// Save new client to file
void SaveClientToFile(const string& filename, const Client& client) {
    ofstream file(filename, ios::app);
    file << client.name << "," << client.serviceType << endl;
}

// Add new client
    void addNewClient(vector<Client>&clients, unordered_map<string, int>&clientLookup) {
        string name;
        int serviceType = 0; // Initialize to prevent warning

        // Prompt for and validate client name
        do {
            cout << "Enter client's full name: ";
            getline(cin >> ws, name);

            // Check for empty or purely numeric names
            bool isInvalid = name.empty() || std::all_of(name.begin(), name.end(), ::isdigit);

            if (isInvalid) {
                cout << "Invalid name. Please enter a non-empty, non-numeric name.\n";
                name.clear();
            }
            else if (clientLookup.count(name)) {
                cout << "Client with this name already exists. Cannot add duplicate.\n";
                return;
            }

        } while (name.empty());
        // Prompt for and validate service type
        while (true) {
            cout << "Select service type (1 = Brokerage, 2 = Retirement): ";
            cin >> serviceType;

            if (cin.fail() || (serviceType != 1 && serviceType != 2)) {
                cout << "Invalid selection. Please enter 1 or 2.\n";
                cin.clear(); // Clear error flag
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Flush input
            }
            else {
                break;
            }
        }

        // Add the new client to the vector and map
        Client newClient{ name, serviceType };
        clients.push_back(newClient);
        clientLookup[name] = static_cast<int>(clients.size() - 1);  // Update lookup

        // Save to file
        SaveClientToFile("clients.txt", newClient);

        cout << "Client added successfully.\n";
    }


// Search client by name
void SearchClientByName(const unordered_map<string, int>& clientLookup, const vector<Client>& clients) {
    string name;
    cout << "Enter client name to search: ";
    getline(std::cin >> std::ws, name);
    const char* serviceNames[] = { "", "Brokerage", "Retirement" };

    auto it = clientLookup.find(name);
    if (it != clientLookup.end()) {
        const Client& found = clients[it->second];
        cout << "Client found:\n"
            << "Name: " << found.name << "\n"
            << "Service Type: " << serviceNames[found.serviceType] << "\n";
    }
    else {
        cout << "Client not found.\n";
    }
}

// Delete client by name
void DeleteClientByName(std::vector<Client>& clients, std::unordered_map<std::string, int>& clientLookup) {
    std::string name;
    std::cout << "Enter the full name of the client to delete: ";
    std::getline(std::cin >> std::ws, name);

    auto it = clientLookup.find(name);
    if (it != clientLookup.end()) {
        int index = it->second;
        clients.erase(clients.begin() + index);
        clientLookup.erase(it);

        // Rebuild the lookup table to fix shifted indexes
        clientLookup.clear();
        for (int i = 0; i < clients.size(); ++i) {
            clientLookup[clients[i].name] = i;
        }

        // Overwrite the file with updated client list
        ofstream outFile("clients.txt");
        if (outFile.is_open()) {
            for (const Client& c : clients) {
                outFile << c.name << "," << c.serviceType << endl;
            }
            outFile.close();
            cout << "Client \"" << name << "\" deleted successfully and file updated.\n";
        }
        else {
            cout << "Error: Could not write to clients.txt\n";
        }
    }
    else {
        std::cout << "Client \"" << name << "\" not found.\n";
    }
}



// Display all clients
void DisplayInfo(const vector<Client>& clients) {
    cout << "\nClient's Name\t\tService Selected (1 = Brokerage, 2 = Retirement)\n";
    for (size_t i = 0; i < clients.size(); ++i) {
        cout << i + 1 << ". " << clients[i].name << "\t\t" << clients[i].serviceType << endl;
    }
}

// Change service selection
void ChangeCustomerChoice(vector<Client>& clients) {
    int clientNum, newService;
    cout << "Enter the number of the client to change: ";
    cin >> clientNum;

    if (cin.fail() || clientNum < 1 || clientNum > static_cast<int>(clients.size())) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid client number.\n";
        return;
    }

    cout << "Enter new service (1 = Brokerage, 2 = Retirement): ";
    cin >> newService;

    if (cin.fail() || (newService != 1 && newService != 2)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid service selection.\n";
        return;
    }

    clients[clientNum - 1].serviceType = newService;
    cout << "Client service updated successfully.\n";

    // Overwrite the file with updated clients
    ofstream outFile("clients.txt");
    if (outFile.is_open()) {
        for (const Client& c : clients) {
            outFile << c.name << "," << c.serviceType << endl;
        }
        outFile.close();
        cout << "Changes saved to file.\n";
    }
    else {
        cout << "Error: Could not write to clients.txt\n";
    }
}


// Check user credentials
int CheckUserPermissionAccess() {
    string inputUsername, storedUsername;
    string storedPassword;

    ifstream envFile(".env");
    if (!envFile.is_open()) {
        cout << "Error: Could not open .env file.\n";
        exit(1);
    }

    string line;
    while (getline(envFile, line)) {
        size_t delim = line.find('=');
        if (delim != string::npos) {
            string key = line.substr(0, delim);
            string value = line.substr(delim + 1);
            if (key == "username") storedUsername = value;
            if (key == "password") storedPassword = value;
        }
    }
    envFile.close();

    const int maxAttempts = 3;
    for (int i = 0; i < maxAttempts; ++i) {
        cout << "Enter your username: ";
        cin >> inputUsername;

        if (inputUsername == storedUsername) {
            for (int j = 0; j < maxAttempts; ++j) {
                string inputPassword = GetHiddenPassword();
                if (inputPassword == storedPassword) return 1;
                else cout << "Incorrect password. Attempt " << j + 1 << " of " << maxAttempts << ".\n";
            }
            break;
        }
        else {
            cout << "Incorrect username. Attempt " << i + 1 << " of " << maxAttempts << ".\n";
        }
    }

    cout << "Access denied.\n";
    exit(1);
}

int main() {
    string filename = "clients.txt";
    vector<Client> clients = LoadClientsFromFile(filename);
    
    std::unordered_map<std::string, int> clientLookup;
    for (int i = 0; i < clients.size(); ++i) {
        clientLookup[clients[i].name] = i;
    }

    cout << "Created by Alexander DeMarco for CS-499, Capstone Artifacts\n";
    cout << "Hello! Welcome to our Investment Company\n";

    CheckUserPermissionAccess();

    int choice = 0;
    do {
        cout << "\nWhat would you like to do?\n";
        cout << "1. DISPLAY the client list\n";
        cout << "2. CHANGE a client's choice\n";
        cout << "3. ADD a new client\n";
		cout << "4. SEARCH for a client by name\n";
        cout << "5. DELETE a client by name\n";
        cout << "6. Exit the program\n";
        cout << "Enter your choice: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
            continue;
        }

        switch (choice) {
        case 1:
            DisplayInfo(clients);
            break;
        case 2:
            ChangeCustomerChoice(clients);
            break;
        case 3:
            addNewClient(clients, clientLookup);
            break;
        case 4:
			SearchClientByName(clientLookup, clients);
            break;
        case 5:
            DeleteClientByName(clients, clientLookup);
			break;
        case 6:
            cout << "Exiting the program.\n";
			break;
        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 6);

    return 0;
}
