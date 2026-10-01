#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>

// Standard RFC 1071 checksum calculation
unsigned short calculate_checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    for (sum = 0; len > 1; len -= 2) sum += *buf++;
    if (len == 1) sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <server-ip-or-hostname>\n", argv[0]);
        return 1;
    }

    // 1. Create a raw ICMP socket (requires root)
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Socket creation failed (are you root?)");
        return 1;
    }

    // 2. Resolve destination address
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    
    struct hostent *host = gethostbyname(argv[1]);
    if (host) {
        memcpy(&dest_addr.sin_addr, host->h_addr_list[0], host->h_length);
    } else {
        dest_addr.sin_addr.s_addr = inet_addr(argv[1]);
    }

    // 3. Prepare ICMP Echo Request packet
    char packet[64];
    memset(packet, 0, sizeof(packet));
    struct icmphdr *icmp = (struct icmphdr *)packet;
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = getpid();
    icmp->un.echo.sequence = 1;
    icmp->checksum = 0;
    icmp->checksum = calculate_checksum(packet, sizeof(packet));

    // 4. Send packet
    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Sendto failed");
        close(sockfd);
        return 1;
    }
    printf("ICMP Echo request sent to %s\n", argv[1]);

    // 5. Receive response
    char recv_buf[1024];
    struct sockaddr_in r_addr;
    socklen_t addr_len = sizeof(r_addr);
    
    if (recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&r_addr, &addr_len) < 0) {
        perror("Recvfrom failed");
    } else {
        printf("Received ICMP reply from %s!\n", inet_ntoa(r_addr.sin_addr));
    }

    close(sockfd);
    return 0;
}

