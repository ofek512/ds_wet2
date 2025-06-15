#ifndef HASHMAP_H
#define HASHMAP_H

#include <memory>
#include "wet2util.h"
#include "LinkedList.h"

template<typename Item>
class HashMap {
public:
    explicit HashMap();

    bool add(int key, std::shared_ptr<Item> value);
    bool erase(int key);
    std::shared_ptr<Item> find(int key) const;
    int count(int key) const;

    int getCapacity() const { return capacity_; }

    // now returns the raw pointer to the list, not the unique_ptr wrapper
    LinkedList<Item>* getBucket(int index) const {
        if (index >= 0 && index < capacity_) {
            return buckets_[index].get();
        }
        return nullptr;
    }

private:
    void expand();
    int hashIndex(int key, int bucket_count) const {
        return ((key % bucket_count) + bucket_count) % bucket_count;
    }

    int capacity_;
    int size_;
    std::unique_ptr<std::unique_ptr<LinkedList<Item>>[]> buckets_;
};

template<typename Item>
HashMap<Item>::HashMap()
        : capacity_(16),
          size_(0),
          buckets_(new std::unique_ptr<LinkedList<Item>>[capacity_])
{
    for (int i = 0; i < capacity_; ++i) {
        buckets_[i] = std::make_unique<LinkedList<Item>>();
    }
}

template<typename Item>
void HashMap<Item>::expand() {
    int new_capacity = capacity_ * 2;
    auto new_buckets = std::unique_ptr<std::unique_ptr<LinkedList<Item>>[]>(
            new std::unique_ptr<LinkedList<Item>>[new_capacity]
    );
    for (int i = 0; i < new_capacity; ++i) {
        new_buckets[i] = std::make_unique<LinkedList<Item>>();
    }
    for (int i = 0; i < capacity_; ++i) {
        int list_size = buckets_[i]->getSize();
        for (int j = 0; j < list_size; ++j) {
            auto node_val = buckets_[i]->removeFirst();
            int idx = hashIndex(node_val->getID(), new_capacity);
            new_buckets[idx]->insert(node_val, node_val->getID());
        }
    }
    capacity_ = new_capacity;
    buckets_ = std::move(new_buckets);
}

template<typename Item>
bool HashMap<Item>::add(int key, std::shared_ptr<Item> value) {
    ++size_;
    if (size_ > capacity_ * 2) {
        expand();
    }
    int idx = hashIndex(key, capacity_);
    buckets_[idx]->insert(value, key);
    return true;
}

template<typename Item>
bool HashMap<Item>::erase(int key) {
    int idx = hashIndex(key, capacity_);
    if (buckets_[idx]->search(key)) {
        buckets_[idx]->remove(key);
        --size_;
        return true;
    }
    return false;
}

template<typename Item>
std::shared_ptr<Item> HashMap<Item>::find(int key) const {
    int idx = hashIndex(key, capacity_);
    return buckets_[idx]->getValue(key);
}

template<typename Item>
int HashMap<Item>::count(int key) const {
    int idx = hashIndex(key, capacity_);
    return buckets_[idx]->countInstances(key);
}

#endif // HASHMAP_H
