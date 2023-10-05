#ifndef __STORAGE_GRAMMAR_PARSER_H__
#define __STORAGE_GRAMMAR_PARSER_H__
#include "public.h"


storage_message* storage_message_init(uint32_t fd);

list_key_value* storage_message_kv_init(const char* key, const char* value);

void storage_message_destruction(int fd);

void storage_grammar_parser(uint32_t fd);

#endif