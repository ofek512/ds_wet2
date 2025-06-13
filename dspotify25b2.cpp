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
        newSong->setGenre(genre); // Update genre pointer
    } else {
        // Otherwise, set the new song's father to the root of the genre
        newSong->setFather(rootGenreSong);
    }
    genre->incrementSize();


    return StatusType::SUCCESS;
} /// V

StatusType DSpotify::mergeGenres(int genreId1, int genreId2, int genreId3) {
    if (genreId1 <= 0 || genreId2 <= 0 || genreId3 <= 0)
        return StatusType::INVALID_INPUT;
    if (genreId1 == genreId2 || genreId1 == genreId3 || genreId2 == genreId3)
        return StatusType::INVALID_INPUT;
    if (!genres.member(genreId1) || !genres.member(genreId2))
        return StatusType::FAILURE;
    if (genres.member(genreId3))
        return StatusType::FAILURE;

    auto genre1 = *genres.member(genreId1);
    auto genre2 = *genres.member(genreId2);
    auto newGenre = make_shared<Genre>(genreId3);
    int newSize = genre1->getSize() + genre2->getSize();
    newGenre->setSize(newSize);

    auto root1 = genre1->getRoot();
    auto root2 = genre2->getRoot();

    if (root1 || root2) {
        if (root1) root1->incrementGenreChanges();
        if (root2) root2->incrementGenreChanges();

        if (!root1) {
            newGenre->setRoot(root2);
            root2->setGenre(newGenre);
        } else if (!root2) {
            newGenre->setRoot(root1);
            root1->setGenre(newGenre);
        } else if (genre1->getSize() >= genre2->getSize()) {
            root2->setFather(root1);
            newGenre->setRoot(root1);
            root1->setGenre(newGenre);
        } else {
            root1->setFather(root2);
            newGenre->setRoot(root2);
            root2->setGenre(newGenre);
        }
    }

    genres.insert(newGenre);
    genre1->setRoot(nullptr);
    genre1->setSize(0);
    genre2->setRoot(nullptr);
    genre2->setSize(0);
    return StatusType::SUCCESS;
}

output_t<int> DSpotify::getSongGenre(int songId) {
    // Input validation
    if (songId <= 0) {
        return {StatusType::INVALID_INPUT};
    }

    // Check if song exists
    if (!songs.member(songId)) {
        return {StatusType::FAILURE}; // Song does not exist
    }

    // Get the song from the hash table
    shared_ptr<Song> song = *songs.member(songId);

    // Find the root with path compression
    shared_ptr<Song> root = findSet(song);

    // Get the genre from the root song
    shared_ptr<Genre> genre = root->getGenre();
    if (!genre) {
        return {StatusType::FAILURE}; // Root song has no genre
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
    // Input validation
    if (songId <= 0) {
        return {StatusType::INVALID_INPUT};
    }

    // Check if song exists
    if (!songs.member(songId)) {
        return {StatusType::FAILURE};
    }

    // Get the song from the hash table
    shared_ptr<Song> song = *songs.member(songId);

    // Find the root - this updates the path and propagates genre changes
    shared_ptr<Song> root = findSet(song);

    // Get the number of genre changes for this song
    int changes = song->getGenreChanges();

    // Special case: If the song has no changes of its own (changes == 0) but its root does,
    // we need to return 2 instead of 1 to account for being part of a changed genre
    if (changes == 0 && root != song && root->getGenreChanges() > 0) {
        return {2};
    }

    // Otherwise, return the genre changes count + 1 (for initial assignment)
    return {changes + 1};
}

/// ---------- helper methods ----------

shared_ptr<Song> DSpotify::findSet(shared_ptr<Song> song) {
    auto father = song->getFather();
    if (!father) {
        // this is the root; nothing to accumulate
        return song;
    }

    // first, recursively find the true root
    shared_ptr<Song> root = findSet(father);

    // **always** absorb your (old) father's genreChanges
    song->setGenreChanges(
            song->getGenreChanges() + father->getGenreChanges()
    );

    // now, if your father wasn't already the root, compress the path
    if (father != root) {
        song->setFather(root);
    }

    return root;
}
