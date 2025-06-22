// You can edit anything you want in this file.
// However you need to implement all public DSpotify function, as provided below as a template

#include "dspotify25b2.h"


DSpotify::DSpotify() : genres(), songs() {}


DSpotify::~DSpotify() {
//Songs cleanup
    int maxSongs = songs.getMaxSize();
    shared_ptr<Song> toCheck;
    for (int i = 0; i < maxSongs; i++) {
        if (songs.getTable()[i]->data && !(songs.getTable()[i]->is_deleted)) {
            toCheck = *(songs.getTable()[i]->data);
            toCheck->setFather(nullptr);
            toCheck->setGenre(nullptr);

        }
    }
//Genres cleanup
    int maxGenres = genres.getMaxSize();
    shared_ptr<Genre> toCheckGenre;
    for (int j = 0; j < maxGenres; j++) {
        if (genres.getTable()[j]->data && !(genres.getTable()[j]->is_deleted)) {
            toCheckGenre = *(genres.getTable()[j]->data);
            toCheckGenre->setRoot(nullptr);
        }
    }
}



StatusType DSpotify::addGenre(int genreId){
    if(genreId <= 0){
        return StatusType::INVALID_INPUT;
    }
    else{
        shared_ptr<Genre> newGenre = *(genres).member(genreId);
        if(newGenre){
            StatusType status = genres.insert(newGenre);
            if(status == StatusType::SUCCESS){
                return StatusType::SUCCESS;
            }
            else{
                return status;
            }
        }
    }
    return StatusType::FAILURE;
}

StatusType DSpotify::addSong(int songId, int genreId){
    if(songId <= 0 || genreId <= 0){
        return StatusType::INVALID_INPUT;
    }
    shared_ptr<Song> song = make_shared<Song>(songId);
    shared_ptr<Genre> genre = *genres.member(genreId);
    StatusType status = songs.insert(song);
    if(status != StatusType::SUCCESS){ return status;}
    shared_ptr<Song> rootGenreSong = genre->getRoot();
    //if now root in genre, set the song as root
    if(!rootGenreSong){
        genre->setRoot(song);
    }
    //if root exists, set the song's father to be the root song:
    else {
        song->setFather(rootGenreSong);
    }


    return StatusType::FAILURE;
}

StatusType DSpotify::mergeGenres(int genreId1, int genreId2, int genreId3) {
    if (genreId1 <= 0 || genreId2 <= 0 || genreId3 <= 0) {
        return StatusType::INVALID_INPUT;
    }
    if (genres.member(genreId3)) {
        return StatusType::FAILURE;
    }
    if (!genres.member(genreId1) || !genres.member(genreId2)) {
        return StatusType::FAILURE; // one of the genres does not exist
    }
    shared_ptr<Genre> genre1 = *genres.member(genreId1);
    shared_ptr<Genre> genre2 = *genres.member(genreId2);
    shared_ptr<Genre> genre3 = make_shared<Genre>(genreId3);
    shared_ptr<Song> root1 = genre1->getRoot();
    shared_ptr<Song> root2 = genre2->getRoot();
    //both genres are empty
    if (genre1->getSize() == 0 && genre2->getSize() == 0) {
        genre3->setRoot(nullptr);
        genre3->setSize(0);
        genres.insert(genre3);
        return StatusType::SUCCESS;
    } else if (genre1->getSize() == 0) {
        genre3->setRoot(root2);;
        root2->setGenre(genre3);
        root2->incrementGenreChanges();
    } else if (genre2->getSize() == 0) {
        genre3->setRoot(root1);;
        root1->setGenre(genre3);
        root1->incrementGenreChanges();
    } else {
        if (genre1->getSize() >= genre2->getSize()) {
            root2->setFather(root1);
            genre3->setRoot(root1);
            root1->setGenre(genre3);
            root2->decreeseGenreChanges(root1->getGenreChanges());
            root1->incrementGenreChanges();
        } else {
            root1->setFather(root2);
            genre3->setRoot(root2);
            root2->setGenre(genre3);
                root1->decreeseGenreChanges(root2->getGenreChanges());
                root2->incrementGenreChanges();
        }
    }
    genres.insert(genre3);
    genre1->setRoot(nullptr);
    genre2->setRoot(nullptr);
    genre1->setSize(0);
    genre2->setSize(0);
    return StatusType::SUCCESS;
}



output_t<int> DSpotify::getSongGenre(int songId){
    if(songId <= 0){
        return StatusType::INVALID_INPUT;
    }
    shared_ptr<Song> song = *songs.member(songId);
    if(!song){
        return StatusType::FAILURE;
    }
    shared_ptr<Song> root = findSet(song);
    shared_ptr<Genre> genre = root->getGenre();
    if(!genre){
        return StatusType::FAILURE;
    }
    int genreId = genre->getGenreId();
    if(genreId <= 0){
        return StatusType::FAILURE;
    }
    return output_t<int>(genreId);
}

output_t<int> DSpotify::getNumberOfSongsByGenre(int genreId){
    if(genreId <= 0){
        return StatusType::INVALID_INPUT;
    }
    if(!genres.member(genreId)){
        return StatusType::FAILURE;
    }
    shared_ptr<Genre> genre = *genres.member(genreId);
    return output_t<int>(genre->getSize());
}

output_t<int> DSpotify::getNumberOfGenreChanges(int songId){
    if(songId <= 0){
        return StatusType::INVALID_INPUT;
    }
    if(!songs.member(songId)){
        return StatusType::FAILURE; // song does not exist
    }
    shared_ptr<Song> song = *songs.member(songId);
    shared_ptr<Song> root = findSet(song);
    if(song == root) {
        return output_t<int>(song->getGenreChanges());
    } else{
        return output_t<int>(song->getGenreChanges() + root->getGenreChanges());
    }
    return 0;
}


/// ---------- helper methods ----------
shared_ptr<Song> DSpotify::findSet(shared_ptr<Song> song) {
    if (!song->getFather()) {
        return song;  // This is the root
    }

    //Find Root
    shared_ptr<Song> root = song;
    int totalChanges = song->getGenreChanges();
    while (root->getFather()) {
        root = root->getFather();
        totalChanges += root->getGenreChanges();
    }

    //Path Compression
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
