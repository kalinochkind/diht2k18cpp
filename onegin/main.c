/** @file main.c */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <error.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "string.h"
#include "lines.h"


bool read_file(const char* filename, string_t* out);

bool read_file_lines(const char* filename, string_t* buf, lines_t* lines);

void write_original_file(const string_t* buf);

void write_sorted_file(const char* filename, int compare(const void*, const void*), lines_t* lines, string_t* temp_buf);

int compare_strings_forward(const void* lhs, const void* rhs);

int compare_strings_backward(const void* lhs, const void* rhs);


int main(int argc, char** argv) {
  if (argc < 2) {
    fputs("Filename required\n", stderr);
    return 1;
  }
  string_t buf = {NULL, NULL};
  lines_t lines = {NULL, NULL};
  if (!read_file_lines(argv[1], &buf, &lines)) {
    return 1;
  }

  string_t temp_buf = {NULL, NULL};
  if (!allocate_string(&temp_buf, string_length(&buf))) {
    error(0, errno, "temp_buf");
    deallocate_lines(&lines);
    return 1;
  }

  write_sorted_file("forward.txt", compare_strings_forward, &lines, &temp_buf);
  write_sorted_file("backward.txt", compare_strings_backward, &lines, &temp_buf);
  write_original_file(&buf);

  deallocate_string(&temp_buf);
  deallocate_lines(&lines);
  deallocate_string(&buf);
  return 0;
}


#define OUTPUT_FILE_MODE 0644


/**
 * Gets file size
 * @param fd file descriptor
 * @return file size or -1 in case of error
 */
ssize_t get_file_size(int fd) {
  struct stat statbuf = {};
  if (fstat(fd, &statbuf) == -1) {
    return -1;
  }
  return statbuf.st_size;
}


bool read_file_by_descriptor(int fd, string_t* out) {
  ssize_t file_size = get_file_size(fd);
  if (file_size == -1) {
    return false;
  }
  if (!allocate_string(out, (size_t) file_size + 1)) {
    return false;
  }
  ssize_t read_size = read(fd, out->begin, (size_t) file_size);
  if (read_size == -1) {
    deallocate_string(out);
    return false;
  }
  assert(read_size <= file_size);
  if (read_size == 0 || out->begin[read_size - 1] != '\n') {
    out->begin[read_size++] = '\n';
  }
  set_string_length(out, (size_t) read_size);
  return true;
}

/**
 * Read the file into a string
 * @param filename file name
 * @param out pointer to string
 * @return true if the operation was successful, false otherwise
 * @note if the file does not end with a newline, it is added automatically
 */
bool read_file(const char* filename, string_t* out) {
  int fd = open(filename, 0);
  if (fd == -1) {
    return false;
  }
  bool result = read_file_by_descriptor(fd, out);
  close(fd);
  return result;
}


bool read_file_lines(const char* filename, string_t* buf, lines_t* lines) {
  if (!read_file(filename, buf)) {
    error(0, errno, "%s", filename);
    return false;
  }
  if (!get_lines(buf, lines)) {
    error(0, errno, "get_lines");
    deallocate_string(buf);
    return false;
  }
  return true;
}


bool write_file_by_descriptor(int fd, const string_t* string) {
  ssize_t fsize = write(fd, string->begin, string_length(string));
  return fsize != -1;
}

/**
 * Save the string into a file
 * @param filename file name
 * @param string the string
 * @return true if the operation was successful, false otherwise
 */
bool write_file(const char* filename, const string_t* string) {
  int fd = creat(filename, OUTPUT_FILE_MODE);
  if (fd == -1) {
    return false;
  }
  bool result = write_file_by_descriptor(fd, string);
  close(fd);
  return result;
}


/**
 * Check whether the character should be ignored when sorting
 * @param c the character
 * @return true if the character should be ignored, false otherwise
 */
bool should_skip(int c) {
  return isblank(c) || ispunct(c);
}

/**
 * Print the warning about bad file encoding
 */
