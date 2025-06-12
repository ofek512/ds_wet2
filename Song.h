//
// Created by ofek5 on 2025-06-08.
//

#ifndef DS_WET2_SONG_H
#define DS_WET2_SONG_H

#include <memory>
#include "Genre.h"

using namespace std;

class Song {
private:
    shared_ptr<Genre> genre;
    shared_ptr<Song> father; // pointer to the father song in the union find
    int song_id;
    int genreChanges;

public:
    // constructor
    Song(int id) : song_id(id), genreChanges(0), father(nullptr),
                   genre(nullptr) {}

    // destructor
    ~Song() = default;

    // getters
    int getSongId() const {
        return song_id;
    }

    int getGenreChanges() const {
        return genreChanges;
    }

    shared_ptr<Genre> getGenre() const {
        return genre;
    }

    shared_ptr<Song> getFather() const {
        return father;
    }

    // setters
    void setGenre(shared_ptr<Genre> new_genre) {
        genre = new_genre;
    }

    void setFather(shared_ptr<Song> new_father) {
        father = new_father;
    }

    void incrementGenreChanges() {
        genreChanges++;
    }

    void decrementGenreChanges() {
        if (genreChanges > 0) {
            genreChanges--;
        }
    }

    // cast to int
    operator int() const {
        return song_id;
    }

    // comparison operator
    bool operator==(const Song &other) const {
        return song_id == other.song_id;
    }

    bool operator!=(const Song &other) const {
        return !(*this == other);
    }

    bool operator<(const Song &other) const {
        return song_id < other.song_id;
    }

};


#endif //DS_WET2_SONG_H
