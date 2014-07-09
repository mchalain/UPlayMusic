#ifndef __NETWORK_HTTP_GET_H__
#define __NETWORK_HTTP_GET_H__
struct http_info
{
	unsigned int length;
	char mime[100];
};

extern int http_get(char *uri, struct http_info *info);

#endif
