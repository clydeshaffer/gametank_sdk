#ifndef UNDO_H_
#define UNDO_H_

void reset_undo(void);

void undo_buffer_push(char move);

// Returns 0xFF if the undo buffer is empty
char undo_buffer_pop();

#endif // UNDO_H_
