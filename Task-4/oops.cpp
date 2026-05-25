#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <ctime>

using namespace std;

class InsufficientBalanceException : public runtime_error {
public:
    InsufficientBalanceException() : runtime_error("Error: Insufficient balance for this transaction.") {}
};

class InvalidPINException : public runtime_error {
public:
    InvalidPINException() : runtime_error("Error: The entered PIN is invalid.") {}
};

class AccountBlockedException : public runtime_error {
public:
    AccountBlockedException() : runtime_error("Error: This account is currently blocked.") {}
};

class LoanRejectedException : public runtime_error {
public:
    LoanRejectedException() : runtime_error("Error: The loan application was rejected.") {}
};


class Account;
class Branch;
class Customer;
class Transaction;


class Notification {
public:
    virtual void sendNotification() = 0;
    virtual ~Notification() = default;
protected:
    string message; 
};

class SMSNotification : public Notification {
private:
    string phoneNumber;
    string deliveryStatus;
public:
    SMSNotification(string phone, string msg) : phoneNumber(phone) {
        this->message = msg;
        this->deliveryStatus = "Pending";
    }
    void sendNotification() override {
        cout << "[SMS to " << phoneNumber << "] " << message << endl;
        deliveryStatus = "Sent";
    }
};

class EmailNotification : public Notification {
private:
    string emailAddress;
    string subject;
    string deliveryStatus;
public:
    EmailNotification(string email, string sub, string msg) 
        : emailAddress(email), subject(sub) {
        this->message = msg;
        this->deliveryStatus = "Pending";
    }
    void sendNotification() override {
        cout << "[Email to " << emailAddress << " | Subject: " << subject << "] " << message << endl;
        deliveryStatus = "Sent";
    }
};


class Employee {
public:
    int employeeId;
    string employeeName;
    string designation;
    double salary;
    weak_ptr<Branch> branch;

    Employee(int id, string name, string desig, double sal) 
        : employeeId(id), employeeName(name), designation(desig), salary(sal) {}
};

class Loan {
public:
    int loanId;
    string loanType;
    double loanAmount;
    double interestRate;
    int tenureYears;
    double EMIAmount;
    string loanStatus;
    weak_ptr<Customer> customer;

    Loan(int id, string type, double amt, double rate, int tenure)
        : loanId(id), loanType(type), loanAmount(amt), interestRate(rate), tenureYears(tenure), loanStatus("Pending") {
        EMIAmount = (loanAmount + (loanAmount * interestRate * tenureYears)) / (tenureYears * 12);
    }
};

class ATMCard {
public:
    long cardNumber;
    int CVV;
    string expiryDate;
    int PIN;
    string cardType;
    string cardStatus;
    weak_ptr<Account> linkedAccount;

    ATMCard(long num, int cvv, string exp, int pin, string type)
        : cardNumber(num), CVV(cvv), expiryDate(exp), PIN(pin), cardType(type), cardStatus("Active") {}
};

class Transaction {
public:
    int transactionId;
    string transactionType;
    double amount;
    string transactionDate;
    weak_ptr<Account> senderAccount;
    weak_ptr<Account> receiverAccount;
    string status;

    Transaction(int id, string type, double amt, string date)
        : transactionId(id), transactionType(type), amount(amt), transactionDate(date), status("Pending") {}
};

class Customer {
public:
    int customerId;
    string fullName;
    string dob;
    string gender;
    string mobileNumber;
    string email;
    string address;
    string aadhaarNumber;
    string PANNumber;
    
    vector<shared_ptr<Account>> accounts;
    vector<shared_ptr<Loan>> loans;

    Customer(int id, string name, string phone, string mail)
        : customerId(id), fullName(name), mobileNumber(phone), email(mail) {}
};

class Branch {
public:
    int branchId;
    string branchName;
    string IFSCCode;
    string address;
    
    vector<shared_ptr<Account>> accounts;
    vector<shared_ptr<Employee>> employees;

    Branch(int id, string name, string ifsc, string addr)
        : branchId(id), branchName(name), IFSCCode(ifsc), address(addr) {}
};

class Account {
public:
    long accountNumber;
    string accountType;
    double balance;
    string dateOpened;
    string status;
    
    weak_ptr<Branch> branch;
    weak_ptr<Customer> customer;
    vector<shared_ptr<Transaction>> transactions;

    Account(long accNum, string type, double bal)
        : accountNumber(accNum), accountType(type), balance(bal), status("Active") {}

    virtual ~Account() = default;

    virtual void withdraw(double amount) = 0;
    virtual void deposit(double amount) = 0;
};

class SavingsAccount : public Account {
public:
    double interestRate;
    double minimumBalance;

