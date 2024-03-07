#ifndef _JSON_H_
#define _JSON_H_
#define JSMN_HEADER
#include "jsmn.h"
#include <stdbool.h>

struct json_data
{
    int array_depth;
    int array_index;
    void *data;
};

struct parser_data
{
    void (*string_handler)(const char *json, const jsmntok_t *t, const size_t count, struct json_data *data);
    struct json_data data;
};

int get_json_tokens_needed(const char *json, const size_t len);
void parser_init(struct parser_data *data, void (*string_handler)(const char *json, const jsmntok_t *t, const size_t count, struct json_data *data), void *user_data);
int json_parse(const char *json, const jsmntok_t *t, const size_t count, struct parser_data *data);
bool is_json_key(const char *json, const jsmntok_t *tok, const char *key);

#endif