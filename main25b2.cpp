// 
// 234218 Data Structures 1.
// Semester: 2025B (Spring).
// Wet Exercise #.
// 
// The following main file is necessary to link and run your code.
// This file is READ ONLY: even if you submit something else, the compiler will use our file.
// 

#include "dspotify25b2.h"
#include <string>
#include <iostream>

using namespace std;

void print(string cmd, StatusType res);
void print(string cmd, output_t<int> res);

int main()
{
    
    int d1, d2, d3;

    // Init
    DSpotify *obj = new DSpotify();
    
    // Execute all commands in file
    string op;
    while (cin >> op)
    {
        if (!op.compare("addGenre")) {
            cin >> d1;
            print(op, obj->addGenre(d1));
        } else if (!op.compare("addSong")) {
            cin >> d1 >> d2;
            print(op, obj->addSong(d1, d2));
        } else if (!op.compare("mergeGenres")) {
            cin >> d1 >> d2 >> d3;
            print(op, obj->mergeGenres(d1, d2, d3));
        } else if (!op.compare("getSongGenre")) {
            cin >> d1;
            print(op, obj->getSongGenre(d1));
        } else if (!op.compare("getNumberOfSongsByGenre")) {
            cin >> d1;
            print(op, obj->getNumberOfSongsByGenre(d1));
        } else if (!op.compare("getNumberOfGenreChanges")) {
            cin >> d1;
            print(op, obj->getNumberOfGenreChanges(d1));
        } else {
            cout << "Unknown command: " << op << endl;
            break;
        }
        // Verify no faults
        if (cin.fail()){
            cout << "Invalid input format" << endl;
            break;
        }
    }

    // Quit 
    delete obj;
    return 0;
}

// Helpers
static const char *StatusTypeStr[] =
{
    "SUCCESS",
    "ALLOCATION_ERROR",
    "INVALID_INPUT",
    "FAILURE"
};

void print(string cmd, StatusType res) 
{
    cout << cmd << ": " << StatusTypeStr[(int) res] << endl;
}

void print(string cmd, output_t<int> res)
{
    if (res.status() == StatusType::SUCCESS) {
        cout << cmd << ": " << StatusTypeStr[(int) res.status()] << ", " << res.ans() << endl;
    } else {
        cout << cmd << ": " << StatusTypeStr[(int) res.status()] << endl;
    }
}
