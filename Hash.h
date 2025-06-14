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
    hashingResult<T> hash_search(const T &data); /// V
public:
    StatusType insert(const T &data);
    StatusType remove(const T &data); /// V
    shared_ptr<T> member(int key) const; /// V
    int getHashKey(const T &data) const; /// Helper method to extract hash key from data

    Hash() : curr_size(0), insert_counter(0), prime_index(0),
             max_size(MIN_SIZE), delete_counter(0) {
        table = new shared_ptr<Node<T>>[max_size];
        for (int i = 0; i < max_size; ++i) {
            table[i] = make_shared<Node<T>>();
        }
    }

    ~Hash() {
        delete[] table;
        table = nullptr;
    }

};

template<class T>
StatusType Hash<T>::resizeTable() {
    // check if we need to enlarge or shrink the table
    if (insert_counter > LOAD_FACTOR * max_size) {
        return enlargeTable();
    } else if (curr_size < max_size * SHRINK_THRESHOLD && max_size > MIN_SIZE) {
        return shrinkTable();
    }
    return StatusType::SUCCESS; // no resizing needed
}

template<class T>
StatusType Hash<T>::enlargeTable() {
    shared_ptr<Node<T>> *newTable = new shared_ptr<Node<T>>[TABLE_OF_PRIMES[prime_index + 1]];
    if (!newTable) {
        return StatusType::ALLOCATION_ERROR; // memory allocation failed
    }
    else{
        // initialize new table
        for (int i = 0; i < TABLE_OF_PRIMES[prime_index + 1]; i++) {
            newTable[i] = make_shared<Node<T>>();
        }
        int newInsertCounter = 0; // reset the insert counter for the new table
        //pass on old table contents
        for(int i = 0; i < max_size; i++){
            if(table[i]->data && !table[i]->is_deleted) {
                /// get hashed index of the data to add to the new table
                hashingResult<T> result = hash_insert(*(table[i]->data));
                /// check success of insertion before using the index
                if (result.status == StatusType::FAILURE) {
                    delete[] newTable; // free memory on failure
                    return StatusType::ALLOCATION_ERROR; // memory allocation failed
                } else{
                    // insert the data into the new table
                    newTable[result.index]->data = table[i]->data; // move the data to the new table
                    newTable[result.index]->is_deleted = false; // mark as "not deleted"
                    newInsertCounter++; // increment the insert counter for the new table
                }
            }
        }
        delete[] table;
        this->table = newTable; // update the table pointer
        prime_index = prime_index + 1; // update the prime index
        max_size = TABLE_OF_PRIMES[prime_index]; // update the max size
        curr_size = newInsertCounter; // update the current size
        insert_counter = newInsertCounter; // update the insert counter
        return StatusType::SUCCESS;
    }
}

