/* ntp-request.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
  /* write an 48-byte (12-word) NTP request to "ntp-request.dat" */

  /* Mills, et al.                Standards Track                   [Page 18]
   *
   * RFC 5905                   NTPv4 Specification                 June 2010
   * 
   * 
   *       0                   1                   2                   3
   *       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *      |LI | VN  |Mode |    Stratum     |     Poll      |  Precision   |
   *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *       etc.
   *
   *
   *      Simple request:
   *
   *       0                   1                   2                   3
   *       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *      |0 0|1 0 0|0 1 0|0 0 0 0 0 0 0 0|0 0 0 0 0 0 0 0|0 0 0 0 0 0 0 0|
   *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *
   *       0x22 0x00 ... 0x00 (all zero bytes)
   *
   *  Create the ntp-request.dat file:
   *
   *      bash$ gcc -Wall ntp-request.c
   *      bash$ ./a.out
   *      bash$ ls -l ntp-request.dat
   *      ... (check that the file exists and is 48 bytes in size)
   *
   *  Confirm file contents using hexdump (hd):
   *
   *      bash$ hd -v ntp-request.dat
   *      00000000  22 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |"...............|
   *      00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
   *      00000020  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
   *      00000030
   *
   *      bash$ netcat -u -w1 129.6.15.28 123 < ntp-request.dat > ntp-response.dat
   *
   *      bash$ hd -v ntp-response.dat
   *      00000000  1c 01 0d e3 00 00 00 10  00 00 00 20 4e 49 53 54  |........... NIST|
   *      00000010  ee 4e b7 80 00 00 00 00  00 00 00 00 00 00 00 00  |.N..............|
   *      00000020  ee 4e b7 f3 cf 97 ca d1  ee 4e b7 f3 cf 97 e0 46  |.N.......N.....F|
   *      00000030
   *      ^^^
   *      from this, the second line (second word), is ERA offset in seconds, i.e., 0xee4eb780
   *
   *      bash$ bc     <== convert 0xee4eb780 to base 10
   *      ibase=16
   *      EE4EB780
   *      3998136192
   *      quit
   *
   *      bash$ date -d @3998136192
   *      Mon Sep 10 01:23:12 PM EDT 2096        <=== whoops, this is the "wrong" era...
   */

  char * request = calloc( 48, sizeof( char ) );
  if ( request == NULL ) { perror( "calloc() failed" ); return EXIT_FAILURE; }

  *request = 0x22;

  int fd = open( "ntp-request.dat", O_WRONLY | O_CREAT | O_TRUNC, 0660 );
  if ( fd == -1 ) { perror( "open() failed" ); return EXIT_FAILURE; }

  int rc = write( fd, request, 48 );
  if ( rc == -1 ) { perror( "write() failed" ); return EXIT_FAILURE; }
  if ( rc != 48 ) { fprintf( stderr, "write() incomplete...?\n" ); return EXIT_FAILURE; }

  close( fd );
  free( request );

  return EXIT_SUCCESS;
}











