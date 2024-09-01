#include <array>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <stack>

#include "pico/rand.h"
#include "picosystem.hpp"

#include "tetris.hpp"

using namespace picosystem;

struct Board board;
struct Piece current_piece;
struct Types types;
struct MainTheme main_theme;

uint8_t volume = 100;
voice_t voice_main = voice(100, 100, 70, 100);
voice_t voice_noise = voice(0, 0, 70, 0, 0, 0, 0, 100);
int16_t duration_delay = 0;
char main_theme_note = 0;
bool main_theme_done = false;

states state = states::TITLE;

buffer_t *title_text = buffer(42, 10);
buffer_t *press_text = buffer(49, 10);
buffer_t *paused_text = buffer(42, 10);
buffer_t *game_over_text = buffer(73, 10);

buffer_t *score_text = buffer(50, 10);

std::deque<uint32_t> next_pieces;
int8_t hold_piece = -1;
uint32_t score = 0;

uint8_t pressed_time_x = 0;
char lock_delay = 20;
char current_y_offset = 0;
bool is_grounded = false;

uint32_t level = 1;
uint8_t lines_cleared = 0;
uint32_t tick = 0;

void draw_background()
{
  pen(2, 8, 2);
  clear();

  pen(0, 0, 0);

  frect(
      board.margin_left - 6,
      board.margin_top - 6,
      board.width * board.cell_size + 12,
      board.height * board.cell_size + 12);
}

void draw_board()
{
  pen(0, 0, 0);

  for (int y = 0; y < board.height; y++)
  {
    for (int x = 0; x < board.width; x++)
    {
      if (board.cells[y][x] > 0 && state != states::PAUSED)
      {
        continue;
      }

      pen(0, 0, 0);

      frect(
          board.margin_left + (x * board.cell_size) + 1,
          board.margin_top + (y * board.cell_size) + 1,
          board.cell_size - 2,
          board.cell_size - 2);

      pen(4, 4, 4);

      rect(
          board.margin_left + (x * board.cell_size),
          board.margin_top + (y * board.cell_size),
          board.cell_size,
          board.cell_size);
    }
  }
}

void draw_cell(int x, int y, color_t colour)
{
  pen(colour);
  frect(
      x,
      y,
      board.cell_size,
      board.cell_size);

  pen(15, 15, 15, 6);
  frect(
      x,
      y,
      board.cell_size,
      1);

  pen(0, 0, 0, 4);
  frect(
      x,
      y,
      1,
      board.cell_size);

  pen(0, 0, 0, 6);
  frect(
      x,
      y + board.cell_size - 1,
      board.cell_size,
      1);

  pen(0, 0, 0, 2);
  frect(
      x + board.cell_size - 1,
      y,
      1,
      board.cell_size);
}

void draw_cell_outline(int x, int y, color_t colour)
{
  pen(colour);

  rect(
      x,
      y,
      board.cell_size,
      board.cell_size);
}

void draw_next_container()
{
  pen(0, 0, 0);
  frect(board.width * board.cell_size + board.margin_left + 6, board.margin_top + 10, 42, 32);

  pen();
  font();
  text("NEXT", board.width * board.cell_size + board.margin_left + 13, board.margin_top + 7);

  pen(1, 1, 1);
  frect(board.width * board.cell_size + board.margin_left + 8, board.margin_top + 44, 37, 32);
  frect(board.width * board.cell_size + board.margin_left + 8, board.margin_top + 78, 37, 32);
}

void draw_next_pieces()
{
  for (int i = 0; i < 3; i++)
  {
    int x_offset = -board.cell_size / 4;
    int y_offset = 0;

    if (next_pieces[i] == 0)
    {
      x_offset = board.cell_size / 4;
      y_offset = board.cell_size / 2;
    }

    if (next_pieces[i] == 3)
    {
      x_offset = board.cell_size / 4;
    }

    for (int x = 0; x < 4; x++)
    {
      for (int y = 0; y < 3; y++)
      {
        if (types.shapes[next_pieces[i]][0][y][x] > 0)
        {
          draw_cell(
              board.margin_left + ((board.width + x) * board.cell_size) + x_offset + 8,
              board.margin_top + ((((i * 4) + y + 1) * board.cell_size) + 3) + y_offset + i,
              types.colours[next_pieces[i]]);
        }
      }
    }
  }
}

void draw_hold_container()
{
  pen(0, 0, 0);
  frect(board.margin_left - 6 - 42, board.margin_top + 10, 42, 32);

  pen();
  font();
  text("HOLD", board.margin_left + 3 - 42, board.margin_top + 7);
}

