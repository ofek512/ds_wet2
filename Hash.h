#include<memory>
#include "wet2util.h"

using namespace std;

#define MIN_SIZE 13
#define LOAD_FACTOR 0.7
#define SHRINK_THRESHOLD 0.3
#define REHASH_THRESHOLD 0.5

template<class T>
struct Node {
    shared_ptr<T> data;
    shared_ptr<Node> next;
    bool is_deleted;

    //constructor default, data will be null
    Node() : data(nullptr), is_deleted(false) {}

};

template<class T>
struct hashingResult {
    int index;
    StatusType status;
};

const int TABLE_OF_PRIMES[] = {
        11,         // start
        23,         // ~2×11
        47,         // ~2×23
        97,         // ~2×47
        197,        // ~2×97
        397,        // ~2×197
        797,        // ~2×397
        1601,       // ~2×797
        3203,       // ~2×1601
        6421,       // ~2×3203
        12853,      // ~2×6421
        25717,      // ~2×12853
        51437,      // ~2×25717
        102877,     // ~2×51437
        205759,     // ~2×102877
        411527,     // ~2×205759
        823117,     // ~2×411527
        1646237,    // ~2×823117
        3292489,    // ~2×1646237
        6584983,    // ~2×3292489
        13169977,   // ~2×6584983
        26339969,   // ~2×13169977
        52679969,   // ~2×26339969
        105359939,  // ~2×52679969
        210719881,  // ~2×105359939
        421439783,  // ~2×210719881
        842879579,  // ~2×421439783
        1685759167  // ~2×842879579
};

template<class T>
class Hash {
private:
    int curr_size;
    int insert_counter; // counts the number of insertions
    int prime_index; // saves the current index in TABLE_OF_PRIMES
    int max_size;
    int delete_counter;
    shared_ptr<Node<T>> *table;
    StatusType resizeTable(); /// adjust the table size according to the current load factor
    StatusType enlargeTable(); /// enlarge table in the case of load factor > 0.7
    StatusType shrinkTable(); /// shrink table in the case of load factor < 0.3
    StatusType rehash();
    hashingResult<T> hash_insert(const T &data); /// V
    hashingResult<T> hash_search(const int &data) const; /// V
public:
    StatusType insert(const T &data);
    StatusType remove(const int &key); /// V
    shared_ptr<T> member(int key) const; /// V
    int getMaxSize();
    int getHashKey(const T &data) const; /// Helper method to extract hash key from data

    Hash() : curr_size(0), insert_counter(0), prime_index(0),
             max_size(MIN_SIZE), delete_counter(0) {
        table = new shared_ptr<Node<T>>[max_size];
        for (int i = 0; i < max_size; ++i) {
            table[i] = make_shared<Node<T>>();
        }
    }

    ~Hash() {
        for(int i = 0; i < max_size; ++i) {
            shared_ptr<Node<T>> currentNode = table[i];
            while (currentNode) {
                shared_ptr<Node<T>> prevNode = currentNode;
                currentNode = currentNode->next;
                delete prevNode;
            }
        }
        delete[] table;
        table = nullptr;
    }
    shared_ptr<Node<T>> *getTable() const {
        return table;
    }

};

template<class T>
StatusType Hash<T>::resizeTable() {
    float overloadFactor = curr_size / (float)max_size;
    // check if we need to enlarge or shrink the table
    if (overloadFactor > LOAD_FACTOR) {
        return enlargeTable();
    } else if (overloadFactor < SHRINK_THRESHOLD) {
        return shrinkTable();
    }
    return StatusType::SUCCESS; // no resizing needed
}

