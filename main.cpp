#include <iostream>
#include <list>
#include <unordered_map>
#include <string.h>

struct Data_entry { 
    int value;
    int prev_value;
    bool committed;
    std::list<int> transactions;
    Data_entry(int v) :  value(v), prev_value(v) {};
};

struct Transaction {
    int tnum;
    int key;
    int value;
    Transaction(int t) : tnum(t), key(0), value(0) {};
};

class Journal {
public:
    bool unvalidated(std::unordered_map<int, Data_entry>::iterator &iter) {
        if (iter != data.end()) return false;
        printf("No data by this key\n");
        return true;
    }

    bool unvalidated(std::unordered_map<int, Transaction>::iterator &iter) {
        if (iter != running.end()) return false;
        printf("No such transaction\n");
        return true;
    }

    void init(int tnum) {
        Transaction t(tnum);
        running.insert(std::make_pair(tnum, t));
    }

    void read(int tnum, int key) {
        auto iter = data.find(key);
        auto it = running.find(tnum);
        if (unvalidated(iter) || unvalidated(it)) 
            return;
        
        Data_entry entry = iter->second;
        running.erase(it);
        printf("%d\n", ( (entry.committed) ? entry.value : 
            entry.prev_value) );
    }

    void write(int tnum, int key, int value) {
        auto it = running.find(tnum);
        if (unvalidated(it)) 
            return;

        it->second.key = key;
        it->second.value = value;

        auto iter = data.find(key);
        if (iter == data.end()) {
            Data_entry entry(value);
            entry.committed = false;
            entry.transactions.push_back(tnum);
            data.insert(std::pair<int, Data_entry>(key, entry));
            return;
        }

        iter->second.transactions.push_back(tnum);
        if (iter->second.transactions.size() != 1) {
            printf("Transaction %d is queued\n", tnum);
            return;
        }
        iter->second.prev_value = value;
        iter->second.value += value;
        iter->second.committed = false;
    }

    void commit(int tnum) {
        auto iter = running.find(tnum);
        if (unvalidated(iter)) return;
        int key = iter->second.key;
        auto it = data.find(key);
        if (unvalidated(it)) return;
        
        if (*it->second.transactions.begin() != tnum) {
            printf("Cannot commit before transaction #%d\n", 
                *it->second.transactions.begin());
            return;
        }

        it->second.committed = true;
        it->second.transactions.pop_front();
        it->second.prev_value = it->second.value;
        running.erase(tnum);

        if (it->second.transactions.size() != 0) {
            auto next = running.find(it->second.transactions.front());
            it->second.prev_value = it->second.value;
            it->second.value += next->second.value;
            it->second.committed = false;
            printf("Transaction #%d can be committed now\n", 
                it->second.transactions.front());
        }
    }
private:
    std::unordered_map<int, Transaction> running; 
    std::unordered_map<int, Data_entry> data;
};

int main() {
    std::string cmd;
    int t = 0, key = 0, value = 0;
    Journal journal; 

    while (std::cin >> cmd) {
        if (cmd == "exit") { 
            return 0;
        } else if (cmd == "init") { 
            std::cin >> t;
            journal.init(t);
        } else if (cmd == "read") { 
            std::cin >> t >> key;
            journal.read(t, key);
        } else if (cmd == "write") { 
            std::cin >> t >> key >> value;
            journal.write(t, key, value);
        } else if (cmd == "commit") { 
            std::cin >> t;
            journal.commit(t);
        } else {
            printf("Enter valid command\n");
        }
    }
}