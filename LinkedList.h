//
// Created by ofek5 on 2025-06-15.
//

#ifndef DS_WET2_LINKEDLIST_H
#define DS_WET2_LINKEDLIST_H
#include <memory>

template <class T>
class Node{
private:
    std::shared_ptr<T> data = nullptr;
    std::shared_ptr<Node<T>> next = nullptr;
    int ID;

public:
    Node<T>(std::shared_ptr<T> data, int ID) : data(data), next(nullptr), ID(ID)
    {}

    void setNext(std::shared_ptr<Node<T>> nextNode){
        next = nextNode;
    }

    std::shared_ptr<Node<T>> getNext(){
        return next;
    }

    int getID(){
        return ID;
    }

    std::shared_ptr<T> getData(){
        return data;
    }

};


template <class T>
class LinkedList {
private:
    std::shared_ptr<Node<T>> start = nullptr;
    int length = 0;

public:

    LinkedList() = default;

    void decreaseSize() {
        length--;
    }

    std::shared_ptr<Node<T>> getStart(){
        return start;
    }

    void setStart(std::shared_ptr<Node<T>> newStart){
        start = newStart;
    }

    int getSize() {
        return length;
    }

    void insert(std::shared_ptr<T> newData, int newID){
        std::shared_ptr<Node<T>> newNode =
                std::make_shared<Node<T>>(newData, newID);
        newNode->setNext(start);
        start = newNode;
        length++;
    }

    bool search(int ID){
        if(start == nullptr){
            return false;
        }
        std::shared_ptr<Node<T>> currentNode = start;
        while(ID != currentNode->getID()){
            currentNode = currentNode->getNext();
            if(currentNode == nullptr){
                return false;
            }
        }
        return true;
    }

    std::shared_ptr<T> removeFirst(){
        if(start == nullptr){
            return nullptr;
        }
        auto oldStart = start;
        start = start->getNext();
        oldStart->setNext(nullptr);
        length--;
        return oldStart->getData();
    }

    std::shared_ptr<T> getValue(int searchID) {
        auto current = start;
        while(current != nullptr) {
            if (current->getID() == searchID) {
                return current->getData();
            }
            current = current->getNext();
        }
        return nullptr;
    }

    bool remove(int id) { //TODO check this function
        if(start == nullptr){
            return false;
        }
        std::shared_ptr<Node<T>> currentNode = start, previousNode = start;
        if(currentNode == nullptr){
            return false;
        }
        while(id != currentNode->getID()){
            previousNode = currentNode;
            currentNode = currentNode->getNext();
            if(currentNode == nullptr){
                return false;
            }
        }
        if(previousNode != currentNode){
            previousNode->setNext(currentNode->getNext());

        } else{
            start = currentNode->getNext();
        }
        length--;
        return true;
    }

    int countInstances(int num) {
        int sum = 0;
        auto current = start;
        while(current != nullptr) {
            if (current->getID() == num) {
                sum++;
            }
            current = current->getNext();
        }
        return sum;
    }
};

#endif //DS_WET2_LINKEDLIST_H
