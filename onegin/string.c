/** @file string.c */
#include <malloc.h>
#include "string.h"


/**
 * Allocate memory for the string and set its length to zero
 * @param string pointer to string_t object
 * @param size number of allocated bytes
 * @return true if the allocation was successful, false otherwise
 */
bool allocate_string(string_t* string, size_t size) {
  string->begin = (char*) calloc(size, 1);
  if (!string->begin) {
    return false;
  }
  string->end = string->begin;
  return true;
}


/**
 * Deallocate storage used by the string
 * @param string the string
 */
void deallocate_string(string_t* string) {
  free(string->begin);
}


/**
 * Length of a string
 * @param string the string
 * @return length of the string
 */
size_t string_length(const string_t* string) {
  return string->end - string->begin;
}

/**
 * Resize the string
 * @param string the string
 * @param length new length of the string
 * @note the string must have enough allocated memory
 */
void set_string_length(string_t* string, size_t length) {
  string->end = string->begin + length;
}