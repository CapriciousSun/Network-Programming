/* fd-read.c */

/**
 * There are three kinds of standard IO functions; stdin, stdout, stderr. 
 * stdin and stderr both default to displaying on the CLI. 
 * stdout is either line-buffered or block-buffered.
 * stderr is unbuffered.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main() {

#if 0
  char *name = "testfile.txt"; /* immutable string */
  *(name + 2) = 'x';
#endif

#if 0
  char name[13] = "testfile.txt";
  *(name + 2) = 'x';
#endif

#if 0
  char *name[] = "testfile.txt";
  *(name + 2) = 'x';
#endif

  char *name = malloc(13);
  strcpy( name, "testfile.txt");

  int fd = open(name, O_RDONLY);

  free(name);
  if (fd == -1) {perror("open() failed"); return EXIT_FAILURE; }

  printf("fd is %d\n", fd);

  char buffer[64];
  int rc = read(fd, buffer, 16);
  buffer[rc] = '\0';
  *(buffer + rc) = '\0';
  printf("read() returned %d -- buffer \"%s\"\n", rc, buffer);

  rc = read(fd, buffer, 32);
  buffer[rc] = '\0';
  printf("read() returned %d -- buffer \"%s\"\n", rc, buffer);

  rc = read(fd, buffer, 32);
  buffer[rc] = '\0';
  printf("read() returned %d -- buffer \"%s\"\n", rc, buffer);

  close(fd);

  return EXIT_SUCCESS;

}
