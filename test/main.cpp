#include "main.h"

static Container con;

struct HASHMAP
{
    HMap db;
}g_data;

static bool entry_eq(HNode *lhs, HNode *rhs) {
    struct Entry *le = container_of(lhs, struct Entry, node);
    struct Entry *re = container_of(rhs, struct Entry, node);
    return lhs->code == rhs->code && le->key == re->key;
}

static bool expect_zset(std::string &s, Entry2 **ent) {
    Entry2 key;
    key.key.swap(s);
    key.node.code = str_hash((uint8_t *)key.key.data(), key.key.size());
    HNode *hnode = hm_lookup(&g_data.db, &key.node, &entry_eq);
    if (hnode == nullptr) {
        return false;
    }

    *ent = container_of(hnode, Entry2, node);
    if ((*ent)->type != T_ZSET) {
        return false;
    }
    return true;
}

static bool str2dbl(const std::string &s, double &out) {
    char *endp = NULL;
    out = strtod(s.c_str(), &endp);
    return endp == s.c_str() + s.size() && !isnan(out);
}

static bool str2int(const std::string &s, int64_t &out) {
    char *endp = NULL;
    out = strtoll(s.c_str(), &endp, 10);
    return endp == s.c_str() + s.size();
}

void do_get(std::vector<std::string>& cmd, char* buf)
{
    Entry key;
    key.key.swap(cmd[1]);
    key.node.code = str_hash((uint8_t*)key.key.data(), key.key.size());

    HNode* node = hm_lookup(&g_data.db, &key.node, &entry_eq);
    if (nullptr == node)
    {
        std::cout << "null" << std::endl;
        strcpy(buf, "null");
        return;
    }

    const std::string& value = container_of(node, Entry, node)->value;
    assert(value.size() <= k_max_args);
    std::cout << value.data() << std::endl;
    strcpy(buf, value.data());
}

void delete_key_value(void* arg){
    if(arg == nullptr){
        return;
    }
    std::string str = (char*)arg;
    Entry key;
    key.key.swap(str);
    key.node.code = str_hash((uint8_t*)key.key.data(), key.key.size());

    HNode* node = hm_pop(&g_data.db, &key.node, &entry_eq);
    if (nullptr != node)
    {
        delete container_of(node, Entry, node);
    }
}

void do_set(std::vector<std::string>& cmd, char* buf, TimerWheel* tw)
{
    Entry key;
    key.key.swap(cmd[1]);
    key.node.code = str_hash((uint8_t*)key.key.data(), key.key.size());

    HNode* node = hm_lookup(&g_data.db, &key.node, &entry_eq);
    if (nullptr != node)
    {
        container_of(node, Entry, node)->value.swap(cmd[2]);
        std::cout << "set error" << std::endl;
        strcpy(buf, "set error");
    }
    else
    {
        Entry* ent = new Entry();
        ent->key.swap(key.key);
        ent->node.code = key.node.code;
        ent->value.swap(cmd[2]);
        hm_insert(&g_data.db, &ent->node);

        if(!cmd[3].empty() && !cmd[4].empty() && cmd[3] == "expire")
        {
            tw->add(atoi(cmd[4].data()), delete_key_value, (void*)ent->key.data());
            std::cout << "timer OK" << std::endl;
        }
        std::cout << "OK" << std::endl;
        strcpy(buf, "OK");
    }

}



void do_del(std::vector<std::string>& cmd, char* buf)
{
    Entry key;
    key.key.swap(cmd[1]);
    key.node.code = str_hash((uint8_t*)key.key.data(), key.key.size());

    HNode* node = hm_pop(&g_data.db, &key.node, &entry_eq);
    if (nullptr != node)
    {
        delete container_of(node, Entry, node);
        std::cout << "OK" << std::endl;
        if(buf != nullptr){
            strcpy(buf, "OK");
        }
    }
}

void get_cmd(std::vector<std::string>& cmd, std::string& str_cmd)
{
    for(int i = 0, j = 0; i < str_cmd.size() && j < 10; ++i)
    {
        std::string str_tmp = "";
        while(i < str_cmd.size() && ' ' != str_cmd[i])
        {
            str_tmp += str_cmd[i++];
        }
        cmd[j++] = str_tmp;
    }
}

