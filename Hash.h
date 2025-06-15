#ifndef HASH_H
#define HASH_H

#include <memory>
#include <type_traits>
#include "wet2util.h"

using namespace std;

// Constants for hash table management
#define MIN_SIZE 11
#define DEFAULT_LOAD_FACTOR 0.7
#define REHASH_THRESHOLD 0.5
#define SHRINK_THRESHOLD 0.25

template<class T>
struct Node {
    T data;
    bool is_deleted;

    // Constructor - default, data will be null
    Node() : data(nullptr), is_deleted(false) {}
};

template<class T>
struct hashingResult {
    long index;
    StatusType status;
};

// Table of prime numbers used for hash table sizing
const int TABLE_OF_PRIMES[] = {
        11, 23, 47, 97, 197, 397, 797, 1601, 3203, 6421, 12853, 25717, 51437,
        102877, 205759, 411527, 823117, 1646237, 3292489, 6584983, 13169977,
        26339969, 52679969, 105359939, 210719881, 421439783, 842879579, 1685759167
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

    // Helper to get hash value from data - now returns long for larger numbers
    inline long getHashKey(const T &data) const;

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
    StatusType insert(const T data);
    StatusType remove(const T data);
    T member(int key) const;

    // Copy constructor and assignment operator deleted for proper RAII
    Hash(const Hash &other) = delete;
    Hash &operator=(const Hash &other) = delete;
};

// Inline helper method to extract hash key from data for better performance
// Now returns long to handle large numbers
template<class T>
inline long Hash<T>::getHashKey(const T &data) const {
    // Type safety check
    static_assert(std::is_convertible<decltype(*std::declval<T>()), long>::value,
                  "Type T must be dereferenceable to a type convertible to long");

    // Safety check before dereferencing
    if (!data) {
        throw std::invalid_argument("Cannot hash null data");
    }

    return static_cast<long>(*data);
}

template<class T>
hashingResult<T> Hash<T>::hash_insert(const T &data) {
    // Check valid input
    if (!data) {
        return {-1, StatusType::FAILURE};
    }

    const long key = getHashKey(data);
    // Precompute hash values outside the loop - using long for large numbers
    const long h1 = key % max_size;
    long h2 = 1 + (key % (max_size - 5)); // Ensure h2 is positive
    if (h2 <= 0) h2 = 1;

    long k = 0;

    // More efficient loop
    do {
        const long index = (h1 + k * h2) % max_size;

        // Skip deleted or empty slots
        if (!table[index]->data || table[index]->is_deleted) {
            return {index, StatusType::SUCCESS};
        }

        // Check if element already exists and is not deleted
        if (table[index]->data && (*table[index]->data) == (*data) && !table[index]->is_deleted) {
            return {-1, StatusType::FAILURE}; // Element already exists
        }

        k++;
    } while (k < max_size);

    return {-1, StatusType::FAILURE}; // Table is full
}

template<class T>
hashingResult<T> Hash<T>::hash_search(const T &data) {
    // Check valid input
    if (!data) {
        return {-1, StatusType::FAILURE};
    }

    const long key = getHashKey(data);
    // Precompute hash values outside the loop - using long for large numbers
    const long h1 = key % max_size;
    long h2 = 1 + (key % (max_size - 5)); // Ensure h2 is positive
    if (h2 <= 0) h2 = 1;

    long k = 0;

    // More efficient loop
    do {
        const long index = (h1 + k * h2) % max_size;

        // Found empty slot (not deleted) - element doesn't exist
        if (!table[index]->data && !table[index]->is_deleted) {
            return {-1, StatusType::FAILURE};
        }

        // Found our element and it's not deleted
        if (table[index]->data && (*table[index]->data) == (*data) && !table[index]->is_deleted) {
            return {index, StatusType::SUCCESS};
        }

        k++;
    } while (k < max_size);

    // If we've checked the entire table and found nothing
    return {-1, StatusType::FAILURE};
}

template<class T>
StatusType Hash<T>::insert(const T data) {
    // Check if we need to enlarge the table
    if (static_cast<double>(curr_size + delete_counter) / max_size >= DEFAULT_LOAD_FACTOR) {
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
        table[result.index]->data = data;
        table[result.index]->is_deleted = false;
        curr_size++;
        insert_counter++;
    } else {
        // Reuse the existing (deleted) slot
        table[result.index]->data = data;
        table[result.index]->is_deleted = false;
        delete_counter--; // Decrement delete counter if we reused a deleted slot
    }

    return StatusType::SUCCESS;
}

