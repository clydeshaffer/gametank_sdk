#ifndef UNDO_H_
#define UNDO_H_

// About the undo buffer
// The undo buffer is a bit of a ring buffer which takes up `UNDO_BUFFER_SIZE` bytes
// (currently 256) and stores twice that many entries (aka `MAX_UNDO_MOVES`)
//
// Two moves are stored in each byte like so:
// 0BDD0BDD
//
// 0 is unused
// B defines whether or not a barrel was pushed
// D defines which direction was taken (2 bits for 4 directions)
//
// Each nibble contains a move

void reset_undo(void);

void undo_buffer_push(char move);

// Returns 0xFF if the undo buffer is empty
char undo_buffer_pop();

#endif // UNDO_H_
