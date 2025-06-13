
#ifndef HASH_H
#define HASH_H

#include <memory>
#include <type_traits>
#include "wet2util.h"

using namespace std;

// Constants for hash table management
#define MIN_SIZE 13
#define DEFAULT_LOAD_FACTOR 0.7
#define REHASH_THRESHOLD 0.5
#define SHRINK_THRESHOLD 0.25

template<class T>
struct Node {
    shared_ptr<T> data;
    bool is_deleted;

    // Constructor - default, data will be null
    Node() : data(nullptr), is_deleted(false) {}
};

template<class T>
struct hashingResult {
    int index;
    StatusType status;
};

// Table of prime numbers used for hash table sizing
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
    int curr_size;      // Current number of active elements
    int insert_counter; // Counts the number of insertions
    int prime_index;    // Saves the current index in TABLE_OF_PRIMES
    int max_size;       // Current table capacity
    int delete_counter; // Count of deleted elements
    shared_ptr<Node<T>> *table;

    // Helper methods for table management
    StatusType enlargeTable();
    StatusType shrinkTable();
    StatusType rehash();

    // Core hash operations
    hashingResult<T> hash_insert(const T &data);
    hashingResult<T> hash_search(const T &data);

    // Helper to get hash value from data
    int getHashKey(const T &data) const;

public:
    // Constructor
    Hash() : curr_size(0), insert_counter(0), prime_index(0),
             max_size(MIN_SIZE), delete_counter(0) {
        table = new shared_ptr<Node<T>>[max_size];
        for (int i = 0; i < max_size; ++i) {
            table[i] = make_shared<Node<T>>();
        }
    }

    // Destructor
    ~Hash() {
        delete[] table;
    }

    // Public operations
    StatusType insert(const T &data);
    StatusType remove(const T &data);
    shared_ptr<T> member(int key) const;

    // Copy constructor and assignment operator deleted for proper RAII
    Hash(const Hash &other) = delete;
    Hash &operator=(const Hash &other) = delete;
};

// Helper method to extract hash key from data
template<class T>
int Hash<T>::getHashKey(const T &data) const {
    // Re-enable type safety check
    static_assert(std::is_convertible<decltype(*std::declval<T>()), int>::value, "Type T must be dereferenceable to a type convertible to int");

    // Safety check before dereferencing
    if (!data) {
        throw std::invalid_argument("Cannot hash null data");
    }

    return static_cast<int>(*data);
}

template<class T>
hashingResult<T> Hash<T>::hash_insert(const T &data) {
    // Check valid input
    if (!data) {
        return {-1, StatusType::FAILURE};
    }

    int key = getHashKey(data);
    int k = 0;
    int attempts = 0;

    // Double hashing algorithm:
    // h(x) = x % m
    // r(x) = 1 + (x % (m - 5)) - ensures r(x) and m are relatively prime
    // hk(x) = (h(x) + k * r(x)) % m
    while (attempts < max_size) {
        int index =
                (key % max_size + k * (1 + (key % (max_size - 5)))) % max_size;

        if (!table[index]->data || table[index]->is_deleted) {
            // Found an empty or deleted slot
            return {index, StatusType::SUCCESS};
        }

        // Check if element already exists and is not deleted
        if (table[index]->data && *(table[index]->data) == data &&
            !table[index]->is_deleted) {
            return {-1, StatusType::FAILURE}; // Element already exists
        }

        attempts++;
        k++;
    }

    return {-1, StatusType::FAILURE}; // Table is full
}

template<class T>
hashingResult<T> Hash<T>::hash_search(const T &data) {
    // Check valid input
    if (!data) {
        return {-1, StatusType::FAILURE};
    }

    int key = getHashKey(data);
    int k = 0;
    int attempts = 0;

    while (attempts < max_size) {
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
        attempts++;
    }

    // If we've checked the entire table and found nothing
    return {-1, StatusType::FAILURE};
}

template<class T>
StatusType Hash<T>::insert(const T &data) {
    // Check if we need to enlarge the table
    if (static_cast<double>(curr_size + delete_counter) / max_size >=
        DEFAULT_LOAD_FACTOR) {
        StatusType status = enlargeTable();
        if (status != StatusType::SUCCESS) {
            return status;
        }
    }

    // Get the index to insert the data
    hashingResult<T> result = hash_insert(data);
    if (result.status != StatusType::SUCCESS) {
        return result.status;
    }

    // Insert the data
    if (!table[result.index]->data) {
        // If the slot is empty, insert normally
        table[result.index]->data = make_shared<T>(data);
        table[result.index]->is_deleted = false;
        curr_size++;
        insert_counter++;
    } else {
        // Reuse the existing (deleted) slot
        table[result.index]->data = make_shared<T>(data);
        table[result.index]->is_deleted = false;
        delete_counter--; // Decrement delete counter if we reused a deleted slot
    }

    return StatusType::SUCCESS;
}

