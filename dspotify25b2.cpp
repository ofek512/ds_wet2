// You can edit anything you want in this file.
// However you need to implement all public DSpotify function, as provided below as a template

#include "dspotify25b2.h"


DSpotify::DSpotify() : genres(), songs() {}

DSpotify::~DSpotify() {
    // probably need to do nothing here since smart pointers
}

StatusType DSpotify::addGenre(int genreId) {
    /// Input validation
    if (genreId <= 0) {
        return StatusType::INVALID_INPUT;
    } else {
        // Check if genre already exists
        if (genres.member(genreId)) {
            return StatusType::FAILURE; // Genre already exists
        }
        // Create a new genre and add it to the hash table
        shared_ptr<Genre> newGenre = make_shared<Genre>(genreId);
        genres.insert(newGenre);
        return StatusType::SUCCESS;
    }
} /// V

StatusType DSpotify::addSong(int songId, int genreId) {
    // Input validation
    if (songId <= 0 || genreId <= 0) {
        return StatusType::INVALID_INPUT;
    }

    // check if genre exists
    if (!genres.member(genreId)) { /// check if need to add other checks
        return StatusType::FAILURE; // Genre does not exist
    }

    // check if song exists
    if (songs.member(songId)) {
        return StatusType::FAILURE; // Song already exists
    }

    // Create song node and add it to the hash table and then to the union find
    shared_ptr<Song> newSong = make_shared<Song>(songId);
    shared_ptr<Genre> genre = *genres.member(
            genreId); // Get the genre from the hash table

    // Add to the hash
    StatusType status = songs.insert(newSong);
    if (status != StatusType::SUCCESS) {
        return status; // Insertion failed
    }

    // Get the root from the genre
    shared_ptr<Song> rootGenreSong = genre->getRoot();

    // If the genre has no root, set the new song as the root
    if (!rootGenreSong) {
        genre->setRoot(newSong);
    } else {
        // Otherwise, set the new song's father to the root of the genre
        newSong->setFather(rootGenreSong);
    }
    genre->incrementSize();


    return StatusType::SUCCESS;
} /// V

StatusType DSpotify::mergeGenres(int genreId1, int genreId2, int genreId3) {
    return StatusType::FAILURE;
}

output_t<int> DSpotify::getSongGenre(int songId) {
    // Input validation
    if (songId <= 0) {
        return output_t<int>(StatusType::INVALID_INPUT);
    }

    // Check if song exists
    if (!songs.member(songId)) {
        return output_t<int>(StatusType::FAILURE); // Song does not exist
    }

    // Get the song from the hash table
    shared_ptr<Song> song = *songs.member(songId);

    // Find the root with path compression
    shared_ptr<Song> root = findSet(song);

    // Get the genre from the root song
    shared_ptr<Genre> genre = root->getGenre();
    if (!genre) {
        return output_t<int>(StatusType::FAILURE); // Root song has no genre
    }

    return output_t<int>(genre->getGenreId());
} /// V

output_t<int> DSpotify::getNumberOfSongsByGenre(int genreId) {
    // Input validation
    if (genreId <= 0) {
        return output_t<int>(StatusType::INVALID_INPUT);
    }

    // Check if genre exists
    if (!genres.member(genreId)) {
        return output_t<int>(StatusType::FAILURE); // Genre does not exist
    }

    // Get the genre from the hash table
    shared_ptr<Genre> genre = *genres.member(genreId);
    return output_t<int>(genre->getSize());

} /// V

output_t<int> DSpotify::getNumberOfGenreChanges(int songId) {
    return 0;
}

/// ---------- helper methods ----------

shared_ptr<Song> DSpotify::findSet(shared_ptr<Song> song) {
    if (!song->getFather()) {
        return song;  // This is the root
    }

    // Path compression - set father to root
    song->setFather(findSet(song->getFather()));
    return song->getFather();
}
