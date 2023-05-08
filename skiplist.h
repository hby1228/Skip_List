#include <iostream> 
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

#define STORE_FILE "store/dumpFile"

#include<math>
//创建了一个名为mtx的互斥量，用于保护一个关键代码段的执行，即所谓的临界区
std::mutex mtx;     // mutex for critical section
//将字符串分隔符设置为:
std::string delimiter = ":";

//节点的类模版
template<typename K, typename V> 
class Node {

public:
    
    Node() {} 

    Node(K k, V v, int); 

    ~Node();

    //返回key
    K get_key() const;

    //返回value
    V get_value() const;

    //设置设置值
    void set_value(V);
    
    //即forward是一个Node<K, V>类型的指针数组，存储的是指向不同层级下一个节点的指针。
    Node<K, V> **forward;
    //层级
    int node_level;

private:
    //键和值
    K key;
    V value;
};

//节点的构造函数
template<typename K, typename V> 
Node<K, V>::Node(const K k, const V v, int level) {
    //分别给键、值、所在的层级赋值
    this->key = k;
    this->value = v;
    this->node_level = level; 

    // level + 1, because array index is from 0 - level
    this->forward = new Node<K, V>*[level+1];
    
	// 构造的新节点的指向NULL
    memset(this->forward, 0, sizeof(Node<K, V>*)*(level+1));
};


//析构函数，删除所在行的节点
template<typename K, typename V> 
Node<K, V>::~Node() {
    delete []forward;
};

//取得节点的键
template<typename K, typename V> 
K Node<K, V>::get_key() const {
    return key;
};


//取得的值
template<typename K, typename V> 
V Node<K, V>::get_value() const {
    return value;
};

//设置值
template<typename K, typename V> 
void Node<K, V>::set_value(V value) {
    this->value=value;
};


//
// 跳表的类模版
template <typename K, typename V> 
class SkipList {
public:
    //
    SkipList(int);
    ~SkipList();
    int get_random_level();
    Node<K, V>* create_node(K, V, int);
    int insert_element(K, V);
    void display_list();
    bool search_element(K);
    void delete_element(K);
    void dump_file();
    void load_file();
    int size();

private:
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);

private:    
    // 跳表的最大层
    int _max_level;

    // 跳表的当前层
    int _skip_list_level;

    // 头节点指针
    Node<K, V> *_header;

    // 文件读写
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    // 跳表的当前元素节点
    int _element_count;
};

// 创建一个新的节点对象并返回它的指针
//函数接受三个参数：
// 一个键值k，用于创建新节点的键值。
// 一个值v，用于创建新节点的值。
// 一个整数level，用于创建新节点的层数。
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level) {
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
}

// Insert given key and value in skip list 
// return 1 means element exists  
// return 0 means insert successfully
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+

*/