void invalid_encoding_warning() {
  static bool printed = false;
  if (!printed) {
    printed = true;
    fputs("WARNING: your file is not in utf-8, correct sorting is not guaranteed\n", stderr);
  }
}


#define IS_UTF8_CONTINUATION_BYTE(c) ((c) >> 6u == 0b10)

/**
 * Restores an utf-8 code point from a pointer to some of its characters
 * and rewinds the pointer to the beginning of that code point
 * @param c pointer to a character
 * @param limit the beginning of the string
 * @return code point, if it could be extracted , 0 otherwise
 * @note prints a warning if the code point could not be extracted
 * @note returning non-zero value does not guarantee that the code point was valid utf-8
 */
uint32_t read_utf8_codepoint_backward(const unsigned char** c, const unsigned char* limit) {
  uint32_t res = 0;
  for (unsigned shift = 0;; shift += 8, --*c) {
    if (*c < limit || shift >= 32) {
      invalid_encoding_warning();
      return 0;
    }
    res |= **c << shift;
    if (!IS_UTF8_CONTINUATION_BYTE(**c)) {
      return res;
    }
  }
}

/**
 * String comparator for sorting forward
 */
int compare_strings_forward(const void* lhs, const void* rhs) {
  const unsigned char* char_a  = (const unsigned char*) ((line_t*) lhs)->first;
  const unsigned char* char_b  = (const unsigned char*) ((line_t*) rhs)->first;
  const unsigned char* a_limit = (const unsigned char*) ((line_t*) lhs)->last;
  const unsigned char* b_limit = (const unsigned char*) ((line_t*) rhs)->last;

  for (;; ++char_a, ++char_b) {
    while (char_a <= a_limit && should_skip(*char_a)) {
      ++char_a;
    }

    while (char_b <= b_limit && should_skip(*char_b)) {
      ++char_b;
    }

    if (char_a > a_limit || char_b > b_limit) {
      return (char_b > b_limit) - (char_a > a_limit);
    }

    if (*char_a != *char_b) {
      return (int) *char_a - (int) *char_b;
    }
  }
}

/**
 * String comparator for sorting backward
 */
int compare_strings_backward(const void* lhs, const void* rhs) {
  const unsigned char* char_a  = (const unsigned char*) ((line_t*) lhs)->last;
  const unsigned char* char_b  = (const unsigned char*) ((line_t*) rhs)->last;
  const unsigned char* a_limit = (const unsigned char*) ((line_t*) lhs)->first;
  const unsigned char* b_limit = (const unsigned char*) ((line_t*) rhs)->first;

  for (;; --char_a, --char_b) {
    while (char_a >= a_limit && should_skip(*char_a)) {
      --char_a;
    }

    while (char_b >= b_limit && should_skip(*char_b)) {
      --char_b;
    }

    if (char_a < a_limit || char_b < b_limit) {
      return (char_b < b_limit) - (char_a < a_limit);
    }

    uint32_t codepoint_a = read_utf8_codepoint_backward(&char_a, a_limit);
    uint32_t codepoint_b = read_utf8_codepoint_backward(&char_b, b_limit);
    if (codepoint_a != codepoint_b) {
      return codepoint_a - codepoint_b;
    }
  }
}


/**
 * Create original.txt
 * @param buf file contents
 */
void write_original_file(const string_t* buf) {
  if (!write_file("original.txt", buf)) {
    error(0, errno, "original.txt");
  }
}

/**
 * Create forward.txt and backward.txt
 * @param filename file name
 * @param compare comparator for sorting lines
 * @param lines line array
 * @param temp_buf buffer for storing generated file contents
 */
void
write_sorted_file(const char* filename, int compare(const void*, const void*), lines_t* lines, string_t* temp_buf) {
  qsort(lines->begin, lines->end - lines->begin, sizeof(line_t), compare);
  lines_to_text(lines, temp_buf);
  if (!write_file(filename, temp_buf)) {
    error(0, errno, "%s", filename);
  }
}

