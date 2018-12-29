/** @file lines.c */
#include <malloc.h>
#include <assert.h>
#include <sys/types.h>
#include "lines.h"


/**
 * Deallocate storage used by line array
 * @param lines the line array
 */
void deallocate_lines(lines_t* lines) {
  free(lines->begin);
}

/**
 * Count newline characters in the string
 * @param string the string
 * @return number of newline characters
 */
static size_t count_lines(const string_t* string) {
  size_t lines = 0;
  for (const char* c = string->begin; c != string->end; ++c) {
    if (*c == '\n') {
      ++lines;
    }
  }
  return lines;
}


/**
 * Split the string into line array
 * @param string the string
 * @param lines pointer to the line array
 * @return true if the line array was successfully allocated and filled, false otherwise
 */
bool get_lines(const string_t* string, lines_t* lines) {
  size_t line_count = count_lines(string);
  lines->begin = malloc((line_count + 1) * sizeof(line_t));
  if (!lines->begin) {
    return false;
  }
  line_t* linebuf = lines->begin;
  linebuf->first = string->begin;
  for (char* c = string->begin; c != string->end; ++c) {
    if (*c == '\n') {
      if (linebuf->first < c) {
        // if the line is not empty, save it
        linebuf->last = c - 1;
        ++linebuf;
        assert(linebuf - lines->begin <= (ssize_t) line_count);
      }
      // if the line is empty, just ignore it
      linebuf->first = c + 1;
    }
  }
  lines->end = linebuf;
  return true;
}

/**
 * Converts line array into a string
 * @param lines line array
 * @param out a string
 * @note output string must have enough memory allocated
 */
void lines_to_text(const lines_t* lines, string_t* out) {
  char* buf = out->begin;
  for (line_t* line = lines->begin; line != lines->end; ++line) {
    for (const char* c = line->first; c <= line->last; ++c) {
      *buf++ = *c;
    }
    *buf++ = '\n';
  }
  out->end = buf;
}