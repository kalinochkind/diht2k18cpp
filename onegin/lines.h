/** @file lines.h */
#pragma once
#include "string.h"

/**
 * Non-owning string fragment
 */
typedef struct {
  const char* first; ///< First character
  const char* last; ///< Last character
} line_t;


/**
 * Array of string fragments
 */
typedef struct {
  line_t* begin; ///< First fragment
  line_t* end; ///< Fragment after the last
} lines_t;


void deallocate_lines(lines_t* lines);
bool get_lines(const string_t* string, lines_t* lines);
void lines_to_text(const lines_t* lines, string_t* out);
