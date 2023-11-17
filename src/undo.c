#include "undo.h"

#define UNDO_BUFFER_SIZE 256
#define MAX_UNDO_MOVES (UNDO_BUFFER_SIZE * 2)

char undo_buffer[UNDO_BUFFER_SIZE];
short undo_moves_remaining;
short current_undo_slot;

void reset_undo() {
  current_undo_slot = 0;
  undo_moves_remaining = 0;
}

void undo_buffer_push(char move) {
    // The undo buffer is a bit of a ring buffer that can hold 256 entries
    // We always increment the current undo slot (allowing it to wrap and
    // overwrite old entries) but we cap the undo moves remaining at 255
    // as to not allow undoing moves which have been overwritten
    char index = (char)(current_undo_slot >> 1);

    if (current_undo_slot & 1) {
        char value = undo_buffer[index];
        undo_buffer[index] = value | (move << 4);
    } else {
        undo_buffer[index] = move;
    }

    current_undo_slot++;
    if (current_undo_slot >= MAX_UNDO_MOVES)
        current_undo_slot -= MAX_UNDO_MOVES;

    if (undo_moves_remaining < MAX_UNDO_MOVES)
        undo_moves_remaining++;
}

// Returns 0xFF if the undo buffer is empty
char undo_buffer_pop() {
    char value;

    if (!undo_moves_remaining)
        return 0xFF;

    undo_moves_remaining--;

    if (current_undo_slot == 0)
        current_undo_slot = MAX_UNDO_MOVES - 1;
    else
        current_undo_slot--;

    // The parity of current_undo_slot tells us which nibble to check
    if (current_undo_slot & 1) {
        value = undo_buffer[current_undo_slot >> 1] & 0xF0;
        value >>= 4;
    } else {
        value = undo_buffer[current_undo_slot >> 1] & 0x0F;
    }

    return value;
}
