#include "avltree.h"

AvlTree::AvlTree(){
    this->con = new Container();
    this->lock = new spinlock();
}

AvlTree::~AvlTree(){
    while(con->root != nullptr){
        AVLNode* node = con->root;
        con->root = avl_find_node_del(con->root);
        delete container_of(node, Data_int, node);
    }
    delete con;
    delete lock;
}

bool AvlTree::avl_add(std::string& key, uint32_t val){
    if(key.empty()){
        return false;
    }

    Data_int* data = new Data_int();

    avl_init(&data->node);
    data->val = val;
    data->key = key;

    lock->spinlock_lock();

    //根节点等于空直接放进根节点
    if(con->root == nullptr)
    {
        con->root = &data->node;
        lock->spinlock_unlock();
        return true;
    }

    AVLNode* cur = con->root;
    while(true)
    {
        //结算val比对应节点大还是小，大就选取右子树，反之选左子树
        AVLNode** from = (val < container_of(cur, Data_int, node)->val) ? &cur->left :&cur->right;
        if(*from == nullptr)
        {
            //是叶子节点就直接插入，并开始开始旋转保持avl树平衡
            *from = &data->node;
            data->node.parent = cur;
            con->root = avl_fix(&data->node);
            break;
        }

        //没到叶子结点继续向下寻找
        cur = *from;
    }

    lock->spinlock_unlock();
    return true;
}

bool AvlTree::avl_delete(uint32_t val){
    lock->spinlock_lock();

    //寻找节点
    AVLNode* cur = con->root;
    while(cur != nullptr)
    {
        uint32_t now_val = container_of(cur, Data_int, node)->val;
        if(val == now_val)
        {
            break;
        }

        cur = val < now_val ? cur->left : cur->right;
    }

    if(cur == nullptr)
    {
        lock->spinlock_unlock(); 
        return false;
    }

    //删除节点
    con->root = avl_find_node_del(cur);
    delete container_of(cur, Data_int, node);
    lock->spinlock_unlock();
    return true;
}

AVLNode* AvlTree::avl_find(AVLNode* node){
    return avl_find_node_del(node);
}

void AvlTree::avl_update(AVLNode* node){
    node->depth = 1 + max_node(avl_depth(node->left), avl_depth(node->right));
    node->cnt   = 1 + avl_cnt(node->left) + avl_cnt(node->right);
}

void AvlTree::avl_init(AVLNode* node){
    node->depth = 1;
    node->cnt = 1;
    node->left = nullptr;
    node->right = nullptr;
    node->parent = nullptr;
}

int AvlTree::avl_depth(AVLNode* node){
    return node ? node->depth : 0;
}

int AvlTree::avl_cnt(AVLNode* node){
    return node ? node->cnt : 0;
}

int AvlTree::max_node(uint32_t lhs, uint32_t rhs){
    return lhs < rhs ? rhs : lhs;
}

AVLNode* AvlTree::avl_find_node_del(AVLNode* node){
    if(nullptr == node->right)
    {
        //如果右节点的父节点为空，并且其左节点不为空，直接把左节点的把左节点的parent指向父节点
        AVLNode* parent = node->parent;
        if(nullptr != node->left)
        {
            node->left->parent = parent;
        }

        //调整avl树平衡
        if(nullptr != parent)
        {
            (parent->left == node ? parent->left : parent->right) = node->left;
            return avl_fix(parent);
        }
        else
        {
            return node->left;
        }
    }
    else
    {
        //右节点不为空,寻找子节点
        AVLNode* victim = node->right;
        while(nullptr != victim->left)
        {
            victim = victim->left;
        }

        AVLNode* root = avl_find_node_del(victim);

        //处理parent指向
        *victim = *node;
        if(nullptr != victim->left)
        {
            victim->left->parent = victim;
        }
        if(nullptr != victim->right)
        {
            victim->right->parent = victim;
        }

        AVLNode* parent = node->parent;
        if(nullptr != parent)
        {
            (parent->left == node ? parent->left : parent->right) = victim;
            return root;
        }
        else
        {
            return victim;
        }
    }
}

AVLNode* AvlTree::avl_fix(AVLNode* root){
    if(root == nullptr){
        return nullptr;
    }
    while(true)
    {
        //更新左右子树高度后拿到他们
        avl_update(root);
        uint32_t lh = avl_depth(root->left);
        uint32_t rh = avl_depth(root->right);

        AVLNode** from = nullptr;
        if(nullptr != root->parent)
        {
            from = (root->parent->left == root) ? &root->parent->left : &root->parent->right;
        }

        //出现不平衡(左右子树高度差等于2)就开始旋转
        if(lh == rh + 2)
        {
            root = avl_fix_left(root);
        }
        if(rh == lh + 2)
        {
            root =  avl_fix_right(root);
        }

        if(nullptr == from)
        {
            return root;
        }

        *from = root;
        root = root->parent;
    }
    return nullptr;
}

//向前或者向后的节点偏移
//无论偏移距离多长，最坏的结果都为O(log N)
AVLNode* AvlTree::avl_offset(AVLNode* node, int64_t offset){
    //起点
    int pos = 0;
    while(pos != offset)
    {
        if(pos < offset && pos + avl_cnt(node->right) >= offset)
        {
            //元组在右子树
            node = node->right;
            pos += avl_cnt(node->right) + 1;
        }
        else if(pos > offset && pos - avl_cnt(node->left) <= offset)
        {
            //元组在左子树
            node = node->left;
            pos -= avl_cnt(node->left) + 1;
        }
        else
        {
            //向上找父节点
            AVLNode* parent = node->parent;
            if(parent == nullptr)
            {
                return nullptr;
            }

            //将之前+（-）的cnt值+（-）回来
            if(parent->right == node)
            {
                pos -= avl_cnt(node->left) + 1;
            }
            else
            {
                pos += avl_cnt(node->right) + 1;
            }

            node = parent;
        }
    }
    return node;
}

AVLNode* AvlTree::avl_offset_t(AVLNode* node, int64_t offset){
    return avl_offset(node, offset);
}

AVLNode* AvlTree::rot_left(AVLNode* node)
{
    AVLNode* new_node = node->right;
    if(nullptr != new_node->left)
    {
        new_node->left->parent = node;
    }

    node->right = new_node->left;
    new_node->left = node;
    new_node->parent = node->parent;
    node->parent = new_node;

    avl_update(node);
    avl_update(new_node);

    return new_node;
}

AVLNode* AvlTree::rot_right(AVLNode* node)
{
    AVLNode* new_node = node->left;
    if(nullptr != new_node->right)
    {
        new_node->right->parent = node;
    }

    node->left = new_node->right;
    new_node->right = node;
    new_node->parent = node->parent;
    node->parent = new_node;

    avl_update(node);
    avl_update(new_node);

    return new_node;
}

AVLNode* AvlTree::avl_fix_left(AVLNode* root)
{
    if(avl_depth(root->left->left) < avl_depth(root->left->right))
    {
        root->left = rot_left(root->left);
    }
    return rot_right(root);
}

AVLNode* AvlTree::avl_fix_right(AVLNode* root)
{
    if(avl_depth(root->right->right) < avl_depth(root->right->left))
    {
        root->right = rot_right(root->right);
    }
    return rot_left(root);
}

Container_s* AvlTree::get(){
    return con;
}