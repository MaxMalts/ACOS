#define _DEFAULT_SOURCE

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <memory.h>
#include <unistd.h>


void HostNameToQueryHost(const char* hostName, char* res) {
	assert(hostName != NULL);
	assert(res != NULL);

	char* lastPoint = res;
	int cursor = 0;
	int lastLen = 0;
	while (*(hostName + cursor) != '\0') {
		if ('.' == *(hostName + cursor)) {
			*lastPoint = lastLen;
			lastLen = 0;
			lastPoint = res + cursor + 1;
		} else {
			*(res + cursor + 1) = *(hostName + cursor);
			++lastLen;
		}

		++cursor;
	}
	*lastPoint = lastLen;
	*(res + cursor + 1) = '\0';
}


uint32_t IpFromDns(const char* hostName) {
	assert(hostName != NULL);

	const char dnsAddr[] = "8.8.8.8";
	const uint16_t dnsPort = 53;
	const char queryPrefix[] = {0xAA, 0xAA, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0};
	const char queryPostfix[] = {0, 0, 1, 0, 1};

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sockfd) {
		return 0;
	}

	struct sockaddr_in sockAddr = {};
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(dnsPort);
	sockAddr.sin_addr.s_addr = inet_addr(dnsAddr);

	if (connect(sockfd, (const struct sockaddr*)(&sockAddr), sizeof(sockAddr)) == -1) {
		return 0;
	}

	char queryHost[4000] = "";
	HostNameToQueryHost(hostName, queryHost);
	int queryHostLen = strlen(queryHost);
	char buf[10000] = "";
	memcpy(buf, queryPrefix, sizeof(queryPrefix));
	memcpy(buf + sizeof(queryPrefix), queryHost, queryHostLen);
	memcpy(buf + sizeof(queryPrefix) + queryHostLen, queryPostfix, sizeof(queryPostfix));

	if (sendto(
			sockfd,
			&buf,
			sizeof(queryPrefix) + queryHostLen + sizeof(queryPostfix),
			0,
			(const struct sockaddr*)(&sockAddr),
			sizeof(sockAddr)) == -1) {
		close(sockfd);
		return 0;
	}

	ssize_t bytesRead = recv(sockfd, buf, sizeof(buf), 0);
	if (-1 == bytesRead) {
		close(sockfd);
		return 0;
	}

	close(sockfd);

	uint32_t res = *((uint32_t*)(buf + bytesRead - 4));
	return res;
}


void PrintIp(uint32_t ip) {
	assert(ip > 0);

	printf("%u.%u.%u.%u\n", *((unsigned char*)(&ip)), *((unsigned char*)(&ip) + 1),
	       *((unsigned char*)(&ip) + 2), *((unsigned char*)(&ip) + 3));
}


int main() {
	char curHostName[4000] = "";
	while (scanf("%s", curHostName) > 0) {
		uint32_t curIp = IpFromDns(curHostName);
		PrintIp(curIp);
	}

	return 0;
}