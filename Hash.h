#include<memory>
#include "wet2util.h"
#include "LinkedList.h"

using namespace std;

// Constants for load factor thresholds
#define LOAD_FACTOR 0.7
#define SHRINK_THRESHOLD 0.3

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
    int bucket_count; // the current number of buckets (i.e., beginnings of linked lists) in the hash table
    int element_count; // counts the number of occupied nodes
    int prime_index; // saves the current index in TABLE_OF_PRIMES
    int max_bucket_count; // maximum number of elements
    std::unique_ptr<std::unique_ptr<LinkedList<T>>[]> table;

    //START: resizing functions
    bool resizeTable(); /// adjust the table size according to the current load factor
    bool dynamicSize(int newSize); /// enlarge table in the case of load factor > 0.7
    //END: resizing functions

    //hash function
    int hashFunc(int key, int bucketCount) const; /// V

public:
    Hash() : bucket_count(0), element_count(0), prime_index(0), max_bucket_count(TABLE_OF_PRIMES[0]) {
        table = std::make_unique<std::unique_ptr<LinkedList<T>>[]>(max_bucket_count);
        for (int i = 0; i < max_bucket_count; i++) {
            table[i] = unique_ptr<LinkedList<T>>();
        }
    }

    ~Hash() = default;

    //START: element-related functions
    bool insert(int id, std::shared_ptr<T> data);
    shared_ptr<T> search(int id) const;
    bool remove(int id);
    //END: element-related functions
    //START: getters
    int getCurrentBucketCount() const; /// V
    LinkedList<T>* getBucket(int index) const;
    int getMaxSize();
    //END: getters

};




//START: resizing functions
template<class T>
bool Hash<T>::resizeTable() {
    if(element_count > LOAD_FACTOR * max_bucket_count) {
        if(prime_index + 1 < sizeof(TABLE_OF_PRIMES) / sizeof(TABLE_OF_PRIMES[0])) {
            prime_index++;
            dynamicSize(TABLE_OF_PRIMES[prime_index]);

        } else{
            return true;
        }
    } else if (element_count < SHRINK_THRESHOLD * max_bucket_count) {
        if (prime_index > 0) {
            prime_index--; // Cannot shrink below the first prime size
            dynamicSize(TABLE_OF_PRIMES[prime_index]);

        } else{
            return true;
        }
    }
    return true; // No resizing needed
}

template<class T>
bool Hash<T>::dynamicSize(int newSize) {
    auto newTable = std::make_unique<std::unique_ptr<LinkedList<T>>[]>(newSize);
    for (int i = 0; i < newSize; i++) {
        newTable[i] = make_unique<LinkedList<T>>();
    }
    auto oldTable = std::move(table);
    int oldSize = max_bucket_count;
    table = std::move(newTable);
    element_count = 0;
    bucket_count = 0;
    // Rehashing the elements
    for (int i = 0; i < oldSize; i++) {
        if (oldTable[i]) {
            auto curr = oldTable[i]->getStart();
            while (curr) {
                auto nodeData = curr->getData();
                int node_id = curr->getID();
                if(nodeData) {
                    int newIdx = hashFunc(node_id, newSize);
                    table[newIdx]->insert(nodeData, node_id);
                    element_count++;
                }
                curr = curr->getNext();

            }
        }
    }
    max_bucket_count = newSize;
    return true;
}

//END: resizing functions

//START: element-related functions
template<class T>
bool Hash<T>::insert(int id, std::shared_ptr<T> data) {
    // resizing the table if necessary
    element_count++;
    resizeTable();
    //insertion into the table
    int idx = hashFunc(id, max_bucket_count);
    if (!table[idx]) {
        table[idx] = std::make_unique<LinkedList<T>>();
        bucket_count++;
    }
    table[idx]->insert(data, id);
    return true; // Insertion successful
}

template<class T>
shared_ptr<T> Hash<T>::search(int id) const {
    int idx = hashFunc(id, max_bucket_count);
    if (!table[idx]) return nullptr;  // Fix: Avoid dereferencing null
    return table[idx]->getValue(id);
}


template<class T>
bool Hash<T>::remove(int id) {
    int idx = hashFunc(id, max_bucket_count);
    if (!table[idx]) return false;  // Fix: Avoid null
    if (table[idx]->search(id)) {
        table[idx]->remove(id);
        element_count--;
        resizeTable(); // Check if resizing is needed after deletion
        if(!(table[idx]->getSize())){
            bucket_count--;
        }
        return true;
    }
    return false;
}

//END: element-related functions

//START: getters

template<class T>
int Hash<T>::getMaxSize(){
    return max_bucket_count;
}

template<class T>
int Hash<T>::getCurrentBucketCount() const {
    return bucket_count;
}

template<class T>
LinkedList<T>* Hash<T>::getBucket(int index) const {
    if (index >= 0 && index < max_bucket_count) {
        return table[index].get();
    }
    return nullptr;
}


//END: getters

//Hash Function
template<class T>
int Hash<T>::hashFunc(int key, int bucketCount) const {
    return (bucketCount > 0) ? (key % bucketCount) : 0;
}



