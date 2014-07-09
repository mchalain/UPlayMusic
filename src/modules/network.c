#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <sched.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "pilot_log.h"
#include "network.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

int http_get_transaction(int fd, char *page, struct http_info *info)
{
	int ret = -1;
	int len;

	/**
	 * generate the request
	 **/
	char *wbuff;
	wbuff = malloc(1024 + 1);
	len = 0;
	sprintf(wbuff+len, "GET %s HTTP/1.0\r\n", page);
	len += strlen(page) + 15;
/*
 * 	sprintf(wbuff+len, "User-Agent: mpg123/1.12.1\r\n");
	len += 27;
	sprintf(wbuff+len, "Host: 10.1.2.9:49152\r\n");
	len += 22;
	sprintf(wbuff+len, "Accept: audio/mpeg, audio/x-mpeg, audio/mp3, audio/x-mp3, audio/mpeg3, audio/x-mpeg3, audio/mpg, audio/x-mpg, audio/x-mpegaudio, audio/mpegurl, audio/mpeg-url, audio/x-mpegurl, audio/x-scpls, audio/scpls, application/pls\r\n");
	len += 227;
*/
	sprintf(wbuff+len, "\r\n");
	len += 2;
	
	/**
	 * send the request
	 **/
	ret = write(fd, wbuff, len);
	free(wbuff);

	/**
	 * look for bytes available on the connection
	 **/
	int bytesAv = 0;
	//int ret;
	fd_set rfds;
	int maxfd;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	maxfd = fd +1;

	ret = select(maxfd, &rfds, NULL, NULL, NULL);
	if (ret > 0 && FD_ISSET(fd, &rfds))
	{
		
		ioctl (fd,FIONREAD,&bytesAv);
		ret = bytesAv;
	}

	/**
	 * allocate buffer with enought space for the header
	 **/
	char *rbuff;
	rbuff = malloc(bytesAv + 1);
	char *it = rbuff;
	memset(rbuff, 0, bytesAv + 1);
	while (ret > 0)
	{
		len = bytesAv;
		ret = recv(fd, rbuff, len, 0);
		if (ret < 0)
		{
			LOG_ERROR("read from socket error %d %s", ret, strerror(errno));
			return -1;
		}
		it[ret] = 0;
		if (strstr(rbuff, "\r\n\r\n"))
		{
			break;
		}
		it += ret;
		if (it >= rbuff + sizeof(rbuff) - len)
		{
			memcpy(rbuff, it - ret, ret);
			it = rbuff + ret;
		}			
	}
	/**
	 * parse the header
	 **/
	char *value;
	if ((value = strstr(rbuff, "CONTENT-LENGTH: ")))
	{
		sscanf(value,"CONTENT-LENGTH: %u[^\r]", &info->length);
	}
	if ((value = strstr(rbuff, "CONTENT-TYPE: ")))
	{
		sscanf(value,"CONTENT-TYPE: %99[^\r]", info->mime);
	}

	return ret;
}

int
http_get(char *uri, struct http_info *info)
{
	int fd = -1;
	int port = 0;
	char proto[10];
	char ip[100];
	char page[200];
	int err = -1;

	memset(proto, 0, 10);
	memset(ip, 0, 100);
	memset(page, 0, 100);
	port = 80;

	page[0]='/';
	if (sscanf(uri, "%9[^:]://%99[^:]:%i/%198[^\n]", proto, ip, &port, page+1) == 4) { err = 0;}
	else if (sscanf(uri, "%9[^:]://%99[^/]/%198[^\n]", proto, ip, page+1) == 3) { err = 0;}
	else if (sscanf(uri, "%9[^:]://%99[^:]:%i[^\n]", proto, ip, &port) == 3) { err = 0;}
	else if (sscanf(uri, "%9[^:]://%99[^\n]", proto, ip) == 2) { err = 0;}

	if (!err)
	{
#ifndef IPV6
		struct sockaddr_in server;
		struct in_addr myaddr;

		if((server.sin_addr.s_addr = inet_addr(ip)) == INADDR_NONE)
		return -1;

		server.sin_port = htons(port);
		server.sin_family = AF_INET;

		if((fd = socket(PF_INET, SOCK_STREAM, 6)) < 0)
		{
			LOG_ERROR("Cannot create socket: %s", strerror(errno));
			return -1;
		}
		if(connect(fd, (struct sockaddr *)&server, sizeof(server)))
			return -1;
#else
		struct addrinfo hints;

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
		hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
		hints.ai_flags = 0;
		hints.ai_protocol = 0;          /* Any protocol */
		if (strncmp(proto,"http",4))
		{
			LOG_ERROR("bad protocol type %s", proto);
			return -1;
		}
		struct addrinfo *result, *rp;
		char aport[6];
		sprintf(aport,"%u",port);

		err = getaddrinfo(ip, aport, &hints, &result);
		if (err)
			return err;

		for (rp = result; rp != NULL; rp = rp->ai_next)
		{
			int yes = 0;

			fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if (fd == -1)
				continue;
			else if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
			{
				LOG_DEBUG("error reuse opt: %s", strerror(errno));
				close(fd);
				fd = -errno;
				continue;
			}

			if (connect(fd, rp->ai_addr, rp->ai_addrlen) != -1)
				break;                  /* Success */

			close(fd);
		}
		freeaddrinfo(result);
		if (rp == NULL)
			return -1;
#endif

		http_get_transaction(fd, uri, info);

	}
	return fd;
}

#ifdef HTTP_GET_MAIN
int main(int argc, char **argv)
{
	if (argc > 1);
	{
		struct http_info info;
		int fd;
		fd = http_get(argv[1], &info);
		printf("content length = %u\n",info.length);
		printf("content type = %s\n",info.mime);
		if (fd > 0)
			close(fd);
	}
	return 0;
}
#endif
