#include "storage_read_message.h"
#include <stdio.h>

#include "storage_hashtable.h"
#include "storage_resource.h"
#include "storage_avltree.h"
#include "storage_orderedlist.h"
#include "storage_write_message.h"
#include "storage_memory_pool.h"
#include "storage.h"
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <pthread.h>
#include <iostream>

static bool isClose = false;
pthread_t pid_s;
static read_queue rq;
static resource* res = resource::getinstance();

static bool hash_cmp(hashtable_node *lhs, hashtable_node *rhs) {
    entry_hashtable *le = container_of(lhs, entry_hashtable, node);
    entry_hashtable *re = container_of(rhs, entry_hashtable, node);
    return lhs->code == rhs->code && le->key == re->key;
}

static bool expect_zset(char* s, entry_zset** ent){
    entry_zset node;
    node.key = s;
    node.node.code = str_hash((const uint8_t*)node.key.data(), node.key.size());
    hashtable_node* hnode = hashtable_find(res->get_orderedlist_hash_resource(), &node.node, hash_cmp);

    if(hnode){
        *ent = container_of(hnode, entry_zset, node);
        return true;
    }

    return false;
}

static bool str2dbl(char* s, double &out){
    char *endp = NULL;
    out = strtod(s, &endp);
    return endp == s + strlen(s) && !isnan(out);
}

static bool str2int(char* s, int64_t &out){
    char *endp = NULL;
    out = strtoll(s, &endp, 10);
    return endp == s + strlen(s);
}

static void storage_set(storage_message** q){
    while((*q)->list_kv){
        auto cur = (*q)->list_kv;
        (*q)->list_kv = (*q)->list_kv->next;
        entry_hashtable node;
        node.key = cur->key;
        node.node.code = str_hash((const uint8_t*)node.key.data(), node.key.length());

        hashtable_node* hnode = hashtable_find(res->get_hash_resource(), &node.node, &hash_cmp);
        if(hnode){
            container_of(hnode, entry_hashtable, node)->value = cur->value;
        }
        else{
            entry_hashtable* ent = new entry_hashtable();
            ent->key.swap(node.key);
            ent->node.code = node.node.code;
            ent->value = cur->value;
            hashtable_add(res->get_hash_resource(), &ent->node);
        }
    }
    
    sprintf(*((*q)->buf), "%s%s", OK, END);
}

static void storage_get(storage_message** q){
    if(!(*q)->list_kv) {
        sprintf(*((*q)->buf), "%s%s", null, END);
        return;
    }

    entry_hashtable node;
    node.key = (*q)->list_kv->key;
    node.node.code = str_hash((const uint8_t*)node.key.data(), node.key.length());

    hashtable_node* hnode = hashtable_find(res->get_hash_resource(), &node.node, &hash_cmp);
    if(hnode){
        const char* data = container_of(hnode, entry_hashtable, node)->value.data();
        sprintf(*((*q)->buf), "%s%s", data, END);
        return;
    }
    sprintf(*((*q)->buf), "%s%s", null, END);
}

static void storage_del(storage_message** q){
    while((*q)->list_kv){
        auto cur = (*q)->list_kv;
        (*q)->list_kv = (*q)->list_kv->next;
        entry_hashtable node;
        node.key = cur->key;
        node.node.code = str_hash((const uint8_t*)node.key.data(), node.key.length());

        hashtable_node* hnode = hashtable_pop(res->get_hash_resource(), &node.node, &hash_cmp);
        if(hnode){
            delete container_of(hnode, entry_hashtable, node);
        }

        sprintf(*((*q)->buf), "%s%s", OK, END);
    }
}

static void storage_insert(storage_message** q){
    while((*q)->list_kv){
        auto cur = (*q)->list_kv;
        (*q)->list_kv = (*q)->list_kv->next;
        avl_add(res->get_avl_resource(), atoi(cur->key), cur->value);
    }
    sprintf(*((*q)->buf), "%s%s", OK, END);
}

static void storage_zget(storage_message** q){
    while((*q)->list_kv){
        auto cur = (*q)->list_kv;
        (*q)->list_kv = (*q)->list_kv->next;
        if(!res->get_avl_resource()->root){
            sprintf(*((*q)->buf), "%s%s", null, END);
            return;
        }

        avl_node* tar = avl_fix(res->get_avl_resource()->root);
        if(tar){
            const char* data = container_of(tar, Data_int, node)->key.data();
            sprintf(*((*q)->buf), "%s%s", data, END);
            return;
        }

        sprintf(*((*q)->buf), "%s%s", null, END);
    }
}

static void storage_zdel(storage_message** q){
    while((*q)->list_kv){
        auto cur = (*q)->list_kv;
        (*q)->list_kv = (*q)->list_kv->next;
        if(avl_delete(res->get_avl_resource(), atoi(cur->key))){
            sprintf(*((*q)->buf), "%s%s", OK, END);
            return;
        }
    }
    sprintf(*((*q)->buf), "%s%s", null, END);
}

