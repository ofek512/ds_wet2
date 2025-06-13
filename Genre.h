//
// Created by ofek5 on 2025-06-08.
//

#ifndef DS_WET2_GENRE_H
#define DS_WET2_GENRE_H

#include "Song.h"
#include <memory>
#include <utility>

using namespace std;

// the genre clase will hold a pointer to the root of the union find, which is a song.
// the genre class has genre id, size.

/// forward declaration of class song

class Song;

class Genre {

private:
    shared_ptr<Song> root; // pointer to the root of the union find
    int genre_id;
    int size;

public:
    // Constructor
    Genre(int id) : root(nullptr), genre_id(id), size(0) {}

    // Destructor
    ~Genre() = default;

    // Getters
    int getGenreId() const {
        return genre_id;
    }

    int getSize() const {
        return size;
    }

    shared_ptr<Song> getRoot() const {
        return root;
    }

    // Increment and decrement size
    void incrementSize() {
        size++;
    }

    void decrementSize() {
        if (size > 0) {
            size--;
        }
    }

    // Setters
    void setRoot(shared_ptr<Song> new_root) {
        root = std::move(new_root); // check if necessary to use std::move
    }

    void setSize(int new_size) {
        size = new_size;
    }

    // cast to int
    operator int() const {
        return genre_id;
    }

    // Comparison operators
    bool operator==(const Genre &other) const {
        return genre_id == other.genre_id;
    }

    bool operator!=(const Genre &other) const {
        return !(*this == other);
    }

    bool operator<(const Genre &other) const {
        return genre_id < other.genre_id;
    }


};


#endif // DS_WET2_GENRE_H