    SavingsAccount(long accNum, double bal, double intRate, double minBal)
        : Account(accNum, "Savings", bal), interestRate(intRate), minimumBalance(minBal) {}

    void withdraw(double amount) override {
        if (status == "Blocked") throw AccountBlockedException();
        if (balance - amount < minimumBalance) throw InsufficientBalanceException();
        balance -= amount;
        cout << "Successfully withdrew $" << amount << " from Savings. New Balance: $" << balance << "\n";
    }

    void deposit(double amount) override {
        if (status == "Blocked") throw AccountBlockedException();
        balance += amount;
        cout << "Successfully deposited $" << amount << " into Savings. New Balance: $" << balance << "\n";
    }
};

class CurrentAccount : public Account {
public:
    double overdraftLimit;
    string businessName;

    CurrentAccount(long accNum, double bal, double overdraft, string bName)
        : Account(accNum, "Current", bal), overdraftLimit(overdraft), businessName(bName) {}

    void withdraw(double amount) override {
        if (status == "Blocked") throw AccountBlockedException();
        if (balance + overdraftLimit < amount) throw InsufficientBalanceException();
        balance -= amount;
        cout << "Successfully withdrew $" << amount << " using Current Account. New Balance: $" << balance << "\n";
    }

    void deposit(double amount) override {
        if (status == "Blocked") throw AccountBlockedException();
        balance += amount;
        cout << "Successfully deposited $" << amount << " into Current Account. New Balance: $" << balance << "\n";
    }
};

class FixedDepositAccount : public Account {
public:
    double FDAmount;
    string maturityDate;
    double FDInterestRate;
    int tenureMonths;

    FixedDepositAccount(long accNum, double amt, double rate, int tenure, string maturity)
        : Account(accNum, "FixedDeposit", amt), FDAmount(amt), FDInterestRate(rate), tenureMonths(tenure), maturityDate(maturity) {}

    // Overrides added to satisfy pure virtual obligations safely based on maturity lock logic [cite: 50]
    void withdraw(double amount) override {
        throw runtime_error("Error: Cannot withdraw from Fixed Deposit account prior to maturity date " + maturityDate + ".");
    }

    void deposit(double amount) override {
        throw runtime_error("Error: Direct balance accumulation deposits cannot be processed on structured Fixed Deposits.");
    }
};

class AccountFactory {
public:
    static shared_ptr<Account> createAccount(const string& type, long accNum, double initialBalance) {
        if (type == "Savings") {
            return make_shared<SavingsAccount>(accNum, initialBalance, 4.0, 500.0);
        } else if (type == "Current") {
            return make_shared<CurrentAccount>(accNum, initialBalance, 10000.0, "General Business");
        } else if (type == "FixedDeposit") {
            return make_shared<FixedDepositAccount>(accNum, initialBalance, 6.5, 12, "2027-05-20");
        }
        throw invalid_argument("Unknown account type requested.");
    }
};


class Bank {
public:
    int bankId;
    string bankName;
    
    vector<shared_ptr<Branch>> branches;
    vector<shared_ptr<Customer>> customers;
    vector<shared_ptr<Employee>> employees;

    Bank(int id, string name) : bankId(id), bankName(name) {}
};


int main() {
    try {
        cout << "--- SmartBank Enterprise System Initialization ---\n";
        
        Bank myBank(1, "SmartBank Global");
        auto mainBranch = make_shared<Branch>(101, "Downtown Branch", "SBG000101", "123 Main St");
        myBank.branches.push_back(mainBranch);

        auto cust1 = make_shared<Customer>(1001, "John Doe", "555-0192", "john.doe@email.com");
        myBank.customers.push_back(cust1);

        cout << "\n--- Creating Accounts ---\n";
        auto savings = AccountFactory::createAccount("Savings", 99887766, 1000.0);
        savings->customer = cust1;
        savings->branch = mainBranch;
        cust1->accounts.push_back(savings);
        mainBranch->accounts.push_back(savings);

        cout << "\n--- Processing Transactions ---\n";
        savings->deposit(500.0); 
        
        cout << "Attempting to withdraw $1200\n";
        savings->withdraw(1200.0); 

        cout << "Attempting to withdraw $10000\n";
        savings->withdraw(10000.0); 

    } catch (const exception& e) {
        cerr << "EXCEPTION : " << e.what() << "\n";
    }

    cout << "\n--- Testing Notifications ---\n";
    unique_ptr<Notification> sms = make_unique<SMSNotification>("555-0192", "Your account was accessed.");
    sms->sendNotification();

    unique_ptr<Notification> email = make_unique<EmailNotification>("suchir@email.com", "Security Alert", "New login detected from unknown device.");
    email->sendNotification();

    return 0;
}