template<class T>
StatusType Hash<T>::shrinkTable() {
    // check if we can shrink the table
    if(prime_index-1 < 0) {
        return StatusType::FAILURE; // cannot shrink below MIN_SIZE
    }
    shared_ptr<Node<T>> *newTable = new shared_ptr<Node<T>>[TABLE_OF_PRIMES[prime_index - 1]];
    if (!newTable) {
        return StatusType::ALLOCATION_ERROR; // memory allocation failed
    }
    else{
        // initialize new table
        for (int i = 0; i < TABLE_OF_PRIMES[prime_index - 1]; i++) {
            newTable[i] = make_shared<Node<T>>();
        }
        int newInsertCounter = 0; // reset the insert counter for the new table
        //pass on old table contents
        for(int i = 0; i < max_size; i++){
            if(table[i]->data && !table[i]->is_deleted) {
                /// get hashed index of the data to add to the new table
                hashingResult<T> result = hash_insert(*(table[i]->data));
                /// check success of insertion before using the index
                if (result.status == StatusType::FAILURE) {
                    delete[] newTable; // free memory on failure
                    return StatusType::ALLOCATION_ERROR; // memory allocation failed
                } else{
                    // insert the data into the new table
                    newTable[result.index]->data = table[i]->data; // move the data to the new table
                    newTable[result.index]->is_deleted = false; // mark as "not deleted"
                    newInsertCounter++; // increment the insert counter for the new table
                }
            }
        }
        delete[] table;
        this->table = newTable; // update the table pointer
        prime_index = prime_index - 1; // update the prime index
        max_size = TABLE_OF_PRIMES[prime_index]; // update the max size
        curr_size = newInsertCounter; // update the current size
        insert_counter = newInsertCounter; // update the insert counter
        return StatusType::SUCCESS;
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
    int k = 0;
    while (1) {
        int index =
                (key % max_size + k * (1 + (key % (max_size - 5)))) % max_size;
        if (!table[index]->data || table[index]->is_deleted) {
            // found an empty slot or a deleted slot
            return {index, StatusType::SUCCESS};
        }
        k++;
    }


}

template<class T>
hashingResult<T> Hash<T>::hash_search(const T &data) {
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
    int k = 0;
    while (1) {
        int index =
                (key % max_size + k * (1 + (key % (max_size - 5)))) % max_size;

        // Found empty slot (not deleted) - element doesn't exist
        if (!table[index]->data && !table[index]->is_deleted) {
            return {-1, StatusType::FAILURE};
        }

        // Found our element and it's not deleted
        if (table[index]->data && *(table[index]->data) == data &&
            !table[index]->is_deleted) {
            return {index, StatusType::SUCCESS};
        }

        k++;
    }
}

template<class T>
shared_ptr<T> Hash<T>::member(int key) const {
    if(key < 0 || key >= max_size) {
        return nullptr; // invalid key
    }
    hashingResult<T> result = hash_search(key);
    if(result.status == StatusType::FAILURE) {
        return nullptr; // element not found
    } else {
        return table[result.index]->data; // return the data
    }
}

template<class T>
StatusType Hash<T>::remove(const T &data){
    // check if we need to resize the table
    StatusType resizeStatus = resizeTable();
    if(resizeStatus != StatusType::SUCCESS) {
        return resizeStatus; // return the status of resizing
    }
    if(!data) {
        return StatusType::FAILURE; // invalid data
    } else{
        hashingResult<T> result = hash_search(data);
        if(result.status == StatusType::FAILURE) {
            return StatusType::FAILURE;
        }
        else{
            int index = result.index;
            table[index]->is_deleted = true;// mark as deleted
            curr_size--;
            return StatusType::SUCCESS;
        }
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
    if(result.status != StatusType::FAILURE){
        int index = result.index;
        table[index]->is_deleted = false; // mark as not deleted
        table[index]->data = make_shared<T>(data); // store the data
        insert_counter++;
        curr_size++;
        return StatusType::SUCCESS;
    }
    return StatusType::FAILURE;
}

// Helper method to extract hash key from data
template<class T>
int Hash<T>::getHashKey(const T &data) const {
    // Use static_assert to ensure T can be converted to int at compile time
    static_assert(
            std::is_convertible<T, int>::value || std::is_integral<T>::value,
            "Type T must be convertible to int for hashing");

    return static_cast<int>(data);
}
template<class T>
StatusType Hash<T>::rehash(){
    // Create a new table of same size
    shared_ptr<Node<T>> *new_table = new shared_ptr<Node<T>>[max_size];
    if(!new_table) {
        return StatusType::ALLOCATION_ERROR; // memory allocation failed
    }
    for (int i = 0; i < max_size; ++i) {
        new_table[i] = make_shared<Node<T>>();
    }
    delete_counter = 0;
    insert_counter = 0;
    curr_size = 0; // reset counters
    shared_ptr<Node<T>> *old_table = table;
    table = new_table; // update the table pointer

    for(int i = 0; i < max_size; i++) {
        if(old_table[i]->data && !old_table[i]->is_deleted) {
            StatusType status = insert(*(old_table[i]->data));
            if (status != StatusType::SUCCESS) {
                delete[] old_table; // free memory on failure
                return status; // return the status of insertion
            }
        }
    }
    delete[] old_table;
    return StatusType::SUCCESS; // rehash successful

}