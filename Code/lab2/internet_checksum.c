#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXBUFFER 64

typedef struct {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;

  uint16_t identifier;
  uint16_t seq_num;

  char data[56]; // lab asks for 56 bytes
} ICMP_Message;

typedef struct {
  uint8_t version_ihl;
  uint8_t type_of_service;
  uint16_t total_length;

  uint16_t identification;
  uint16_t flags_fragment_offset;

  uint8_t ttl;
  uint8_t protocol;
  uint16_t header_checksum;

  uint32_t source_address;

  uint32_t destination_address;
  
  ICMP_Message message;
} IP_Packet;

void initIPPacket(IP_Packet* header) {
  memset(header, 0, sizeof(*header));

  header->version_ihl = (4 << 4) | 5;
  header->type_of_service = 0;
  header->total_length = sizeof(IP_Packet);

  header->identification = (uint16_t)getpid();
  header->flags_fragment_offset = 0 << 13;

  header->ttl = 64;
  header->protocol = IPPROTO_ICMP;
  header->header_checksum = 0;
}

void initICMPMessage(ICMP_Message* message) {
  memset(message, 0, sizeof(*message));

  message->type = 8;
  message->code = 0;

  message->identifier = (uint16_t)getpid();
  message->seq_num = 0;
  message->checksum = 0;
}
	
uint16_t calculate_internet_checksum(const uint8_t *data, size_t len)
{
  uint32_t checksum = 0;

  while (len >= 2) {
    checksum += ((uint16_t)data[0] << 8) | data[1];

    data += 2;
    len -= 2;
  }

  // If there's an odd byte, pad it with zero
  if (len == 1) {
    checksum += (uint16_t)data[0] << 8;
  }

  // Fold carries back into the lower 16 bits
  while (checksum >> 16) {
    checksum = (checksum & 0xFFFF) + (checksum >> 16);
  }

  // One's complement
  return (uint16_t)~checksum;
}

int main(int argc, char *argv[]) {
  if (argc == 1 || argc > 5) {
    printf("In correct number of arguments");
    return EXIT_FAILURE;
  }
  char *dest_ip = argv[1];
  int interval = 1;
  if (argc == 4) { interval = atoi(argv[3]); }

  setvbuf( stdout, NULL, _IONBF, 0);

  // Host address rigmarole
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET; // IPv4

  
  struct addrinfo hints;
  struct addrinfo *results; 
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // IPv4
  if (getaddrinfo(dest_ip, NULL, &hints, &results) != 0) {
    printf("Hostname error\n");
    return EXIT_FAILURE;
  }
  
  addr.sin_addr = ((struct sockaddr_in *)(results->ai_addr))->sin_addr;

  uint32_t dest_addr = addr.sin_addr.s_addr;
  
  // Socket construction
  int raw_socket;
  raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (raw_socket == -1) { perror( "socket() failed" ); return EXIT_FAILURE; }
  printf("SERVER: created raw socket endpoint on descriptor %d\n", raw_socket);

  struct timeval timeout = {.tv_sec = 2, .tv_usec = 0};
  if (setsockopt(raw_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
    printf("Could not set socket operation\n");
    close(raw_socket);
    return EXIT_FAILURE;
  }
  
  while (1) {
    char buffer[MAXBUFFER + 1];
    memset(buffer, 0, MAXBUFFER + 1);    

    ICMP_Message outgoing_message;
    IP_Packet outgoing_packet;

    initICMPMessage(&outgoing_message);
    initIPPacket(&outgoing_packet);

    outgoing_message.identifier = htons(outgoing_message.identifier);
    outgoing_message.seq_num = htons(outgoing_message.seq_num);
    if (argc == 3) { strncpy(outgoing_message.data, argv[2], 4); }
    outgoing_message.checksum = (uint16_t) (htons(calculate_internet_checksum((uint8_t *)&outgoing_message, sizeof(outgoing_message))));

    outgoing_packet.message = outgoing_message;

    outgoing_packet.total_length = htons(outgoing_packet.total_length);
    outgoing_packet.identification = htons(outgoing_packet.identification);
    outgoing_packet.flags_fragment_offset = htons(outgoing_packet.flags_fragment_offset);

    outgoing_packet.source_address = 0;
    outgoing_packet.destination_address = dest_addr;

    outgoing_packet.header_checksum = htons(calculate_internet_checksum((uint8_t *)&outgoing_packet, 20));

    // Packet Sending
    printf("ICMPv4 ping (interval %d seconds); host %s\n", interval, inet_ntoa(addr.sin_addr));
    if (sendto(raw_socket, buffer, MAXBUFFER + 1, 0, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
      printf("Failed to send to host\n");
      close(raw_socket);
      return EXIT_FAILURE;
    }
    
    // Packet receiving
  }
  

  return 0;
}
