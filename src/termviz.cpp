#include "imbibe.h"

#include "termviz.h"


uint8_t const termviz::fade_masks[termviz::fade_steps] = {
  // 0x00, 0x04, 0x44, 0x45, 0x55, 0x57, 0x77, 0x7F, 0xFF
  0x00, 0x04, 0x04, 0x44, 0x44, 0x45, 0x45, 0x55,
  0x55, 0x57, 0x57, 0x77, 0x77, 0x7F, 0x7F, 0xFF
};


// Watch out, these are in octal notation
uint8_t const termviz::fade_seqs[termviz::fade_steps][16] = {
  // {000, 000, 000, 000, 000, 000, 000, 000, 000},
  // {000, 000, 000, 000, 001, 001, 001, 001, 001},
  // {000, 000, 001, 010, 010, 010, 010, 002, 002},
  // {000, 001, 001, 010, 010, 010, 002, 003, 003},
  // {000, 000, 000, 000, 004, 004, 004, 004, 004},
  // {000, 000, 000, 004, 001, 005, 005, 005, 005},
  // {000, 004, 001, 010, 010, 010, 010, 006, 006},
  // {000, 001, 001, 010, 010, 010, 011, 006, 007},
  // {000, 000, 000, 001, 001, 001, 010, 010, 010},
  // {000, 000, 001, 001, 010, 010, 010, 011, 011},
  // {000, 001, 010, 010, 002, 002, 003, 012, 012},
  // {000, 001, 010, 010, 002, 003, 007, 012, 013},
  // {000, 000, 004, 005, 005, 005, 010, 014, 014},
  // {000, 000, 001, 005, 010, 010, 016, 015, 015},
  // {000, 001, 010, 010, 006, 006, 007, 016, 016},
  // {000, 001, 010, 010, 011, 007, 007, 016, 017},

  // {000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000},
  // {000, 000, 000, 001, 000, 000, 004, 001, 000, 000, 001, 001, 000, 000, 001, 001},
  // {000, 000, 001, 001, 000, 000, 001, 001, 000, 001, 010, 010, 004, 001, 010, 010},
  // {000, 000, 010, 010, 000, 004, 010, 010, 001, 001, 010, 010, 005, 005, 010, 010},
  // {000, 001, 010, 010, 004, 001, 010, 010, 001, 010, 002, 002, 005, 010, 006, 011},
  // {000, 001, 010, 010, 004, 005, 010, 010, 001, 010, 002, 003, 005, 010, 006, 007},
  // {000, 001, 010, 002, 004, 005, 010, 011, 010, 010, 003, 007, 010, 016, 007, 007},
  // {000, 001, 002, 003, 004, 005, 006, 006, 010, 011, 012, 012, 014, 015, 016, 016},
  // {000, 001, 002, 003, 004, 005, 006, 007, 010, 011, 012, 013, 014, 015, 016, 017},

  {000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000},
  {000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000},
  {000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000},
  {000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 001, 000, 000, 010, 010},
  {000, 000, 000, 001, 000, 000, 000, 010, 000, 001, 010, 010, 000, 001, 010, 010},
  {000, 000, 000, 010, 000, 000, 010, 010, 000, 001, 010, 010, 004, 005, 010, 010},
  {000, 000, 000, 010, 000, 000, 010, 010, 000, 001, 010, 010, 004, 005, 010, 010},
  {000, 000, 010, 010, 000, 004, 010, 010, 000, 001, 010, 010, 004, 005, 010, 010},
  {000, 001, 010, 010, 004, 001, 010, 010, 010, 010, 002, 003, 010, 010, 006, 007},
  {000, 001, 010, 010, 004, 005, 010, 010, 010, 010, 002, 003, 010, 010, 006, 007},
  {000, 001, 010, 010, 004, 005, 010, 010, 010, 010, 002, 003, 010, 010, 006, 007},
  {000, 001, 002, 002, 004, 005, 010, 010, 010, 010, 002, 003, 014, 014, 006, 007},
  {000, 001, 002, 003, 004, 005, 006, 007, 010, 011, 003, 007, 014, 015, 007, 007},
  {000, 001, 002, 003, 004, 005, 006, 007, 010, 011, 012, 013, 014, 015, 016, 017},
  {000, 001, 002, 003, 004, 005, 006, 007, 010, 011, 012, 013, 014, 015, 016, 017},
  {000, 001, 002, 003, 004, 005, 006, 007, 010, 011, 012, 013, 014, 015, 016, 017},
};

uint8_t const termviz::dissolve_masks[9][2][4] = {
  {
    {  0,   0,   0,   0},
    {  0,   0,   0,   0},
  },
  {
    {  0, 255,   0,   0},
    {  0,   0,   0,   0},
  },
  {
    {  0, 255,   0, 255},
    {  0,   0,   0,   0},
  },
  {
    {  0, 255,   0, 255},
    {255,   0,   0,   0},
  },
  {
    {  0, 255,   0, 255},
    {255,   0, 255,   0},
  },
  {
    {255, 255,   0, 255},
    {255,   0, 255,   0},
  },
  {
    {255, 255, 255, 255},
    {255,   0, 255,   0},
  },
  {
    {255, 255, 255, 255},
    {255, 255, 255,   0},
  },
  {
    {255, 255, 255, 255},
    {255, 255, 255, 255},
  },
};