void draw_level_container()
{
  pen(1, 1, 1);
  frect(board.margin_left - 46, 84, 38, 28);

  pen();
  font();
  text("LEVEL", (board.margin_left / 2) - 17, 80);
}

void draw_level()
{
  pen(1, 1, 1);
  frect(board.margin_left - 33, 94, 12, 9);

  int8_t x_offset = 0;

  if (level >= 10)
  {
    x_offset = -3;
  }

  pen();
  font();
  text(str(level), (board.margin_left / 2) - 5 + x_offset, 95);
}

void draw_hold_piece()
{
  if (hold_piece > -1)
  {
    int x_offset = -board.cell_size / 4;
    int y_offset = 0;

    if (hold_piece == 0)
    {
      x_offset = board.cell_size / 4;
      y_offset = board.cell_size / 2;
    }

    if (hold_piece == 3)
    {
      x_offset = board.cell_size / 4;
    }

    for (int x = 0; x < 4; x++)
    {
      for (int y = 0; y < 3; y++)
      {
        if (types.shapes[hold_piece][0][y][x] > 0)
        {
          draw_cell(
              board.margin_left + ((x - 4) * board.cell_size) + x_offset - 14,
              board.margin_top + ((y + 1) * board.cell_size) + 3 + y_offset,
              types.colours[hold_piece]);
        }
      }
    }
  }
}

void draw_score()
{
  font(6, 10, 0);

  target(score_text);

  pen(0, 0, 0);
  frect(0, 0, 50, 12);

  pen();
  std::string score_string = std::to_string(score);
  std::string padding = std::string(8 - score_string.length(), '0');
  text(padding + score_string, 1, 1);

  target();

  blit(score_text, 0, 0, 50, 10, 40, 212, 100, 20);
}

void queue_random_pieces()
{
  int bag[7] = {0, 1, 2, 3, 4, 5, 6};

  for (int j = 0; j < 7; j++)
  {
    int r = get_rand_32() % 7;
    int tmp = bag[j];
    bag[j] = bag[r];
    bag[r] = tmp;
  }

  for (int j = 0; j < 7; j++)
  {
    next_pieces.push_back(bag[j]);
  }
}

void draw_title_screen(uint32_t tick)
{
  pen(2, 2, 8, 8);
  frect(30, 71, 180, 72);
  pen(15, 15, 15, 6);
  rect(30, 71, 180, 1);
  rect(31, 72, 178, 1);
  rect(32, 73, 176, 1);
  rect(33, 74, 174, 1);
  pen(15, 15, 15, 5);
  frect(30, 71, 1, 72);
  frect(31, 72, 1, 70);
  frect(32, 73, 1, 68);
  frect(33, 74, 1, 66);
  pen(0, 0, 4, 5);
  frect(210, 71, 1, 72);
  frect(209, 72, 1, 70);
  frect(208, 73, 1, 68);
  frect(207, 74, 1, 66);
  pen(0, 0, 4, 6);
  rect(30, 142, 180, 1);
  rect(31, 141, 178, 1);
  rect(32, 140, 176, 1);
  rect(33, 139, 174, 1);

  blit(title_text, 0, 0, 42, 10, 36, 80, 168, 40);

  if (tick % 40 < 20)
  {
    blit(press_text, 0, 0, 49, 10, 72, 120, 98, 20);
  }
}

void init_title_screen()
{
  font(6, 10, 1);

  target(title_text);

  pen(0, 0, 0, 8);
  text("TETRIS", 1, 1);
  pen(15, 3, 3);
  text("T", 0, 0);
  pen(14, 10, 0);
  text("E", 7, 0);
  pen(14, 14, 0);
  text("T", 14, 0);
  pen(0, 14, 0);
  text("R", 21, 0);
  pen(0, 14, 14);
  text("I", 28, 0);
  pen(14, 4, 14);
  text("S", 36, 0);

  target(press_text);

  pen(0, 0, 0, 8);
  text("PRESS A", 1, 1);
  pen();
  text("PRESS A", 0, 0);

  target();

  draw_board();
  draw_next_container();
  draw_hold_container();
  draw_level_container();
  draw_level();
  draw_next_pieces();
  draw_hold_piece();
  draw_score();

  draw_title_screen(0);
}

void init_paused_text()
{
  target(paused_text);

  font(6, 10, 1);
  pen(0, 0, 0, 8);
  text("PAUSED", 1, 1);
  pen();
  text("PAUSED", 0, 0);

  target();
}

