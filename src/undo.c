#include <stdio.h>
#include <stdlib.h>

#include "card.h"
#include "deck.h"
#include "game.h"
#include "gui.h"
#include "stack.h"
#include "undo.h"

struct undo_history undo_history;

int get_stack_id(struct stack *stack) {
  if (!stack) return -1;
  if (stock_stack(stack)) return 0;
  if (waste_pile_stack(stack)) return 1;
  if (stack->card->frame->begin_y == FOUNDATION_BEGIN_Y) {
    for (int i = 0; i < FOUNDATION_STACKS_NUMBER; i++) {
      if (stack->card->frame->begin_x == foundation_begin_x(i)) return 2 + i;
    }
  }
  if (stack->card->frame->begin_y >= MANEUVRE_BEGIN_Y) {
    for (int i = 0; i < MANEUVRE_STACKS_NUMBER; i++) {
      if (stack->card->frame->begin_x == maneuvre_begin_x(i)) return 6 + i;
    }
  }
  return -1;
}

struct stack **stack_by_id(int id) {
  switch (id) {
  case 0: return &deck->stock;
  case 1: return &deck->waste_pile;
  case 2: return &deck->foundation[0];
  case 3: return &deck->foundation[1];
  case 4: return &deck->foundation[2];
  case 5: return &deck->foundation[3];
  case 6: return &deck->maneuvre[0];
  case 7: return &deck->maneuvre[1];
  case 8: return &deck->maneuvre[2];
  case 9: return &deck->maneuvre[3];
  case 10: return &deck->maneuvre[4];
  case 11: return &deck->maneuvre[5];
  case 12: return &deck->maneuvre[6];
  default: return NULL;
  }
}

void undo_push(enum undo_type type, int origin_id, int dest_id,
               int block_size) {
  if (undo_history.count >= MAX_UNDO_DEPTH) return;
  struct undo_entry *entry = &undo_history.entries[undo_history.count++];
  entry->type = type;
  entry->origin_id = origin_id;
  entry->dest_id = dest_id;
  entry->block_size = block_size;
}

void undo_last_move(void) {
  if (undo_history.count == 0) return;

  struct undo_entry *entry = &undo_history.entries[undo_history.count - 1];

  switch (entry->type) {
  case UNDO_DRAW_STOCK: {
    struct stack **waste = stack_by_id(1);
    struct stack **stock = stack_by_id(0);
    if (!stack_empty(*waste)) {
      erase_stack(*stock);
      erase_stack(*waste);
      move_card(waste, stock);
      card_cover((*stock)->card);
      if (!stack_empty(*waste)) {
        card_expose((*waste)->card);
      }
      if (entry->block_size) {
        game.passes_through_deck_left++;
      }
      draw_stack(*stock);
      draw_stack(*waste);
    }
    break;
  }
  case UNDO_RECYCLE_WASTE: {
    struct stack **stock = stack_by_id(0);
    struct stack **waste = stack_by_id(1);
    int count = entry->block_size;
    if (count <= 0) break;
    erase_stack(*stock);
    erase_stack(*waste);
    for (int i = 0; i < count; i++) {
      if (!stack_empty(*stock)) {
        move_card(stock, waste);
        card_expose((*waste)->card);
      }
    }
    draw_stack(*stock);
    draw_stack(*waste);
    break;
  }
  case UNDO_EXPOSE_CARD: {
    struct stack **origin = stack_by_id(entry->origin_id);
    if (origin && !stack_empty(*origin)) {
      card_cover((*origin)->card);
      erase_card((*origin)->card);
      draw_card((*origin)->card);
    }
    break;
  }
  case UNDO_MOVE_CARD: {
    struct stack **origin = stack_by_id(entry->origin_id);
    struct stack **dest = stack_by_id(entry->dest_id);
    erase_stack(*origin);
    erase_stack(*dest);
    move_card(dest, origin);
    draw_stack(*origin);
    draw_stack(*dest);
    break;
  }
  case UNDO_MOVE_BLOCK: {
    struct stack **origin = stack_by_id(entry->origin_id);
    struct stack **dest = stack_by_id(entry->dest_id);
    erase_stack(*origin);
    erase_stack(*dest);
    move_block(dest, origin, entry->block_size);
    draw_stack(*origin);
    draw_stack(*dest);
    break;
  }
  case UNDO_AUTO_MOVE_FOUNDATION: {
    struct stack **origin = stack_by_id(entry->origin_id);
    struct stack **dest = stack_by_id(entry->dest_id);
    erase_stack(*origin);
    erase_stack(*dest);
    move_card(dest, origin);
    draw_stack(*origin);
    draw_stack(*dest);
    break;
  }
  }

  undo_history.count--;

  if (undo_history.count == 0) {
    draw_deck(deck);
  }
}
