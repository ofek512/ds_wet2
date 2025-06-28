#ifndef HASH_H
#define HASH_H

#include <memory>
#include "wet2util.h"
#include "LinkedList.h"

using namespace std;

// Constants for load factor thresholds
#define LOAD_FACTOR 0.7
#define SHRINK_THRESHOLD 0.3

// Table of prime sizes for resizing
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

// Number of primes in the above table
constexpr int PRIMES_COUNT =
        static_cast<int>(sizeof(TABLE_OF_PRIMES) / sizeof(TABLE_OF_PRIMES[0]));

template<class T>
class Hash {
private:
    int bucket_count;           // current number of non-empty buckets
    int element_count;          // total number of elements stored
    int prime_index;            // index into TABLE_OF_PRIMES for current capacity
    int max_bucket_count;       // current capacity (number of buckets)
    unique_ptr<unique_ptr<LinkedList<T>>[]> table;

    // Resizing helpers
    bool resizeTable();
    bool dynamicSize(int newSize);

    // Hash function
    int hashFunc(int key, int bucketCount) const;

public:
    Hash()
            : bucket_count(0),
              element_count(0),
              prime_index(0),
              max_bucket_count(TABLE_OF_PRIMES[0])
    {
        table = make_unique<unique_ptr<LinkedList<T>>[]>(max_bucket_count);
        for (int i = 0; i < max_bucket_count; ++i)
            table[i].reset();
    }

    ~Hash() = default;

    bool insert(int id, shared_ptr<T> data);
    shared_ptr<T> search(int id) const;
    bool remove(int id);

    int getCurrentBucketCount() const { return bucket_count; }
    int getMaxSize() const { return max_bucket_count; }
    LinkedList<T>* getBucket(int index) const;
};

// ===== Implementation =====

template<class T>
bool Hash<T>::resizeTable() {
    // Grow?
    if (element_count > LOAD_FACTOR * max_bucket_count) {
        if (prime_index + 1 < PRIMES_COUNT) {
            ++prime_index;
            return dynamicSize(TABLE_OF_PRIMES[prime_index]);
        }
        return true;
    }
    // Shrink?
    if (element_count < SHRINK_THRESHOLD * max_bucket_count) {
        if (prime_index > 0) {
            --prime_index;
            return dynamicSize(TABLE_OF_PRIMES[prime_index]);
        }
        return true;
    }
    // No change needed
    return true;
}

template<class T>
bool Hash<T>::dynamicSize(int newSize) {
    // Allocate new table
    auto newTable = make_unique<unique_ptr<LinkedList<T>>[]>(newSize);
    for (int i = 0; i < newSize; ++i)
        newTable[i] = make_unique<LinkedList<T>>();

    // Rehash old elements
    auto oldTable = move(table);
    int oldSize = max_bucket_count;
    table = move(newTable);
    bucket_count = 0;
    element_count = 0;

    for (int i = 0; i < oldSize; ++i) {
        if (oldTable[i]) {
            auto curr = oldTable[i]->getStart();
            while (curr) {
                auto nodeData = curr->getData();
                int nodeId = curr->getID();
                if (nodeData) {
                    int idx = hashFunc(nodeId, newSize);
                    if (!table[idx]) {
                        table[idx] = make_unique<LinkedList<T>>();
                        ++bucket_count;
                    }
                    table[idx]->insert(nodeData, nodeId);
                    ++element_count;
                }
                curr = curr->getNext();
            }
        }
    }

    max_bucket_count = newSize;
    return true;
}

template<class T>
bool Hash<T>::insert(int id, shared_ptr<T> data) {
    // Reserve space first
    ++element_count;
    resizeTable();

    int idx = hashFunc(id, max_bucket_count);
    if (!table[idx]) {
        table[idx] = make_unique<LinkedList<T>>();
        ++bucket_count;
    }
    table[idx]->insert(data, id);
    return true;
}

template<class T>
shared_ptr<T> Hash<T>::search(int id) const {
    int idx = hashFunc(id, max_bucket_count);
    if (!table[idx]) return nullptr;
    return table[idx]->getValue(id);
}

template<class T>
bool Hash<T>::remove(int id) {
    int idx = hashFunc(id, max_bucket_count);
    if (!table[idx]) return false;

    if (table[idx]->search(id)) {
        table[idx]->remove(id);
        --element_count;
        resizeTable();
        if (table[idx]->getSize() == 0)
            --bucket_count;
        return true;
    }
    return false;
}

template<class T>
LinkedList<T>* Hash<T>::getBucket(int index) const {
    if (index >= 0 && index < max_bucket_count)
        return table[index].get();
    return nullptr;
}

template<class T>
int Hash<T>::hashFunc(int key, int bucketCount) const {
    return (bucketCount > 0 ? (key % bucketCount) : 0);
}

#endif // HASH_H