template<class T>
StatusType Hash<T>::enlargeTable() {
    int new_max_size = TABLE_OF_PRIMES[prime_index + 1]; // update max size
    shared_ptr<Node<T>> *newTable = new shared_ptr<Node<T>>[new_max_size];
    if (!newTable) {
        return StatusType::ALLOCATION_ERROR; // memory allocation failed
    }
    else{
        //initialize new table
        for(int i = 0; i < new_max_size; ++i) {
            newTable[i] = make_shared<Node<T>>();
        }
        shared_ptr<Node<T>> *old_table = table;
        table = newTable; // point table to the new table
        // rehash all elements from the old table to the new table
        int attempts = 0;
        int i = 0;
        int availableNodes = (insert_counter - delete_counter);
        while(attempts < availableNodes && i < new_max_size) {
            if (old_table[i]->data) {
                shared_ptr<Node<T>> newNode = old_table[i];
                while (newNode && newNode->data) {
                    if (!(newNode->is_deleted)) {
                        hash_insert(*(newNode->data));
                        attempts++;// insert the data into the new table
                    }
                    newNode = newNode->next;
                }
            }
            i++;
        }
        curr_size = attempts;
        delete_counter = 0;
        prime_index++; // move to the next prime index
        max_size = new_max_size;
        delete[] old_table; // free the old table memory
        return StatusType::SUCCESS; // resizing successful
    }
}

template<class T>
StatusType Hash<T>::shrinkTable() {
    if (max_size <= MIN_SIZE || prime_index <= 0) {
        return StatusType::FAILURE; // Cannot shrink below MIN_SIZE
    }
    int new_max_size = TABLE_OF_PRIMES[prime_index - 1]; // update max size
    shared_ptr<Node<T>> *newTable = new shared_ptr<Node<T>>[new_max_size];
    if (!newTable) {
        return StatusType::ALLOCATION_ERROR; // memory allocation failed
    }
    else{
        //initialize new table
        for(int i = 0; i < new_max_size; ++i) {
            newTable[i] = make_shared<Node<T>>();
        }
        shared_ptr<Node<T>> *old_table = table;
        table = newTable; // point table to the new table
        // rehash all elements from the old table to the new table
        int attempts = 0;
        int i = 0;
        int availableNodes = (insert_counter - delete_counter);
        while(attempts < availableNodes && i < new_max_size) {
            if (old_table[i]->data) {
                shared_ptr<Node<T>> newNode = old_table[i];
                while (newNode && newNode->data) {
                    if (!(newNode->is_deleted)) {
                        hash_insert(*(newNode->data));
                        attempts++;// insert the data into the new table
                    }
                    newNode = newNode->next;
                }
            }
            i++;
        }
        curr_size = attempts;
        delete_counter = 0;
        prime_index--; // move to the next prime index
        max_size = new_max_size;
        delete[] old_table; // free the old table memory
        return StatusType::SUCCESS; // resizing successful
    }
}

template<class T>
StatusType Hash<T>::rehash(){
    shared_ptr<Node<T>> *newTable = new shared_ptr<Node<T>>[max_size];
    if (!newTable) {
        return StatusType::ALLOCATION_ERROR; // memory allocation failed
    }
    else{
        //initialize new table
        for(int i = 0; i < max_size; ++i) {
            newTable[i] = make_shared<Node<T>>();
        }
        shared_ptr<Node<T>> *old_table = table;
        table = newTable; // point table to the new table
        // rehash all elements from the old table to the new table
        int attempts = 0;
        int i = 0;
        int availableNodes = (insert_counter - delete_counter);
        while(attempts < availableNodes && i < max_size) {
            if (old_table[i]->data) {
                shared_ptr<Node<T>> newNode = old_table[i];
                while (newNode && newNode->data) {
                    if (!(newNode->is_deleted)) {
                        hash_insert(*(newNode->data));
                        attempts++;// insert the data into the new table
                    }
                    newNode = newNode->next;
                }
            }
            i++;
        }
        curr_size = attempts;
        delete_counter = 0;
        delete[] old_table; // free the old table memory
        return StatusType::SUCCESS; // resizing successful
    }
}

