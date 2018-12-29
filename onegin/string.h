/** @file string.h */
#pragma once
#include <stddef.h>
#include <stdbool.h>

/**
 * String aware of its length
 */
typedef struct {
  char* begin; ///< First character
  char* end; ///< Character after the last
} string_t;


bool allocate_string(string_t* string, size_t size);
void deallocate_string(string_t* string);
size_t string_length(const string_t* string);
void set_string_length(string_t* string, size_t length);
