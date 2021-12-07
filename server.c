#include "server.h"

uint32_t delays[MAX_DELAY];

void udp_latency(char *ip_address, int port)	
{
	
	// information_preprocessing	
	
    if (sysTimestampEnable() == ERROR) {
        perror("Time stamps could not be enabled");
        return;
    };
    
	memset(delays, 0, sizeof(delays));
	sysClkRateSet(CLOCK_RATE);
	
	
	int sockd;
	struct sockaddr_in my_name, cli_name, srv_addr;
	int count;
	int addrlen;
	int i;
	
	
	// Common part for both receiver and the transmitter
	sockd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockd == -1) {
		perror("Could not open the socket...");
		exit(1);
	}
	
	my_name.sin_family = AF_INET;
	my_name.sin_addr.s_addr = INADDR_ANY;
	my_name.sin_port = htons(port);
	
	if (bind(sockd, (struct sockaddr*)&my_name, sizeof(my_name)) == -1) {
		perror("Could not bind the socket...");
		close(sockd);
		exit(2);
	}
	
	if (ip_address == NULL) {
		// Running as a receiver
		
#ifdef DEBUG_OUTPUT
		printf("Runnign as a receiver...\n");
#endif

		i = 0;
		addrlen = sizeof(cli_name);
		while (1) {			
			
			if (recvfrom(sockd, buf, MAX_BUF, 0, (struct sockaddr*)&cli_name, &addrlen) == -1) {
				continue;
			}
			if (sendto(sockd, buf, strlen(buf)+1, 0, (struct sockaddr*)&cli_name, sizeof(cli_name)) == -1) {
				perror("Message not sent back");
			}
		}
		close(sockd);
	} else {
#ifdef DEBUG_OUTPUT
		printf("Runnign as a transmitter...\n");
#endif
		// Running as a transmitter
		srv_addr.sin_family = AF_INET;
		inet_aton(ip_address, &srv_addr.sin_addr);
		srv_addr.sin_port = htons(port);
		
		i = 0;
		while(1) {
			addrlen = sizeof(srv_addr);
			
			sendto(sockd, buf, payload_size, 0, (struct sockaddr*)&srv_addr, &addr_len);
			count = recvfrom(sockd, recv_buf, payload_size, 0, (struct sockaddr*)&srv_addr, &addr_len);
			
		}
		close(sockd);
		
		
	}
	printf("Measurement finished\n");
}
