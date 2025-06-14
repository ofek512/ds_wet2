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
        newSong->incrementGenreChanges(); // Increment genre changes for the new root
    } else {
        // Otherwise, set the new song's father to the root of the genre
        newSong->setFather(rootGenreSong);
        newSong->decreeseGenreChanges(rootGenreSong->getGenreChanges());
        newSong->incrementGenreChanges(); // Increment genre changes for the new root
    }
    genre->incrementSize();


    return StatusType::SUCCESS;
} /// V

StatusType DSpotify::mergeGenres(int genreId1, int genreId2, int genreId3) {
    // Input validation
    if (genreId1 <= 0 || genreId2 <= 0 || genreId3 <= 0) {
        return StatusType::INVALID_INPUT;
    } else if (genreId1 == genreId2 || genreId1 == genreId3 || genreId2 == genreId3) {
        return StatusType::INVALID_INPUT; // Cannot merge the same genre
    }

    // Check if genres exist
    if (!genres.member(genreId1) || !genres.member(genreId2)) {
        return StatusType::FAILURE; // One or more genres do not exist
    }

    // If genre 3 already exists, return failure
    if (genres.member(genreId3)) {
        return StatusType::FAILURE; // Genre 3 already exists
    }

    // Get the genres from the hash table
    shared_ptr<Genre> genre1 = *genres.member(genreId1);
    shared_ptr<Genre> genre2 = *genres.member(genreId2);

    // Create a new genre for the merged genres
    shared_ptr<Genre> newGenre = make_shared<Genre>(genreId3);

    // Calculate the size for the new genre
    int newSize = genre1->getSize() + genre2->getSize();
    newGenre->setSize(newSize);

    // Handle empty genres case
    shared_ptr<Song> root1 = genre1->getRoot();
    shared_ptr<Song> root2 = genre2->getRoot();

    if (!root1 && !root2) {
        // Both genres are empty - insert new empty genre
    } else if (!root1) {
        // Only genre2 has songs
        newGenre->setRoot(root2);
        root2->setGenre(newGenre);  // Update genre pointer
        if(root2-> getGenreChanges() == 0) {
            root2->setGenreChanges(2);
        } else {
            root2->incrementGenreChanges();
        }
    } else if (!root2) {
        // Only genre1 has songs
        newGenre->setRoot(root1);
        root1->setGenre(newGenre);  // Update genre pointer
        if(root1-> getGenreChanges() == 0) {
            root1->setGenreChanges(2);
        } else {
            root1->incrementGenreChanges();
        }
    } else {
        // Both genres have songs
        if (genre1->getSize() >= genre2->getSize()) {
            // attach smaller tree (root2) under root1
            root2->setFather(root1);
            newGenre->setRoot(root1);
            root1->setGenre(newGenre);

            if(genre1->getRoot() -> getGenreChanges() == 0) {
                genre1->getRoot() ->setGenreChanges(2);
                if(root2->getGenreChanges() == 0) {
                    root2->setGenreChanges(0);
                } else {
                    root2->decreeseGenreChanges(1);
                }

            } else {
                if(root2->getGenreChanges() == 0) {
                    root2->decreeseGenreChanges(root1->getGenreChanges());
                    root2->addGenreChanges(2);
                } else {
                    root2->decreeseGenreChanges(root1->getGenreChanges());
                }
                root1->incrementGenreChanges();
            }

        } else {
            // attach smaller tree (root2) under root1
            root1->setFather(root2);
            newGenre->setRoot(root2);
            root2->setGenre(newGenre);

            if(genre2->getRoot() -> getGenreChanges() == 0) {
                genre2->getRoot() ->setGenreChanges(2);
                if(root1->getGenreChanges() == 0) {
                    root1->setGenreChanges(0);
                } else {
                    root1->decreeseGenreChanges(1);
                }

            } else {
                if(root1->getGenreChanges() == 0) {
                    root1->decreeseGenreChanges(root2->getGenreChanges());
                    root1->addGenreChanges(2);
                } else {
                    root1->decreeseGenreChanges(root2->getGenreChanges());
                }
                root2->incrementGenreChanges();
            }
        }
    }

    // Insert the new genre into the hash table
    genres.insert(newGenre);

    // Make the original genres empty instead of removing them
    genre1->setRoot(nullptr);
    genre1->setSize(0);

    genre2->setRoot(nullptr);
    genre2->setSize(0);

    return StatusType::SUCCESS;
} /// need to add genre changes fixes

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
    // input validation
    if (songId <= 0) {
        return output_t<int>(StatusType::INVALID_INPUT);
    }

    // check if song exists
    if (!songs.member(songId)) {
        return output_t<int>(StatusType::FAILURE); // Song does not exist
    }

    // Get the song from the hash table
    shared_ptr<Song> song = *songs.member(songId);
    // Compress the path to find the root song and then simply find Song again
    auto root = findSet(song);
    if (song == root) {
        return output_t<int>(root->getGenreChanges());
    } else {
        return output_t<int>(song->getGenreChanges() + root->getGenreChanges());
    }
}

/// ---------- helper methods ----------
shared_ptr<Song> DSpotify::findSet(shared_ptr<Song> song) {
    if (!song->getFather()) {
        return song;  // This is the root
    }

    // First find the root without compression
    shared_ptr<Song> root = song;
    int totalChanges = song->getGenreChanges();
    while (root->getFather()) {
        root = root->getFather();
        totalChanges += root->getGenreChanges();
    }

    // Now compress the path and update genre changes
    shared_ptr<Song> current = song;
    shared_ptr<Song> parent;
    while (current != root) {
        parent = current->getFather();
        current->setFather(root);
        // Update genre changes to preserve the total
        int currentChanges = current->getGenreChanges();
        current->setGenreChanges(totalChanges - root->getGenreChanges());
        totalChanges -= currentChanges;
        current = parent;
    }

    return root;
}