template<class T>
StatusType Hash<T>::remove(const T &data) {
    // Check if we need to rehash the table
    if (static_cast<double>(delete_counter) / max_size >= REHASH_THRESHOLD) {
        rehash();
    }

    // Check if we need to shrink the table
    if (max_size > MIN_SIZE &&
        static_cast<double>(curr_size) <= max_size * SHRINK_THRESHOLD) {
        StatusType status = shrinkTable();
        if (status != StatusType::SUCCESS) {
            return status;
        }
    }

    hashingResult<T> result = hash_search(data);
    if (result.status != StatusType::SUCCESS) {
        return result.status; // Element not found
    }

    table[result.index]->is_deleted = true; // Mark as deleted
    delete_counter++;
    curr_size--;

    return StatusType::SUCCESS;
}

template<class T>
shared_ptr<T> Hash<T>::member(int key) const {
    // Validate key
    if (key <= 0) {
        return nullptr;
    }

    int k = 0;
    int attempts = 0;

    while (attempts < max_size) {
        int index =
                (key % max_size + k * (1 + (key % (max_size - 5)))) % max_size;

        // Skip empty and deleted slots
        if (!table[index]->data || table[index]->is_deleted) {
            k++;
            attempts++;
            continue;
        }

        if (table[index]->data) {
            int dataKey = getHashKey(*(table[index]->data));
            if (dataKey == key) {
                return table[index]->data;
            }
        }

        k++;
        attempts++;
    }

    return nullptr; // Not found
}

template<class T>
StatusType Hash<T>::enlargeTable() {
   if (prime_index >= static_cast<int>(sizeof(TABLE_OF_PRIMES) / sizeof(int) - 1)) {
        return StatusType::FAILURE; // No more primes available
    }

    prime_index++; // Update prime index before using it
    int new_max_size = TABLE_OF_PRIMES[prime_index];

    // Create a new table
    shared_ptr<Node<T>> *new_table = new shared_ptr<Node<T>>[new_max_size];
    for (int i = 0; i < new_max_size; ++i) {
        new_table[i] = make_shared<Node<T>>();
    }

    // Save old table
    shared_ptr<Node<T>> *old_table = table;
    int old_max_size = max_size;

    // Update the table and max size
    table = new_table;
    max_size = new_max_size;

    // Reset counters for rehashing
    curr_size = 0;
    delete_counter = 0;
    insert_counter = 0;

    // Rehash the old table
    for (int i = 0; i < old_max_size; i++) {
        if (old_table[i]->data && !old_table[i]->is_deleted) {
            insert(*(old_table[i]->data));
        }
    }

    delete[] old_table;
    return StatusType::SUCCESS;
}

template<class T>
StatusType Hash<T>::shrinkTable() {
    if (max_size <= MIN_SIZE || prime_index <= 0) {
        return StatusType::FAILURE; // Cannot shrink below MIN_SIZE
    }

    prime_index--; // Update prime index before using it
    int new_max_size = TABLE_OF_PRIMES[prime_index];

    // Create a new table
    shared_ptr<Node<T>> *new_table = new shared_ptr<Node<T>>[new_max_size];
    for (int i = 0; i < new_max_size; ++i) {
        new_table[i] = make_shared<Node<T>>();
    }

    // Save old table
    shared_ptr<Node<T>> *old_table = table;
    int old_max_size = max_size;

    // Update table and parameters
    table = new_table;
    max_size = new_max_size;

    // Reset counters for rehashing
    curr_size = 0;
    delete_counter = 0;
    insert_counter = 0;

    // Rehash the old table
    for (int i = 0; i < old_max_size; i++) {
        if (old_table[i]->data && !old_table[i]->is_deleted) {
            insert(*(old_table[i]->data));
        }
    }

    delete[] old_table;
    return StatusType::SUCCESS;
}

template<class T>
StatusType Hash<T>::rehash() {
    // Create a new table of same size
    shared_ptr<Node<T>> *new_table = new shared_ptr<Node<T>>[max_size];
    for (int i = 0; i < max_size; ++i) {
        new_table[i] = make_shared<Node<T>>();
    }

    // Save old table
    shared_ptr<Node<T>> *old_table = table;

    // Update the table
    table = new_table;

    // Reset counters for rehashing
    //int old_curr_size = curr_size;
    curr_size = 0;
    delete_counter = 0;
    insert_counter = 0;

    // Rehash only the active elements
    for (int i = 0; i < max_size; ++i) {
        if (old_table[i]->data && !old_table[i]->is_deleted) {
            insert(*(old_table[i]->data));
        }
    }

    delete[] old_table;
    return StatusType::SUCCESS;
}

#endif // HASH_H
