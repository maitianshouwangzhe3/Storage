#ifndef __STORAGE_GRAMMAR_PARSER_H__
#define __STORAGE_GRAMMAR_PARSER_H__
#include "public.h"
#include "storage_read_message.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <pthread.h>
#include <stdio.h>

storage_message* storage_message_init(uint32_t fd);

list_key_value* storage_message_kv_init(const char* key, const char* value);

void storage_message_destruction(int fd);

void storage_grammar_parser(uint32_t fd);

#endif