template<class T>
StatusType Hash<T>::remove(const T data) {
    // Check if we need to rehash the table
    if (static_cast<double>(delete_counter) / max_size >= REHASH_THRESHOLD) {
        rehash();
    }

    // Check if we need to shrink the table
    if (max_size > MIN_SIZE && static_cast<double>(curr_size) <= max_size * SHRINK_THRESHOLD) {
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
T Hash<T>::member(int key) const {
    // Validate key
    if (key <= 0) {
        return nullptr;
    }

    // Convert to long for consistency with other hash operations
    const long long_key = static_cast<long>(key);

    // Precompute hash values outside the loop - using long for large numbers
    const long h1 = long_key % max_size;
    long h2 = 1 + (long_key % (max_size - 5)); // Ensure h2 is positive
    if (h2 <= 0) h2 = 1;

    long k = 0;

    // More efficient loop
    do {
        const long index = (h1 + k * h2) % max_size;

        // Skip empty slots
        if (!table[index]) {
            return nullptr;
        }

        // Skip deleted slots and empty data
        if (!table[index]->data || table[index]->is_deleted) {
            k++;
            continue;
        }

        // Check if this is our key
        if (getHashKey(table[index]->data) == long_key) {
            return table[index]->data;
        }

        k++;
    } while (k < max_size);

    return nullptr; // Not found
}

template<class T>
StatusType Hash<T>::enlargeTable() {
    if (prime_index >= static_cast<int>(sizeof(TABLE_OF_PRIMES) / sizeof(int)) - 1) {
        return StatusType::FAILURE; // No more primes available
    }

    // Pre-increment prime_index to avoid another increment operation
    const int new_prime_index = prime_index + 1;
    const int new_max_size = TABLE_OF_PRIMES[new_prime_index];

    // Create a new table with pre-allocated capacity
    shared_ptr<Node<T>> *new_table = new shared_ptr<Node<T>>[new_max_size];
    for (int i = 0; i < new_max_size; ++i) {
        new_table[i] = make_shared<Node<T>>();
    }

    // Save old table
    shared_ptr<Node<T>> *old_table = table;
    const int old_max_size = max_size;

    // Update parameters before rehashing
    table = new_table;
    max_size = new_max_size;
    prime_index = new_prime_index;

    // Reset counters for rehashing
    curr_size = 0;
    delete_counter = 0;
    insert_counter = 0;

    // Rehash only active elements
    for (int i = 0; i < old_max_size; i++) {
        if (old_table[i]->data && !old_table[i]->is_deleted) {
            insert(old_table[i]->data);
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

    // Pre-decrement prime_index to avoid another decrement operation
    const int new_prime_index = prime_index - 1;
    const int new_max_size = TABLE_OF_PRIMES[new_prime_index];

    // Create a new table with pre-allocated capacity
    shared_ptr<Node<T>> *new_table = new shared_ptr<Node<T>>[new_max_size];
    for (int i = 0; i < new_max_size; ++i) {
        new_table[i] = make_shared<Node<T>>();
    }

    // Save old table
    shared_ptr<Node<T>> *old_table = table;
    const int old_max_size = max_size;

    // Update parameters before rehashing
    table = new_table;
    max_size = new_max_size;
    prime_index = new_prime_index;

    // Reset counters for rehashing
    curr_size = 0;
    delete_counter = 0;
    insert_counter = 0;

    // Rehash only active elements
    for (int i = 0; i < old_max_size; i++) {
        if (old_table[i]->data && !old_table[i]->is_deleted) {
            insert(old_table[i]->data);
        }
    }

    delete[] old_table;
    return StatusType::SUCCESS;
}

template<class T>
StatusType Hash<T>::rehash() {
    // Create a new table of same size with pre-allocated capacity
    shared_ptr<Node<T>> *new_table = new shared_ptr<Node<T>>[max_size];
    for (int i = 0; i < max_size; ++i) {
        new_table[i] = make_shared<Node<T>>();
    }

    // Save old table
    shared_ptr<Node<T>> *old_table = table;

    // Update the table
    table = new_table;

    // Reset counters for rehashing
    curr_size = 0;
    delete_counter = 0;
    insert_counter = 0;

    // Rehash only active elements
    for (int i = 0; i < max_size; ++i) {
        if (old_table[i]->data && !old_table[i]->is_deleted) {
            insert(old_table[i]->data);
        }
    }

    delete[] old_table;
    return StatusType::SUCCESS;
}

#endif // HASH_H