void init_game_over_text()
{
  font(7, 10, 1);

  target(game_over_text);

  pen(0, 0, 0, 8);
  text("GAME OVER", 1, 1);
  pen();
  text("GAME OVER", 0, 0);

  target();
}

void draw_pieces()
{
  for (int x = 0; x < board.width; x++)
  {
    for (int y = 0; y < board.height; y++)
    {
      if (board.cells[y][x] > 0)
      {
        draw_cell(
            board.margin_left + (x * board.cell_size),
            board.margin_top + (y * board.cell_size),
            types.colours[board.cells[y][x] - 1]);
      }
    }
  }
}

bool can_move(int tx, int ty, int tr)
{
  for (int x = 0; x < 4; x++)
  {
    for (int y = 0; y < 4; y++)
    {
      int nx = tx + x;
      int ny = ty + y;

      if (
          types.shapes[current_piece.type][tr][y][x] > 0 &&
          (nx < 0 || nx >= board.width ||
           ny < 0 || ny >= board.height ||
           board.cells[ny][nx] > 0))
      {
        return false;
      }
    }
  }

  lock_delay = 20;
  return true;
}

void draw_current_piece()
{
  for (int x = 0; x < 4; x++)
  {
    for (int y = 0; y < 4; y++)
    {
      if (types.shapes[current_piece.type][current_piece.rotation][y][x] > 0)
      {
        draw_cell(
            board.margin_left + ((current_piece.x + x) * board.cell_size),
            board.margin_top + ((current_piece.y + y) * board.cell_size) + current_y_offset,
            types.colours[current_piece.type]);
      }
    }
  }
}

void draw_current_piece_outline()
{
  int bottom_y = current_piece.y;

  while (can_move(current_piece.x, bottom_y + 1, current_piece.rotation))
  {
    bottom_y++;
  }

  for (int x = 0; x < 4; x++)
  {
    for (int y = 0; y < 4; y++)
    {
      int nx = current_piece.x + x;
      int ny = bottom_y + y;

      if (
          types.shapes[current_piece.type][current_piece.rotation][y][x] > 0 &&
          (nx < 0 || nx >= board.width ||
           ny < 0 || ny >= board.height ||
           board.cells[ny][nx] > 0))
      {
        continue;
      }

      if (types.shapes[current_piece.type][current_piece.rotation][y][x] > 0)
      {
        draw_cell_outline(
            board.margin_left + ((current_piece.x + x) * board.cell_size),
            board.margin_top + ((bottom_y + y) * board.cell_size),
            types.colours[current_piece.type]);
      }
    }
  }
}

void draw_paused_screen(uint32_t tick)
{
  if (tick % 40 == 0)
  {
    blit(paused_text, 0, 0, 42, 10, 78, 108, 84, 20);
  }

  if (tick % 40 == 20)
  {
    draw_board();
    draw_next_container();

    pen(0, 0, 0);
    frect(board.margin_left + (board.width * board.cell_size), 108, 6, 16);
    pen(2, 8, 2);
    frect(board.margin_left + (board.width * board.cell_size) + 6, 108, 2, 16);
  }
}

void draw_game_over_screen(uint32_t tick)
{
  blit(game_over_text, 0, 0, 73, 10, 47, 110, 146, 20);
}

void rotate_right()
{
  int tr = (current_piece.rotation + 1) % 4;

  if (can_move(current_piece.x, current_piece.y, tr))
  {
    current_piece.rotation = tr;
    return;
  }

  for (int x = 0; x < 3; x++)
  {
    if (can_move(current_piece.x - x, current_piece.y, tr))
    {
      current_piece.x -= x;
      current_piece.rotation = tr;
      return;
    }

    if (can_move(current_piece.x + x, current_piece.y, tr))
    {
      current_piece.x += x;
      current_piece.rotation = tr;
      return;
    }
  }
}

void rotate_left()
{
  int tr = (current_piece.rotation + 3) % 4;

  for (int x = 0; x < 3; x++)
  {
    if (can_move(current_piece.x - x, current_piece.y, tr))
    {
      current_piece.x -= x;
      current_piece.rotation = tr;
      return;
    }

    if (can_move(current_piece.x + x, current_piece.y, tr))
    {
      current_piece.x += x;
      current_piece.rotation = tr;
      return;
    }
  }
}

