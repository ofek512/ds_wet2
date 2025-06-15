//
// Created by nival on 1/20/2025.
//

#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <memory>
#include "LinkedList.h"

#endif //HASHTABLE_H

template <class T>
class HashTable {
public:
    HashTable(bool isRecordTable);
    void resize();
    bool insert(int id, std::shared_ptr<T> val);
    bool remove(int id);
    bool removeByRecord(int id, int record);
    std::shared_ptr<T> search(int id);
    std::shared_ptr<T> searchByRecord(int id, int record);
    int countInstances(int num);



private:
    std::unique_ptr<std::unique_ptr<LinkedList<T>>[]> values;
    int countValues;
    int length;
    int computeHash(const int key, int size) const { return  (key % size + size) % size;}
    bool recordTable = false;
};



template<class T>
HashTable<T>::HashTable(bool isRecordTable) {
    recordTable = isRecordTable;
    countValues = 0;
    length = 16;
    values = std::unique_ptr<std::unique_ptr<LinkedList<T>>[]>(new std::unique_ptr<LinkedList<T>>[length]);
    for (int i = 0; i < length; i++) {
        values[i] = std::unique_ptr<LinkedList<T>>(new LinkedList<T>);
    }
}


template<class T>
void HashTable<T>::resize() {
    int tempLength = length*2;
    std::unique_ptr<std::unique_ptr<LinkedList<T>>[]> newValues = std::unique_ptr<std::unique_ptr<LinkedList<T>>[]>(new std::unique_ptr<LinkedList<T>>[tempLength]);
    for (int i = 0; i < tempLength; i++) {
        newValues[i] = std::unique_ptr<LinkedList<T>>(new LinkedList<T>);
    }
    for (int i = 0; i < length; i++) {
        int listSize = values[i]->getSize();
        for (int j = 0; j < listSize; j++) {
            std::shared_ptr<T> tempValue = values[i]->removeFirst();
            int tempID = computeHash(tempValue->getID(), tempLength);
            newValues[tempID]->insert(tempValue, tempValue->getID());
        }
    }
    length = tempLength;
    values = std::move(newValues);
}

template<class T>
bool HashTable<T>::insert(int id, std::shared_ptr<T> val) {
    countValues++;
    if (countValues > length*2) {
        resize();
    }

    int hashValue = computeHash(id, length);
    values[hashValue]->insert(val, id);
    return true;
}

template<class T> //TODO can maybe remove the check from here because they are already made in plains
bool HashTable<T>::remove(int id) {
    int hashValue = computeHash(id, length);
    if (values[hashValue]->search(id)) {
        values[hashValue]->remove(id);
        countValues--;
        return true;
    }
    return false;
}

template<class T>
bool HashTable<T>::removeByRecord(int id, int record){

    int hashValue = computeHash(record, length);
    std::shared_ptr<Node<T>> currentNode = values[hashValue]->getStart(),
            previousNode = currentNode;
    while(currentNode != nullptr){
        if(currentNode->getData()->getID() == id){
            if (currentNode->getData()->getID() == values[hashValue]->getStart()->getData()->getID()){
                values[hashValue]->setStart(values[hashValue]->getStart()->getNext());
            } else{
                previousNode->setNext(currentNode->getNext());
            }
            values[hashValue]->decreaseSize();
            countValues--;
            return true;
        }
        previousNode = currentNode;
        currentNode = currentNode->getNext();
    }
    return false;
}


template<class T>
std::shared_ptr<T> HashTable<T>::search(int id) {
    int hashValue = computeHash(id, length);
    return values[hashValue]->getValue(id); //This function returns nullptr if doesn't exist
}


//TODO needs to be checked for new behavior
template<class T>
std::shared_ptr<T> HashTable<T>::searchByRecord(int id, int record){
    int hashValue = computeHash(record, length);
    auto currentNode = values[hashValue]->getStart();
    while(currentNode != nullptr){
        if(currentNode->getData()->getID() == id){
            return currentNode->getData();
        }
        currentNode = currentNode->getNext();
    }
    return nullptr;
}


template<class T>
int HashTable<T>::countInstances(int num) {
    int hashValue = computeHash(num, length);
    return values[hashValue]->countInstances(num);
}
