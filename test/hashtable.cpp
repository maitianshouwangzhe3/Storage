#include "../include/hashtable.h"

HashTable::HashTable(){
	this->g_data = new HMap();
	this->lock = new spinlock();
}

HashTable::~HashTable(){
	delete lock;
	delete g_data;
}

bool HashTable::hm_insert(std::string& key, std::string& val){
	if(key.empty()){
		return false;
	}

	Entry* ent = new Entry();
    ent->key.swap(key);
	ent->value.swap(val);
    ent->node.code = str_hash((uint8_t*)ent->key.data(), ent->key.size());

	lock->spinlock_lock();
	HNode** node = h_lookup(&g_data->table1, &ent->node);
    if (nullptr != node){
		//不为空直接替换，这里暂时不考虑哈希冲突
        container_of(*node, Entry, node)->value.swap(val);
		lock->spinlock_unlock();
		return true;
    }

	if (nullptr == g_data->table1.table){
		hm_init(&g_data->table1, 4);
	}

	//插入节点
	h_insert(&g_data->table1, &ent->node);

	if (nullptr == g_data->table2.table){
		//判断是否需要迁移表
		int load_factor = g_data->table1.size / (g_data->table1.mask + 1);
		if (load_factor >= k_max_load_factor)
		{
			hm_start_resizing();
		}
	}

	hm_help_resizing();
	lock->spinlock_unlock();
	return true;
}

bool HashTable::hm_insert(HMap* hp, HNode* node){
	if (nullptr == hp->table1.table)
	{
		hm_init(&hp->table1, 4);
	}

	h_insert(&hp->table1, node);

	if (nullptr == hp->table2.table)
	{

		int load_factor = hp->table1.size / (hp->table1.mask + 1);
		if (load_factor >= k_max_load_factor)
		{
			hm_start_resizing();
		}
	}

	hm_help_resizing();
}

bool HashTable::hm_pop(std::string& key){
	if(key.empty()){
		return false;
	}

	//生成零时节点
	Entry ent;
    ent.key.swap(key);
    ent.node.code = str_hash((uint8_t*)ent.key.data(), ent.key.size());

	lock->spinlock_lock();
	hm_help_resizing();
	HNode** from = h_lookup(&g_data->table1, &ent.node);
	if (nullptr != from)
	{
		HNode* del_node = hm_detach(&g_data->table1, from);
		delete container_of(del_node, Entry, node);
		del_node = nullptr;
		lock->spinlock_unlock();
		return true;
	}


	from = h_lookup(&g_data->table2, &ent.node);
	if (nullptr != from)
	{
		HNode* del_node = hm_detach(&g_data->table2, from);
		delete container_of(del_node, Entry, node);
		del_node = nullptr;
		lock->spinlock_unlock();
		return true;
	}

	lock->spinlock_unlock();
	return false;
}

HNode* HashTable::hm_pop(HMap* hp, HNode* node, bool (*cmp)(HNode*, HNode*)){
	hm_help_resizing(hp);
	HNode** from = h_lookup(&hp->table1, node, cmp);
	if (nullptr != from)
	{
		return hm_detach(&hp->table1, from);
	}

	from = h_lookup(&hp->table2, node, cmp);
	if (nullptr != from)
	{
		return hm_detach(&hp->table2, from);
	}

	return nullptr;
}

HNode* HashTable::hm_lookup(std::string& key){
	if(key.empty()){
		return nullptr;
	}
	//生成临时节点
	Entry ent;
    ent.key.swap(key);
    ent.node.code = str_hash((uint8_t*)ent.key.data(), ent.key.size());

	lock->spinlock_lock();
	hm_help_resizing();
	HNode** from = h_lookup(&g_data->table1, &ent.node);
	if (nullptr == from)
	{
		from = h_lookup(&g_data->table2, &ent.node);
	}
	lock->spinlock_unlock();
	return from ? *from : nullptr;
}