template<class T>
hashingResult<T> Hash<T>::hash_insert(const T &data) {
    //h(x) = x % m
    //r(x) = 1 + (x % (m - 5 ))
    //hk(x) = (h(x) + k * r(x)) % m

    // check valid input
    if (!data) {
        //return hashing result with index -1 and failure status
        return {-1, StatusType::FAILURE};
    }
    // find index, make sure there is no collision, if there is, increment k until there is no collision.
    int key = getHashKey(data);
    int index = key % max_size;
    //empty cell in the table
    if(!table[index]->data){
        table[index] = make_shared<Node<T>>();
        table[index]->data = make_shared<T>(data);
        insert_counter++;
        curr_size++;
        return {index, StatusType::SUCCESS};
    } else {
        //add chain
        shared_ptr<Node<T>> newNode = table[index];
        while (newNode && newNode->data) {
            newNode = newNode->next;
        }
        newNode->next = make_shared<Node<T>>();
        newNode->next->data = make_shared<T>(data);
        insert_counter++;
        curr_size++;
        return {index, StatusType::SUCCESS};
    }
}

template<class T>
hashingResult<T> Hash<T>::hash_search(const int &data) const {
    // check valid input
    if (!data) {
        //return hashing result with index -1 and failure status
        return {-1, StatusType::FAILURE};
    }
    // find index, make sure there is no collision, if there is, increment k until there is no collision.
    int key = data;
    int index = key % max_size;
    //empty cell in the table
    if(!table[index]->data){
        return {-1, StatusType::FAILURE};
    } else {
        shared_ptr<Node<T>> newNode = table[index];
        while (newNode && newNode->data) {
            if(getHashKey(*(newNode->data)) == key){
                return {index, StatusType::SUCCESS};
            }
            newNode = newNode->next;
        }
        return {-1, StatusType::FAILURE};
    }
}

template<class T>
shared_ptr<T> Hash<T>::member(int key) const {
    if (key < 0 || key >= max_size) {
        return nullptr; // invalid key
    }
    int index = key% max_size;
    shared_ptr<Node<T>> newNode = table[index];
    while (newNode && newNode->data) {
        if(getHashKey(*(newNode->data)) == key){
            return newNode->data; // found the data
        }
        newNode = newNode->next;
    }
    return nullptr; // Not found
}

template<class T>
StatusType Hash<T>::remove(const int &key){
    // check if we need to resize the table
    if(key < 0 || key >= max_size) {
        return StatusType::INVALID_INPUT; // invalid data
    }
    StatusType resizeStatus = resizeTable();
    if(resizeStatus != StatusType::SUCCESS) {
        return resizeStatus; // return the status of resizing
    }
    if(!key) {
        return StatusType::FAILURE; // invalid data
    } else{
        hashingResult<T> nodeFound = hash_search(key);
        if(nodeFound.status == StatusType::FAILURE) {
            return StatusType::FAILURE;
        }
        else{

            int index = nodeFound.index;
            shared_ptr<Node<T>> toDelete = table[index];
            while (toDelete && toDelete->data) {
                if(getHashKey(*(toDelete->data)) == key){
                    if(toDelete->is_deleted){
                        return StatusType::FAILURE; // already deleted
                    }
                    toDelete->is_deleted = true; // mark as deleted
                    curr_size--;
                    delete_counter++;
                    return StatusType::SUCCESS;
                }
                toDelete = toDelete->next;
            }
        }
        return StatusType::FAILURE; // Not found
    }
}

template<class T>
StatusType Hash<T>::insert(const T &data) {
    // check if we need to resize the table
    StatusType resizeStatus = resizeTable();
    if(resizeStatus != StatusType::SUCCESS) {
        return resizeStatus; // return the status of resizing
    }
    hashingResult<T> result = hash_insert(data);
    return result.status;
}

// Helper method to extract hash key from data
template<class T>
int Hash<T>::getHashKey(const T &data) const {
    // Use static_assert to ensure T can be converted to int at compile time
    return (int)*data;
}

template<class T>
int Hash<T>::getMaxSize(){
    return max_size;
}