static void storage_zset(storage_message** q){
    while((*q)->list_kv){
        auto cur = (*q)->list_kv;
        (*q)->list_kv = (*q)->list_kv->next;
        entry_zset ent;
        ent.key = (*q)->key;
        ent.node.code = str_hash((const uint8_t*)ent.key.data(), ent.key.size());
        hashtable_node* hnode = hashtable_find(res->get_orderedlist_hash_resource(), &ent.node, &hash_cmp);

        entry_zset* node = nullptr;
        if(!hnode){
            node = new entry_zset();
            node->key.swap(ent.key);
            node->node.code = ent.node.code;
            node->pzset = new orderedlist_zset();
            hashtable_add(res->get_orderedlist_hash_resource(), &node->node);        
        }
        else{
            node = container_of(hnode, entry_zset, node);
        }

        orderedlist_add(node->pzset, cur->value, strlen(cur->value), (double)atoi(cur->key));
        sprintf(*((*q)->buf), "%s%s", OK, END);
    }
}

static void storage_ztroy(storage_message** q){
    while((*q)->list_kv){
        auto cur = (*q)->list_kv;
        (*q)->list_kv = (*q)->list_kv->next;
        entry_zset* ent = nullptr;
        if(expect_zset(cur->key, &ent)){
            if(orderedlist_pop(ent->pzset, cur->key, strlen(cur->key))){
            strcpy(*((*q)->buf), OK);
            return;
            }
        } 
        sprintf(*((*q)->buf), "%s%s", null, END);
    }
}

static void storage_query(storage_message** q){
    while((*q)->list_kv){
        auto cur = (*q)->list_kv;
        (*q)->list_kv = (*q)->list_kv->next;
        entry_zset* ent = nullptr;
        double score = 0;
        const std::string& name = cur->value;
        int64_t offset = 0;
        int64_t limit = 0;
        //安全检查，确保命令正确和zset不为null
        {   
            if(!str2dbl(cur->key, score))
            {
                sprintf(*((*q)->buf), "%s%s", EXPECT_FP_NUMBER, END);
                return;
            }
        
            if(!str2int((*q)->limit1, offset))
            {
                sprintf(*((*q)->buf), "%s%s", EXPECT_INT, END);
                return;
            }

            if(!str2int((*q)->limit2, limit))
            {
                sprintf(*((*q)->buf), "%s%s", EXPECT_INT, END);
                return;
            }
        
            if(!expect_zset((*q)->key, &ent))
            {
                sprintf(*((*q)->buf), "%s%s", null, END);
                return;
            }

            if(limit <= 0)
            {
                sprintf(*((*q)->buf), "%s%s", null, END);
                return;
            }
        }
        zset_node* znode = orderedlist_find(ent->pzset, name.data(), name.size());
        if(znode){
            uint32_t n = 0;
            std::string tmp = "> ";
            while(znode != nullptr && static_cast<uint64_t>(n) < limit)
            {
                tmp.append(znode->name);
                tmp.append(" ");
                tmp.append(std::to_string(znode->score));
                tmp.append(" ");
                znode = container_of(avl_offset(&znode->tree, +1), zset_node, tree);
                n += 1;
            }
            sprintf(*((*q)->buf), "%s%s", tmp.data(), END);
            return;
        }
        sprintf(*((*q)->buf), "%s%s", null, END);
    }
}

static void storage_select(storage_message** q){
    switch ((*q)->cmd)
    {
        case SET:
            storage_set(q);
            break;
        case GET:
            storage_get(q);
            break;
        case DEL:
            storage_del(q);
            break;
        case ZADD:
            storage_insert(q);
            break;
        case ZDEL:
            storage_zget(q);
            break;
        case ZGET:
            storage_zdel(q);
            break;
        case ZSET:
            storage_zset(q);
            break;
        case ZTROY:
            storage_ztroy(q);
            break;
        case QUERY:
            storage_query(q);
            break;
        case PING:
            sprintf(*((*q)->buf), "%s%s", PANG, END);
            break;
        case QUIT:
            sprintf(*((*q)->buf), "%s%s", "Bey", END);
            Storage::set_isClose();
            break;
    default:
        sprintf(*((*q)->buf), "%s%s", null, END);
        break;
    }
}

static void storage_task(storage_message** q){
    storage_select(q);
}

void storage_insert_read_message(storage_message** q){
    rq.q.push(q);
    rq.size++;
}

void storage_rq_init(){
    pthread_create(&pid_s, nullptr, storage_read_message, nullptr);
    pthread_detach(pid_s);
}

void* storage_read_message(void* arg){
    while(true) {
        if(rq.size > 0){
            auto q = rq.q.front();
            rq.q.pop();
            rq.size--;
            storage_task(q);
            storage_insert_write_message(q);
        }
        else if(isClose){
            break;
        }
        else{
            usleep(0);
        }
    }
    return nullptr;
}

void storage_read_thread_close(){
    while (true)
    {
        if(rq.size <= 0){
            isClose = true;
            resource::DstoryInstance();
            return;
        }
    }    
}