#include<memory>
#include "wet2util.h"

using namespace std;

#define MIN_SIZE 13
#define load_factor 0.7

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
    StatusType enlargeTable();
    StatusType shrinkTable();
    StatusType rehash();
    hashingResult<T> hash_insert(const T &data); /// V
    hashingResult<T> hash_search(const T &data); /// V
public:
    StatusType insert(const T &data);
    StatusType remove(const T &data);
    shared_ptr<T> member(int key) const;

    Hash() : curr_size(0), insert_counter(0), prime_index(0),
             max_size(MIN_SIZE), delete_counter(0) {
        table = new shared_ptr<Node<T>>[max_size];
        for (int i = 0; i < max_size; ++i) {
            table[i] = make_shared<Node<T>>();
        }
    }

    ~Hash() {
        delete[] table;
    }

};

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
    int key = data;
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
    int key = data;
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

