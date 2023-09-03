#include "storage_hashtable.h"


static void init(table* tle, int n){
    assert((n > 0) && (n & (n - 1)) == 0);
	tle->table = (hashtable_node**)calloc(sizeof(hashtable_node*), n);
	tle->mask = n - 1;
	tle->size = 0;
}
    
static void insert(table* tb, hashtable_node* node){
    size_t pos = node->code & tb->mask;
	hashtable_node* next = tb->table[pos];
	node->next = next;
	tb->table[pos] = node;
	tb->size++;
}
    
static hashtable_node* detach(table* tb, hashtable_node** from){
    hashtable_node* node = *from;
	*from = (*from)->next;
	tb->size--;
	return node;
}
    
static void hm_help_resizing(hashtable_map* hp){
    if (nullptr == hp->table2.table)
	{
		return;
	}

	int work = 0;
	while (work < k_resizing_work && hp->table2.size > 0)
	{
		//扫描表2节点并移到表1
		hashtable_node** from = &hp->table2.table[hp->ResizingPos];
		if (nullptr == (*from))
		{
			hp->ResizingPos++;
			continue;
		}

		insert(&hp->table1, detach(&hp->table2, from));
		++work;
	}

	if (hp->table2.size == 0)
	{
		free(hp->table2.table);
		hp->table2 = table{};
	}
}

static void hm_start_resizing(hashtable_map* hp){
    assert(nullptr == hp->table2.table);
	hp->table2 = hp->table1;
	init(&hp->table1, (hp->table1.mask + 1) * 2);
	hp->ResizingPos = 0;
}

static hashtable_node** lookup(table* tb, hashtable_node* key, bool (*cmp)(hashtable_node*, hashtable_node*)){
    if (nullptr == tb->table)
	{
		return nullptr;
	}

	int pos = key->code & tb->mask;
	hashtable_node** from = &tb->table[pos];
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

hashtable_node* hashtable_find(hashtable_map* hp, hashtable_node* key, bool (*cmp)(hashtable_node*, hashtable_node*)){
	hm_help_resizing(hp);
	hashtable_node** from = lookup(&hp->table1, key, cmp);
	if (nullptr == from)
	{
		from = lookup(&hp->table2, key, cmp);
	}

	return from ? *from : nullptr;
}
    
void hashtable_add(hashtable_map* hp, hashtable_node* node){
    if (nullptr == hp->table1.table)
	{
		init(&hp->table1, 4);
	}

	insert(&hp->table1, node);

	if (nullptr == hp->table2.table)
	{

		int load_factor = hp->table1.size / (hp->table1.mask + 1);
		if (load_factor >= k_max_load_factor)
		{
			hm_start_resizing(hp);
		}
	}

	hm_help_resizing(hp);
}
    
hashtable_node* hashtable_pop(hashtable_map* hp, hashtable_node* node, bool (*cmp)(hashtable_node*, hashtable_node*)){
    hm_help_resizing(hp);
	hashtable_node** from = lookup(&hp->table1, node, cmp);
	if (nullptr != from)
	{
		return detach(&hp->table1, from);
	}

	from = lookup(&hp->table2, node, cmp);
	if (nullptr != from)
	{
		return detach(&hp->table2, from);
	}

	return nullptr;
}
    
void hashtable_destroy(hashtable_map* hp){
    assert(hp->table1.size + hp->table2.size == 0);
	free(hp->table1.table);
	free(hp->table2.table);
	hp = nullptr;
}
    
uint64_t str_hash(const uint8_t* data, size_t len){
    uint32_t h = 0x811C9DC5;
	for (size_t i = 0; i < len; i++) {
		h = (h + data[i]) * 0x01000193;
	}
	return h;
}
