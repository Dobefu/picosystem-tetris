#include "picosystem.hpp"

using namespace picosystem;

#ifndef _TETRIS_H
#define _TETRIS_H

void init();
void update(uint32_t tick);
void draw(uint32_t tick);

struct Board
{
  char width = 10;
  char height = 24;
  char visual_height = 20;
  char cell_size = 8;
  char margin_left = 50;
  char margin_top = 14;
  char cells[24][10] = {0};
};

struct Piece
{
  int x = 0;
  int y = 0;
  int type = 0;
  int rotation = 0;
  bool can_hold = true;
};

struct Types
{
  char count = 7;

  color_t colours[7] = {
      rgb(0, 14, 14),
      rgb(0, 0, 15),
      rgb(14, 10, 0),
      rgb(14, 14, 0),
      rgb(0, 14, 0),
      rgb(10, 0, 14),
      rgb(14, 0, 0)};

  char shapes[7][4][4][4] = {
      {{{0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},
       {{0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0}},
       {{0, 0, 0, 0},
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0}},
       {{0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0}}},
      {{{0, 0, 0, 0},
        {0, 2, 0, 0},
        {0, 2, 2, 2},
        {0, 0, 0, 0}},
       {{0, 0, 0, 0},
        {0, 0, 2, 2},
        {0, 0, 2, 0},
        {0, 0, 2, 0}},
       {{0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 2, 2, 2},
        {0, 0, 0, 2}},
       {{0, 0, 0, 0},
        {0, 0, 2, 0},
        {0, 0, 2, 0},
        {0, 2, 2, 0}}},
      {{{0, 0, 0, 0},
        {0, 0, 0, 3},
        {0, 3, 3, 3},
        {0, 0, 0, 0}},
       {{0, 0, 0, 0},
        {0, 0, 3, 0},
        {0, 0, 3, 0},
        {0, 0, 3, 3}},
       {{0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 3, 3, 3},
        {0, 3, 0, 0}},
       {{0, 0, 0, 0},
        {0, 3, 3, 0},
        {0, 0, 3, 0},
        {0, 0, 3, 0}}},
      {{{0, 0, 0, 0},
        {0, 4, 4, 0},
        {0, 4, 4, 0},
        {0, 0, 0, 0}},
       {{0, 0, 0, 0},
        {0, 4, 4, 0},
        {0, 4, 4, 0},
        {0, 0, 0, 0}},
       {{0, 0, 0, 0},
        {0, 4, 4, 0},
        {0, 4, 4, 0},
        {0, 0, 0, 0}},
       {{0, 0, 0, 0},
        {0, 4, 4, 0},
        {0, 4, 4, 0},
        {0, 0, 0, 0}}},
      {{{0, 0, 0, 0},
        {0, 0, 5, 5},
        {0, 5, 5, 0},
        {0, 0, 0, 0}},
       {{0, 0, 0, 0},
        {0, 0, 5, 0},
        {0, 0, 5, 5},
        {0, 0, 0, 5}},
       {{0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 5, 5},
        {0, 5, 5, 0}},
       {{0, 0, 0, 0},
        {0, 5, 0, 0},
        {0, 5, 5, 0},
        {0, 0, 5, 0}}},
      {{{0, 0, 0, 0},
        {0, 0, 6, 0},
        {0, 6, 6, 6},
        {0, 0, 0, 0}},
       {{0, 0, 0, 0},
        {0, 0, 6, 0},
        {0, 0, 6, 6},
        {0, 0, 6, 0}},
       {{0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 6, 6, 6},
        {0, 0, 6, 0}},
       {{0, 0, 0, 0},
        {0, 0, 6, 0},
        {0, 6, 6, 0},
        {0, 0, 6, 0}}},
      {{{0, 0, 0, 0},
        {0, 7, 7, 0},
        {0, 0, 7, 7},
        {0, 0, 0, 0}},
       {{0, 0, 0, 0},
        {0, 0, 0, 7},
        {0, 0, 7, 7},
        {0, 0, 7, 0}},
       {{0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 7, 7, 0},
        {0, 0, 7, 7}},
       {{0, 0, 0, 0},
        {0, 0, 7, 0},
        {0, 7, 7, 0},
        {0, 7, 0, 0}}}};
};

enum states
{
  TITLE,
  PLAYING,
  PAUSED,
  GAME_OVER
};

// Song data courtesy of https://www.jk-quantized.com/blog/2013/11/22/tetris-theme-song-using-processing.
struct MainTheme
{
  char length = 40;
  char tempo = 100;
  float frequencies[40] = {
      659.25511,
      493.8833,
      523.25113,
      587.32954,
      523.25113,
      493.8833,
      440.0,
      440.0,
      523.25113,
      659.25511,
      587.32954,
      523.25113,
      493.8833,
      523.25113,
      587.32954,
      659.25511,
      523.25113,
      440.0,
      440.0,
      440.0,
      493.8833,
      523.25113,
      587.32954,
      698.45646,
      880.0,
      783.99087,
      698.45646,
      659.25511,
      523.25113,
      659.25511,
      587.32954,
      523.25113,
      493.8833,
      493.8833,
      523.25113,
      587.32954,
      659.25511,
      523.25113,
      440.0,
      440.0};
  float durations[40] = {
      406.250,
      203.125,
      203.125,
      406.250,
      203.125,
      203.125,
      406.250,
      203.125,
      203.125,
      406.250,
      203.125,
      203.125,
      609.375,
      203.125,
      406.250,
      406.250,
      406.250,
      406.250,
      203.125,
      203.125,
      203.125,
      203.125,
      609.375,
      203.125,
      406.250,
      203.125,
      203.125,
      609.375,
      203.125,
      406.250,
      203.125,
      203.125,
      406.250,
      203.125,
      203.125,
      406.250,
      406.250,
      406.250,
      406.250,
      406.250,
  };
};

#endif
