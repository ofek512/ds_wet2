// You can edit anything you want in this file.
// However you need to implement all public DSpotify function, as provided below as a template

#include "dspotify25b2.h"


DSpotify::DSpotify(){

}

DSpotify::~DSpotify(){

}

StatusType DSpotify::addGenre(int genreId){
    return StatusType::FAILURE;
}

StatusType DSpotify::addSong(int songId, int genreId){
    return StatusType::FAILURE;
}

StatusType DSpotify::mergeGenres(int genreId1, int genreId2, int genreId3){
    return StatusType::FAILURE;
}

output_t<int> DSpotify::getSongGenre(int songId){
    return 0;
}

output_t<int> DSpotify::getNumberOfSongsByGenre(int genreId){
    return 0;
}

output_t<int> DSpotify::getNumberOfGenreChanges(int songId){
    return 0;
}
