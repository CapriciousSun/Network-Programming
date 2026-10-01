struct ipheader {

  unsigned int version[4];
  unsigned int IHL[4];
  unsigned int service[8];
  unsigned int fragmentLength[16];
  unsigned int id[16];
  unsigned int flag[4];
  unsigned int offset[12];
  unsigned int TTL[8];
  unsigned int protocol[8];
  unsigned int checksum[16];
  unsigned int source[32];
  unsigned int destination[32];
  unsigned int options[32];

};

unsigned int* calculateChecksum(unsigned int* fragmentLength, unsigned int* id) {

  unsigned int checksum[16];
  unsigned int sum = ~&fragmentLength + ~&id;
  
}

int main() {
  
  struct ipheader example;
  example.version[0] = 0b0100;
  example.fragmentLength[0] = 0b1001000000;
  example.id[0] = 0b1001000100;

  example.checksum[0] = *calculateChecksum(example.fragmentLength, example.id);

}