//插入元素
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {
    //先上锁   
    mtx.lock();
    //头节点指针
    Node<K, V> *current = this->_header;

    //创建undate数组并初始化它
    //包含_max_level+1个指向Node<K, V>类型的指针的数组，数组名为update。
    Node<K, V> *update[_max_level+1];
    //将这些指针都设为NULL
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));

    // 从跳表的最高层级开始操作
    for(int i = _skip_list_level; i >= 0; --i) {
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i]; 
        }
        update[i] = current;
    }

    // reached level 0 and forward pointer to right node, which is desired to insert key.
    current = current->forward[0];

    // 如果要插入的值和跳表中的一个值相等了，返回提示：值已经存在了
    if (current != NULL && current->get_key() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }

    // if current is NULL that means we have reached to end of the level 
    // if current's key is not equal to key that means we have to insert node between update[0] and current node 
    if (current == NULL || current->get_key() != key ) {
        
        // 生成一个节点随机层，然后从这个随机层进行构建
        int random_level = get_random_level();

        // 如果随机生成的层数大于当前节点的层数, 用头指针初始化更新值
        if (random_level > _skip_list_level) {
            for (int i = _skip_list_level+1; i < random_level+1; i++) {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        // 在随机层数上创建一个新节点
        Node<K, V>* inserted_node = create_node(key, value, random_level);
        
        // 把这个新节点插入跳表中
        for (int i = random_level; i >= 0; --i) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        //输出提示信息，成功插入了节点
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        //跳表中节点数量增加
        _element_count ++;
    }
    mtx.unlock();
    return 0;
}


// 展示跳表
template<typename K, typename V> 
void SkipList<K, V>::display_list() {

    std::cout << "\n*****Skip List*****"<<"\n"; 
    //i是层数
    for (int i = 0; i <= _skip_list_level; i++) {
        Node<K, V> *node = this->_header->forward[i]; 
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

// 将内存中的数据转存到文件中 
template<typename K, typename V> 
void SkipList<K, V>::dump_file() {

    std::cout << "dump_file-----------------" << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0]; 

    while (node != NULL) {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return ;
}

// 从文件中读取数据
template<typename K, typename V> 
void SkipList<K, V>::load_file() {

    _file_reader.open(STORE_FILE);
    std::cout << "load_file-----------------" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while (getline(_file_reader, line)) {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty()) {
            continue;
        }
        insert_element(*key, *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    _file_reader.close();
}

// 获取当前跳表大小
template<typename K, typename V> 
int SkipList<K, V>::size() { 
    return _element_count;
}

//从键值对字符串中获取键和值
template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {

    if(!is_valid_string(str)) {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter)+1, str.length());
}

//判断是否是有效的键值对字符串。
template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {
    //是空不行
    if (str.empty()) {
        return false;
    }
    //没有连接符不行
    //npos表示不存在查找分隔符
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

// 从跳表中删除元素 
template<typename K, typename V> 
void SkipList<K, V>::delete_element(K key) {

    mtx.lock();
    Node<K, V> *current = this->_header; 
    Node<K, V> *update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));

    // start from highest level of skip list
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] !=NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];
    if (current != NULL && current->get_key() == key) {
       
        // start for lowest level and delete the current node of each level
        for (int i = 0; i <= _skip_list_level; i++) {

            // if at level i, next node is not target node, break the loop.
            if (update[i]->forward[i] != current) 
                break;

            update[i]->forward[i] = current->forward[i];
        }

        // Remove levels which have no elements
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            _skip_list_level --; 
        }

        std::cout << "Successfully deleted key "<< key << std::endl;
       delete current;
        _element_count --;
    }
    mtx.unlock();
    return;
}

// Search for element in skip list 
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100

*/

//查找元素
template<typename K, typename V> 
bool SkipList<K, V>::search_element(K key) {

    std::cout << "search_element-----------------" << std::endl;
    Node<K, V> *current = _header;

    // start from highest level of skip list
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    //reached level 0 and advance pointer to right node, which we search
    current = current->forward[0];

    // if current node have key equal to searched key, we get it
    if (current and current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

// 构建跳表，最大层是输入值，当前链表层和元素都设置为0，同时设置跳表元素为null
template<typename K, typename V> 
SkipList<K, V>::SkipList(int max_level) {

    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    // create header node and initialize key and value to null
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
};

//析构函数的作用是关闭文件读写
template<typename K, typename V> 
SkipList<K, V>::~SkipList() {

    if (_file_writer.is_open()) {
        _file_writer.close();
    }
    if (_file_reader.is_open()) {
        _file_reader.close();
    }
    
       //添加部分内容,循环删除链表的所有节点
    Node<K, V>* current = _header -> forward[0];
    while(current){
        Node<K, V>* t = current -> forward[0];
        delete current;
        current = t;
    }
    delete _header;
}

//从哪层开始重新构建，这是个概率问题
template<typename K, typename V>
int SkipList<K, V>::get_random_level(){

    int k = 0;
    while (rand() % 2) {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
};
// vim: et tw=100 ts=4 sw=4 cc=120
