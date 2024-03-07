#ifndef _HTTPS_CLIENT_H_
#define _HTTPS_CLIENT_H_
#include <stddef.h>

char *https_get_request(const char *server, const char *path, size_t *len);

#endif