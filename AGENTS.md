# tty-solitaire — Agent Summary

## Project

Ncurses-based Klondike solitaire game written in C99. Build with `make`, test with `make test`.

### Architecture (bottom-up)

| Module | Purpose |
|--------|---------|
| `src/frame.{c,h}` | 7x5 ncurses WINDOW wrapper per card |
| `src/card.{c,h}` | Playing card: value, suit, face, position |
| `src/stack.{c,h}` | Singly-linked list with sentinel empty-card pattern |
| `src/deck.{c,h}` | Game layout: stock, waste, 4 foundations, 7 maneuvre stacks |
| `src/cursor.{c,h}` | 1x1 ncurses window; maps x/y to stacks |
| `src/game.{c,h}` | Game logic: init, shuffle, deal, move validation, win check |
| `src/gui.{c,h}` | Rendering: Unicode suit symbols, color pairs |
| `src/keyboard.{c,h}` | Input: movement (hjkl/arrows), card selection (m/n/M/N), placing, resize |
| `src/common.{c,h}` | Terminal size check, error handling |
| `src/ttysolitaire.c` | Main entry: CLI parsing, ncurses init, game loop |

### Key patterns
- Globals `deck` and `cursor` defined in `game.c`, extern'd elsewhere
- Empty stacks: sentinel card with `NO_VALUE/NO_SUIT/NO_FACE`, `next==NULL`
- Card positions: absolute terminal coords on each frame
- Only external dependency: `ncursesw`

## Undo Feature (`u` key)

### Files added
- `src/undo.h` — Types (`enum undo_type`, `struct undo_entry`, `struct undo_history`), declarations
- `src/undo.c` — 1000-slot undo queue and reverse logic for 6 action types

### Files modified
- `src/game.h` / `src/game.c` — `foundation_begin_x()` and `maneuvre_begin_x()` made non-static
- `src/keyboard.c` — Records undo entries at each state-changing action; handles `u` key
- `Makefile` — Added `undo.o`

### Undo action types

| Type | Recorded at | Undo logic |
|------|------------|------------|
| `UNDO_DRAW_STOCK` | Space on stock | `move_card(waste, stock)` + cover + expose next waste top |
| `UNDO_RECYCLE_WASTE` | Space on empty stock | `move_card(stock, waste)` for each card, all exposed |
| `UNDO_EXPOSE_CARD` | Space on covered maneuvre | `card_cover` on top card |
| `UNDO_MOVE_CARD` | Single card move | `erase_stack` both → `move_card(dest, origin)` → re-cover auto-exposed |
| `UNDO_MOVE_BLOCK` | Block move | `erase_stack` both → `move_block(dest, origin)` → re-cover auto-exposed |
| `UNDO_AUTO_MOVE` | Auto-move to foundation | `erase_stack` both → `move_card(dest, origin)` |

### Stack identification
Uses position-based integer IDs (0=stock, 1=waste, 2-5=foundations, 6-12=maneuvres) via `get_stack_id()` / `stack_by_id()`. Captured before moves execute to avoid pointer invalidation.

### Key constraints
- All undo move operations use `move_card()`/`move_block()` (not manual pop/push) so frame positions update correctly
- Cursor stays in place during undo (no repositioning)
- `expose_top()` state (from user's latest commit) is tracked via `auto_exposed` flag and reversed on undo
- Passes counter only adjusted by `UNDO_DRAW_STOCK` when the original draw decremented it
