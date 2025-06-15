//
// Created by ofek5 on 2025-06-15.
//

#ifndef DS_WET2_LINKEDLIST_H
#define DS_WET2_LINKEDLIST_H

#include <memory>
using namespace std;

template<typename T>
class ListNode {
private:
    shared_ptr<T>  data_;
    shared_ptr<ListNode<T>> next_;
    int                  id_;

public:
    explicit ListNode(const shared_ptr<T>& data, int id)
            : data_(data), next_(nullptr), id_(id)
    {}

    void setNext(const shared_ptr<ListNode<T>>& next) {
        next_ = next;
    }

    shared_ptr<ListNode<T>> getNext() const {
        return next_;
    }

    int getID() const {
        return id_;
    }

    shared_ptr<T> getData() const {
        return data_;
    }
};


template<typename T>
class LinkedList {
private:
    shared_ptr<ListNode<T>> head_;
    int                           size_;

public:
    LinkedList()
            : head_(nullptr), size_(0)
    {}

    void decreaseSize() {
        --size_;
    }

    shared_ptr<ListNode<T>> getStart() const {
        return head_;
    }

    void setStart(const shared_ptr<ListNode<T>>& node) {
        head_ = node;
    }

    int getSize() const {
        return size_;
    }

    void insert(const shared_ptr<T>& item, int id) {
        auto node = std::make_shared<ListNode<T>>(item, id);
        node->setNext(head_);
        head_ = node;
        ++size_;
    }

    bool search(int id) const {
        auto curr = head_;
        while (curr) {
            if (curr->getID() == id)
                return true;
            curr = curr->getNext();
        }
        return false;
    }

    shared_ptr<T> removeFirst() {
        if (!head_)
            return nullptr;
        auto node = head_;
        head_ = head_->getNext();
        node->setNext(nullptr);
        --size_;
        return node->getData();
    }

    shared_ptr<T> getValue(int id) const {
        auto curr = head_;
        while (curr) {
            if (curr->getID() == id)
                return curr->getData();
            curr = curr->getNext();
        }
        return nullptr;
    }

    bool remove(int id) {
        if (!head_)
            return false;
        auto curr = head_;
        shared_ptr<ListNode<T>> prev = nullptr;
        while (curr && curr->getID() != id) {
            prev = curr;
            curr = curr->getNext();
        }
        if (!curr)
            return false;
        if (prev)
            prev->setNext(curr->getNext());
        else
            head_ = curr->getNext();
        --size_;
        return true;
    }

    int countInstances(int id) const {
        int cnt = 0;
        auto curr = head_;
        while (curr) {
            if (curr->getID() == id)
                ++cnt;
            curr = curr->getNext();
        }
        return cnt;
    }
};

#endif // DS_WET2_LINKEDLIST_H
