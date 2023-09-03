#include "storage_grammar_parser.h"

std::unordered_map<int, storage_message*> message_map;
pthread_mutex_t mtx;
static std::unordered_map<std::string, type_> hash_map = {
    {"set", type_::SET},
    {"get", type_::GET},
    {"del", type_::DEL},
    {"zadd", type_::ZADD},
    {"zdel", type_::ZDEL},
    {"zget", type_::ZGET},
    {"zset", type_::ZSET},
    {"ztroy", type_::ZTROY},
    {"query", type_::QUERY},
    {"ping", type_::PING},
    {"quit", type_::QUIT}
};

storage_message* storage_message_init(uint32_t fd) {
    storage_message* message = new storage_message();
    message->fd = fd;
    message->cmd = ERROR;
    message->limit1 = 0;
    message->limit2 = 0;
    message->key = nullptr;
    message->list_kv = nullptr;
    message->time = 0;
    return message;
}

list_key_value* storage_message_kv_init(const char* key, const char* value) {
    list_key_value* kv = new list_key_value();
    kv->key = const_cast<char*>(key);
    kv->value = const_cast<char*>(value);
    kv->next = nullptr;
    return kv;
}

void storage_message_destruction(int fd) {
    auto message = message_map[fd];
    message_map.erase(fd);
    list_key_value* cur = message->list_kv;
    while(message->list_kv) {
        cur = message->list_kv->next;
        delete message->list_kv;
        message->list_kv = cur;
    }
    delete message;
    message = nullptr;
    close(fd);
}

static void grammar_parser(char* buffer, storage_message*  message) {
    size_t size = strlen(buffer);
    std::vector<std::string> pool; 
    for(int i = 0; i < size; ++i) {
        std::string str = "";
        while(i < size && ' ' != buffer[i]) {
            str += buffer[i++];
        }
        pool.emplace_back(str);
    }

    if(hash_map.find(pool[0]) != hash_map.end()) {
        message->cmd = hash_map[pool[0]];
    }
    else {
        message->cmd = ERROR;
    }
        
    switch (message->cmd)
    {
        case SET:
            {
                if(pool.size() < 3) {
                    message->cmd = ERROR;
                    break;
                }
                message->list_kv = storage_message_kv_init(pool[1].c_str(), pool[2].c_str());
            }
            break;
        case GET:
            {
                if(pool.size() < 2) {
                    message->cmd = ERROR;
                    break;
                }
                message->list_kv = storage_message_kv_init(pool[1].c_str(), nullptr);
            }
            break;
        case DEL:
            {
                if(pool.size() < 2) {
                    message->cmd = ERROR;
                    break;
                }
                message->list_kv = storage_message_kv_init(pool[1].c_str(), nullptr);
            }
            break;
        case ZADD:
            {
                if(pool.size() < 3) {
                    message->cmd = ERROR;
                    break;
                }
                for(int i = 1; i < pool.size() - 1; i += 2) {
                    if(pool[i] == "timer") {
                        continue;
                    }
                    message->list_kv = storage_message_kv_init(pool[i].c_str(), pool[i + 1].c_str());
                }                
            }
            break;
        case ZDEL:
            {
                if(pool.size() < 2) {
                    message->cmd = ERROR;
                    break;
                }
                message->list_kv = storage_message_kv_init(pool[1].c_str(), nullptr);
            }
            break;
        case ZGET:
            {
                if(pool.size() < 2) {
                    message->cmd = ERROR;
                    break;
                }
                message->list_kv = storage_message_kv_init(pool[1].c_str(), nullptr);
            } 
            break;
        case ZSET:    
            {
                if(pool.size() < 4) {
                    message->cmd = ERROR;
                    break;
                }
                message->key = const_cast<char*>(pool[1].c_str());
                message->list_kv = storage_message_kv_init(pool[2].c_str(), pool[3].c_str());
            }
            break;
        case ZTROY:
            {
                if(pool.size() < 3) {
                    message->cmd = ERROR;
                    break;
                }
                message->key = const_cast<char*>(pool[1].c_str());
                message->list_kv = storage_message_kv_init(pool[2].c_str(), nullptr);
            }
            break;
        case QUERY:
            {
                if(pool.size() < 6) {
                    message->cmd = ERROR;
                    break;
                }
                message->key = const_cast<char*>(pool[1].c_str());
                message->list_kv = storage_message_kv_init(pool[2].c_str(), pool[3].c_str());
                message->limit1 = (char*)pool[4].c_str();
                message->limit2 = (char*)pool[5].c_str();
            }
            break;
        case PING:
            {
                message->cmd = PING;
            }
            break;
    default:
        message->cmd = ERROR;
        break;
    }
}

void storage_grammar_parser(uint32_t fd) {
    storage_message* q = storage_message_init(fd);
    ssize_t ret = recv(fd, q->buf, sizeof(q->buf), 0);
    if(0 > ret)
    {
        return;
    }

    grammar_parser(q->buf, q);
    memset(q->buf, 0, sizeof(q->buf));
    pthread_mutex_lock(&mtx);
    storage_insert_read_message(q);
    message_map[fd] = q;
    pthread_mutex_unlock(&mtx);
}