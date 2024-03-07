#include "jsmn.h"
#include "json.h"
#include <stdlib.h>
#include <string.h>

struct parser_data
{
    void (*string_handler)(const char *json, const jsmntok_t *t, const size_t count, struct json_data *data);
    struct json_data data;
};

static int dump_json(const char *json, const jsmntok_t *t, const size_t count, struct parser_data *data)
{
    if (count == 0)
        return 0;

    if (t->type == JSMN_PRIMITIVE)
    {
        return 1;
    }
    else if (t->type == JSMN_STRING)
    {
        if (data->string_handler)
            data->string_handler(json, t, count, &data->data);
        return 1;
    }
    else if (t->type == JSMN_OBJECT)
    {
        int j = 0;
        for (int i = 0; i < t->size; i++)
        {
            const jsmntok_t *key = t + 1 + j;
            j += dump_json(json, key, count - j, data);
            if (key->size > 0)
                j += dump_json(json, t + 1 + j, count - j, data);
        }
        return j + 1;
    }
    else if (t->type == JSMN_ARRAY)
    {
        data->data.array_depth++;
        int j = 0;
        for (int i = 0; i < t->size; i++)
        {
            data->data.array_index = i;
            j += dump_json(json, t + 1 + j, count - j, data);
        }
        data->data.array_depth--;
        return j + 1;
    }
    return 0;
}

bool json_parse(const char *json, void (*string_handler)(const char *json, const jsmntok_t *t, const size_t count, struct json_data *data), void *user_data)
{
    const int json_len = strlen(json);
    jsmn_parser p;
    jsmn_init(&p);
    const int count = jsmn_parse(&p, json, json_len, NULL, 0);
    if (count < 1)
        return false;
    jsmntok_t *t = malloc(sizeof(jsmntok_t) * count);
    jsmn_init(&p);
    jsmn_parse(&p, json, json_len, t, count);
    struct parser_data data;
    data.string_handler = string_handler;
    data.data.array_depth = 0;
    data.data.array_index = 0;
    data.data.data = user_data;
    dump_json(json, t, count, &data);
    free(t);
    return true;
}

bool is_json_key(const char *json, const jsmntok_t *tok, const char *key)
{
    const size_t length = tok->end - tok->start;
    if (strlen(key) != length)
        return false;
    if (strncmp(json + tok->start, key, length) != 0)
        return false;
    return true;
}