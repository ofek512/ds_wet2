// 
// 234218 Data Structures 1.
// Semester: 2025B (Spring).
// Wet Exercise #2.
// 
// The following header file contains all methods we expect you to implement.
// You MAY add private methods and fields of your own.
// DO NOT erase or modify the signatures of the public methods.
// DO NOT modify the preprocessors in this file.
// DO NOT use the preprocessors in your other code files.
// 

#ifndef DSPOTIFY25SPRING_WET2_H_
#define DSPOTIFY25SPRING_WET2_H_

#include "wet2util.h"
#include "Genre.h" // genre.h includes song.h already.
#include "Song.h" // song.h includes genres already.
#include "Hash.h"

class DSpotify {
    // This class will act as a union find of groups(genres) of songs.
private:
    // Hash table for genres and songs to allow fast access to the upside down tree of songs.
    Hash<shared_ptr<Genre>> genres;
    Hash<shared_ptr<Song>> songs;

public:
    // <DO-NOT-MODIFY> {
    DSpotify(); /// V

    virtual ~DSpotify(); /// V

    shared_ptr<Song> findSetWithChanges(shared_ptr<Song> song);

    StatusType addGenre(int genreId); /// V

    StatusType addSong(int songId, int genreId);

    StatusType mergeGenres(int genreId1, int genreId2, int genreId3);

    output_t<int> getSongGenre(int songId);

    output_t<int> getNumberOfSongsByGenre(int genreId);

    output_t<int> getNumberOfGenreChanges(int songId);
    // } </DO-NOT-MODIFY>

    shared_ptr<Song> findSet(shared_ptr<Song> song);
};

#endif // DSPOTIFY25SPRING_WET2_H_