void get_next_piece()
{
  current_piece.x = 3;
  current_piece.y = -1;
  current_piece.type = next_pieces.front();
  current_piece.rotation = 0;
  current_piece.can_hold = true;

  next_pieces.pop_front();

  if (next_pieces.size() < 3)
  {
    queue_random_pieces();
  }

  pen(0, 0, 0);
  frect(board.width * board.cell_size + board.margin_left + 10, board.margin_top + 19, 32, 16);

  pen(1, 1, 1);
  frect(board.width * board.cell_size + board.margin_left + 10, board.margin_top + 52, 32, 16);
  frect(board.width * board.cell_size + board.margin_left + 10, board.margin_top + 85, 32, 16);
}

void swap_hold_piece()
{
  if (!current_piece.can_hold)
  {
    play(voice_main, 200, 40, volume);
    return;
  }

  current_piece.can_hold = false;

  if (hold_piece == -1)
  {
    hold_piece = current_piece.type;
    get_next_piece();
    current_piece.can_hold = false;
    play(voice_main, 600, 120, volume);
    return;
  }

  int tmp = hold_piece;
  hold_piece = current_piece.type;
  current_piece.x = 3;
  current_piece.y = -1;
  current_piece.type = tmp;
  current_piece.rotation = 0;

  pen(0, 0, 0);
  frect(board.margin_left - 44, board.margin_top + 19, 32, 16);
  play(voice_main, 600, 120, volume);
}

void init()
{
  draw_background();

  queue_random_pieces();
  init_title_screen();
  init_paused_text();
  init_game_over_text();
}

