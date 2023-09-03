#include "storage_resource.h"

resource* resource::ress = nullptr;

resource* resource::getinstance(){
    if(!ress){
        ress = new resource();
    }

    return ress;
}

void resource::DstoryInstance(){
    if(ress){
        delete ress;
        ress = nullptr;
    }
}

resource::resource(){
    con = new Container();
    tb = new hashtable_map();
    orderedlist_zset_tb = new hashtable_map();
}

resource::~resource(){
    if(con){
        delete con;
        con = nullptr;
    }

    if(tb){
        delete tb;
        tb = nullptr;
    }

    if(orderedlist_zset_tb){
        delete orderedlist_zset_tb;
        orderedlist_zset_tb = nullptr;
    }
}

Container* resource::get_avl_resource(){
    return con;
}

hashtable_map* resource::get_hash_resource(){
    return tb;
}

hashtable_map* resource::get_orderedlist_hash_resource(){
    return orderedlist_zset_tb;
}