void do_zadd(std::vector<std::string>& cmd, char* buf, TimerWheel* tw)
{
    for(int i = 1; i < cmd.size() - 1; i += 2)
    {
        avl_add(con, atoi(cmd[i].c_str()), cmd[i + 1]);
    }
    std::cout << "OK" << std::endl;
    strcpy(buf, "OK");
}

void do_zdel(std::vector<std::string>& cmd, char* buf)
{
    avl_delete(con, atoi(cmd[1].c_str()));
    std::cout << "OK" << std::endl;
    strcpy(buf, "OK");
}

void do_zget(std::vector<std::string>& cmd, char* buf)
{
    if(con.root == nullptr)
    {
        strcpy(buf, "null");
        return;
    }
    AVLNode* cur = avl_fix(con.root);
    if(cur != nullptr)
    {
        int ret = container_of(cur, Data_int, node)->val;
        std::string str = container_of(cur, Data_int, node)->key;

        sprintf(buf, "key %d value %s\n", ret, str.c_str());
    }   
}

void do_zset(std::vector<std::string>& cmd, char* buf, TimerWheel* tw)
{
    double score = 0.0;
    if(!str2dbl(cmd[2], score))
    {
        strcpy(buf, "error");
        return;
    }

    Entry2 key;
    key.key.swap(cmd[1]);
    key.node.code = str_hash((const uint8_t*)key.key.data(), key.key.size());
    HNode* hnode = hm_lookup(&g_data.db, &key.node, &entry_eq);

    //表里没有直接插入，有type不同直接返回error
    Entry2* ent = nullptr;
    if(hnode == nullptr)
    {
        ent = new Entry2();
        ent->key.swap(key.key);
        ent->node.code = key.node.code;
        ent->type = T_ZSET;
        ent->zset = new ZSet();
        hm_insert(&g_data.db, &ent->node);
    }
    else
    {
        ent = container_of(hnode, Entry2, node);
        if(ent->type != T_ZSET)
        {
            strcpy(buf, "error");
            return;
        }
    }

    const std::string& name = cmd[3];
    if(ZNode_add(ent->zset, name.data(), name.size(), score))
    {
        strcpy(buf, "OK");
        return;
    }

    strcpy(buf, "FAIL");
}

// zrem zset name
void do_troy(std::vector<std::string>& cmd, char* buf)
{
    //安全检查
    Entry2* ent = nullptr;
    if(!expect_zset(cmd[1], &ent))
    {
        strcpy(buf, "null");
        return;
    }

    const std::string& name = cmd[2];
    if(ZNode_pop(ent->zset, name.data(), name.size()))
    {
        strcpy(buf, "OK");
        return;
    }

    strcpy(buf, "FAIL");
}

// zquery zset score name offset limit
void do_query(std::vector<std::string>& cmd, char* buf)
{
    Entry2* ent = nullptr;
    double score = 0.0;
    const std::string& name = cmd[3];
    int64_t offset = 0;
    int64_t limit = 0;
    //安全检查，确保命令正确和zset不为null
    {        
        if(!str2dbl(cmd[2], score))
        {
            strcpy(buf, "expect fp number");
            return;
        }
        
        if(!str2int(cmd[4], offset))
        {
            strcpy(buf, "expect int");
            return;
        }

        if(!str2int(cmd[5], limit))
        {
            strcpy(buf, "expect int");
            return;
        }
        
        if(!expect_zset(cmd[1], &ent))
        {
            strcpy(buf, "null");
            return;
        }

        if(limit <= 0)
        {
            strcpy(buf, "null");
            return;
        }
    }

    ZNode* znode = ZNode_query(ent->zset, score, name.data(), name.size(), offset);
    uint32_t n = 0;
    std::string tmp = "> ";
    while(znode != nullptr && static_cast<uint64_t>(n) < limit)
    {
        tmp.append(znode->name);
        tmp.append(" ");
        tmp.append(std::to_string(znode->score));
        tmp.append(" ");
        znode = container_of(avl_offset(&znode->tree, +1), ZNode, tree);
        n += 2;
    }
    strcpy(buf, tmp.data());
}