void update(uint32_t internal_tick)
{
  uint32_t prev_score = score;

  if (state != states::PAUSED)
  {
    tick++;
  }

  if (state == states::GAME_OVER)
  {
    if (pressed(A))
    {
      state = states::TITLE;

      score = 0;
      lines_cleared = 0;
      level = 1;

      next_pieces.clear();
      queue_random_pieces();
      hold_piece = -1;

      for (int y = 0; y < board.height; y++)
      {
        for (int x = 0; x < board.width; x++)
        {
          board.cells[y][x] = 0;
        }
      }

      init_title_screen();
    }

    return;
  }

  if (state == states::TITLE)
  {
    if (!main_theme_done && duration_delay <= 0)
    {
      play(voice_main, main_theme.frequencies[main_theme_note], main_theme.durations[main_theme_note], volume);
      duration_delay = main_theme.durations[main_theme_note] + main_theme.tempo;
      main_theme_note = main_theme_note + 1;

      if (main_theme_note > main_theme.length)
      {
        main_theme_note = 0;
        main_theme_done = true;
      }
    }
    else
    {
      duration_delay = duration_delay - stats.fps;
    }

    if (pressed(A))
    {
      state = states::PLAYING;

      draw_background();
      get_next_piece();
      draw_next_container();
      draw_next_pieces();
      draw_hold_container();
      draw_level_container();
      draw_level();
      draw_score();

      duration_delay = 0;
      main_theme_note = 0;
      main_theme_done = false;
    }

    return;
  }

  if (state == states::PAUSED)
  {
    if (pressed(Y))
    {
      state = states::PLAYING;

      pen(0, 0, 0);
      frect(board.margin_left + (board.width * board.cell_size), 108, 6, 20);
      pen(2, 8, 2);
      frect(board.margin_left + (board.width * board.cell_size) + 6, 108, 2, 20);

      draw_next_container();
      draw_next_pieces();
      draw_hold_container();
      draw_hold_piece();
    }

    return;
  }

  if (pressed(Y))
  {
    state = states::PAUSED;
    draw_board();
    draw_next_container();
    return;
  }

  if (pressed(X))
  {
    swap_hold_piece();
    draw_hold_piece();
    draw_next_pieces();
  }

  if (pressed(A))
  {
    rotate_right();
    play(voice_main, 300, 80, volume);
  }

  if (pressed(B))
  {
    rotate_left();
    play(voice_main, 400, 80, volume);
  }

  if (button(LEFT) && lock_delay > 0)
  {
    if (pressed_time_x == 0 || (pressed_time_x > 10 && pressed_time_x % 4 == 0))
    {
      if (can_move(current_piece.x - 1, current_piece.y, current_piece.rotation))
      {
        current_piece.x--;
        play(voice_main, 300, 40, volume);
      }
      else
      {
        play(voice_main, 200, 40, volume);
      }
    }
  }

  if (button(RIGHT) && lock_delay > 0)
  {
    if (pressed_time_x == 0 || (pressed_time_x > 10 && pressed_time_x % 4 == 0))
    {
      if (can_move(current_piece.x + 1, current_piece.y, current_piece.rotation))
      {
        current_piece.x++;
        play(voice_main, 300, 40, volume);
      }
      else
      {
        play(voice_main, 200, 40, volume);
      }
    }
  }

  if (button(LEFT) || button(RIGHT))
  {
    pressed_time_x = pressed_time_x + 1;
  }
  else
  {
    pressed_time_x = 0;
  }

  if (button(DOWN))
  {
    if (tick % 2 == 0)
    {
      if (can_move(current_piece.x, current_piece.y + 1, current_piece.rotation))
      {
        current_piece.y++;
        score += 1;
        play(voice_main, 400, 40, volume);
      }
    }
  }

  if (pressed(UP))
  {
    char prev_y = current_piece.y;

    while (can_move(current_piece.x, current_piece.y + 1, current_piece.rotation))
    {
      current_piece.y++;
      score += 2;
    }

    lock_delay = 0;

    if (prev_y != current_piece.y)
    {
      play(voice_main, 500, 100, volume);
    }
  }

  is_grounded = (!can_move(current_piece.x, current_piece.y + 1, current_piece.rotation));

  if (is_grounded && button(DOWN) && lock_delay > 0)
  {
    lock_delay = lock_delay - 1;
  }

  if (!is_grounded || lock_delay > 0)
  {
    lock_delay = lock_delay - 1;
  }

  if (tick % (40 - ((40 / 15) * level)) == 0 || level == 15)
  {
    if (!is_grounded)
    {
      current_piece.y++;
    }
    else if (lock_delay <= 0)
    {
      for (int x = 0; x < 4; x++)
      {
        for (int y = 0; y < 4; y++)
        {
          int nx = current_piece.x + x;
          int ny = current_piece.y + y;

          if (types.shapes[current_piece.type][current_piece.rotation][y][x] > 0)
          {
            board.cells[ny][nx] = current_piece.type + 1;
          }
        }
      }

      draw_current_piece();

      if (current_piece.y == -1)
      {
        state = states::GAME_OVER;
        return;
      }

      get_next_piece();
      draw_next_pieces();

      int rows_full = 0;

      for (int y = board.height - 1; y >= 0; y--)
      {
        bool full = true;
        bool empty = true;

        for (int x = 0; x < board.width; x++)
        {
          if (board.cells[y][x] == 0)
          {
            full = false;
          }
          else
          {
            empty = false;
          }

          if (!full && !empty)
          {
            break;
          }
        }

        // You cannot have populated rows after an empty one, so skip checking.
        if (empty)
        {
          break;
        }

        if (full)
        {
          rows_full++;

          for (int yy = y; yy > 0; yy--)
          {
            for (int x = 0; x < board.width; x++)
            {
              board.cells[yy][x] = board.cells[yy - 1][x];
            }
          }

          for (int x = 0; x < board.width; x++)
          {
            board.cells[0][x] = 0;
          }

          // More than 4 full rows can't happen, so let's skip checking the rest.
          if (rows_full == 4)
          {
            break;
          }

          y++;
        }
      }

      lines_cleared = lines_cleared + rows_full;
      uint8_t last_level = level;
      level = (lines_cleared / 5) + 1;

      if (level > 15)
      {
        level = 15;
      }

      if (level != last_level)
      {
        draw_level();
      }

      switch (rows_full)
      {
      case 4:
        score += 800 * level;
        break;
      case 3:
        score += 500 * level;
        break;
      case 2:
        score += 300 * level;
        break;
      case 1:
        score += 100 * level;
        break;
      }
    }
  }

  if (score != prev_score)
  {
    draw_score();
    play(voice_main, 1000, 80, volume);
  }
}

void draw(uint32_t internal_tick)
{
  if (state == states::PLAYING)
  {
    draw_board();
    draw_pieces();

    if (!is_grounded)
    {
      current_y_offset = (((tick) % (stats.fps - ((stats.fps / 15) * level)) / (float)stats.fps)) * board.cell_size;
      draw_current_piece_outline();
    }
    else
    {
      current_y_offset = 0;
    }

    draw_current_piece();
  }

  if (state == states::PAUSED)
  {
    draw_paused_screen(internal_tick);
  }

  if (state == states::GAME_OVER)
  {
    draw_game_over_screen(tick);
  }

  // DEBUG
  // font();
  // pen();
  // frect(225, 200, 20, 40);
  // pen(0, 0, 0);
  // text(str((uint32_t)lines_cleared), 226, 201);
  // text(str((uint32_t)lock_delay), 226, 211);
  // text(str(stats.idle), 226, 221);
  // text(str(stats.fps), 226, 231);
}
