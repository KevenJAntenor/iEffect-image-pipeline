#include "utils.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Source:
// https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
int is_regular_file(const char *path) {
  struct stat path_stat;
  if (stat(path, &path_stat) < 0)
    return 0;
  return S_ISREG(path_stat.st_mode);
}

int is_dir(const char *path) {
  struct stat path_stat;
  if (stat(path, &path_stat) < 0)
    return 0;
  return S_ISDIR(path_stat.st_mode);
}

// Source:
// https://stackoverflow.com/questions/744766/how-to-compare-ends-of-strings-in-c
int ends_with(const char *str, const char *suffix) {
  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);

  return (str_len >= suffix_len) &&
         (!memcmp(str + str_len - suffix_len, suffix, suffix_len));
}