HNode* HashTable::hm_lookup(HMap* hp, HNode* key, bool (*cmp)(HNode*, HNode*)){
	hm_help_resizing(hp);
	HNode** from = h_lookup(&hp->table1, key, cmp);
	if (nullptr == from)
	{
		from = h_lookup(&hp->table2, key, cmp);
	}

	return from ? *from : nullptr;
}

void HashTable::hm_destroy(){
	assert(g_data->table1.size + g_data->table2.size == 0);
	free(g_data->table1.table);
	free(g_data->table2.table);
	g_data = nullptr;
}

uint64_t HashTable::str_hash(const uint8_t* data, size_t len){
	uint32_t h = 0x811C9DC5;
	for (size_t i = 0; i < len; i++) {
		h = (h + data[i]) * 0x01000193;
	}
	return h;
}

bool HashTable::entry_eq(HNode* lhs, HNode* rhs){
	struct Entry* le = container_of(lhs, struct Entry, node);
	struct Entry* re = container_of(rhs, struct Entry, node);
	return lhs->code == rhs->code && le->key == re->key;
}

void HashTable::hm_init(HTable* tle, int n){
	assert((n > 0) && (n & (n - 1)) == 0);
	tle->table = (HNode**)calloc(sizeof(HNode*), n);
	tle->mask = n - 1;
	tle->size = 0;
}

void HashTable::h_insert(HTable* tb, HNode* node){
	size_t pos = node->code & tb->mask;
	HNode* next = tb->table[pos];
	node->next = next;
	tb->table[pos] = node;
	++tb->size;
}

HNode* HashTable::hm_detach(HTable* tb, HNode** from){
	HNode* node = *from;
	*from = (*from)->next;
	--tb->size;
	return node;
}

void HashTable::hm_help_resizing(){
	if (nullptr == g_data->table2.table)
	{
		return;
	}

	int work = 0;
	while (work < k_resizing_work && g_data->table2.size > 0)
	{
		//扫描表2节点并移到表1
		HNode** from = &g_data->table2.table[g_data->ResizingPos];
		if (nullptr == (*from))
		{
			++g_data->ResizingPos;
			continue;
		}

		h_insert(&g_data->table1, hm_detach(&g_data->table2, from));
		++work;
	}

	if (g_data->table2.size == 0)
	{
		free(g_data->table2.table);
		g_data->table2 = HTable{};
	}
}

void HashTable::hm_help_resizing(HMap* hp){
	if (nullptr == hp->table2.table)
	{
		return;
	}

	int work = 0;
	while (work < k_resizing_work && hp->table2.size > 0)
	{
		//扫描表2节点并移到表1
		HNode** from = &hp->table2.table[hp->ResizingPos];
		if (nullptr == (*from))
		{
			hp->ResizingPos++;
			continue;
		}

		h_insert(&hp->table1, hm_detach(&hp->table2, from));
		++work;
	}

	if (hp->table2.size == 0)
	{
		free(hp->table2.table);
		hp->table2 = HTable{};
	}
}

void HashTable::hm_start_resizing(){
	assert(nullptr == g_data->table2.table);
	g_data->table2 = g_data->table1;
	hm_init(&g_data->table1, (g_data->table1.mask + 1) * 2);
	g_data->ResizingPos = 0;
}

HNode** HashTable::h_lookup(HTable* tb, HNode* ent){
	if (nullptr == tb->table)
	{
		return nullptr;
	}

	int pos = ent->code & tb->mask;
	HNode** from = &tb->table[pos];
	while (*from != nullptr)
	{
		if (entry_eq(*from, ent))
		{
			return from;
		}
		from = &(*from)->next;
	}
	return nullptr;
}

HNode** HashTable::h_lookup(HTable* tb, HNode* key, bool (*cmp)(HNode*, HNode*)){
	if (nullptr == tb->table)
	{
		return nullptr;
	}

	int pos = key->code & tb->mask;
	HNode** from = &tb->table[pos];
	while (*from != nullptr)
	{
		if (cmp(*from, key))
		{
			return from;
		}
		from = &(*from)->next;
	}
	return nullptr;
}


HMap* HashTable::get(){
	return g_data;
}