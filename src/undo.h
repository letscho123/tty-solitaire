#ifndef TTY_SOLITAIRE_UNDO_H
#define TTY_SOLITAIRE_UNDO_H

#include <stdbool.h>

#include "game.h"

enum undo_type {
  UNDO_DRAW_STOCK,
  UNDO_RECYCLE_WASTE,
  UNDO_EXPOSE_CARD,
  UNDO_MOVE_CARD,
  UNDO_MOVE_BLOCK,
  UNDO_AUTO_MOVE_FOUNDATION
};

#define MAX_UNDO_DEPTH 1000

struct undo_entry {
  enum undo_type type;
  int origin_id;
  int dest_id;
  int block_size;
  bool auto_exposed;
};

struct undo_history {
  struct undo_entry entries[MAX_UNDO_DEPTH];
  int count;
};

extern struct undo_history undo_history;

void undo_push(enum undo_type, int, int, int, bool);
void undo_last_move(void);
int get_stack_id(struct stack *);
struct stack **stack_by_id(int);

#endif
