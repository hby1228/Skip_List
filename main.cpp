#include <iostream>
#include "skiplist.h"
#define FILE_PATH "./store/dumpFile"
using namespace std;


int main() {

    // 键值中的key用int型，如果用其他类型，需要自定义比较函数
    // 而且如果修改key的类型，同时需要修改skipList.load_file函数
    
    cout << "先往跳表中插入数据" << endl;
    SkipList<int, std::string> skipList(6);
	skipList.insert_element(1, "霍"); 
	skipList.insert_element(3, "博岩"); 
	skipList.insert_element(7, "是"); 
	skipList.insert_element(8, "河北"); 
	skipList.insert_element(9, "省"); 
	skipList.insert_element(19, "保定市"); 
	// skipList.insert_element(19, "的"); 

    cout << "查看跳表大小" << endl;
    std::cout << "skipList size:" << skipList.size() << std::endl;

    cout << "把数据读入文件中" << endl;
    skipList.dump_file();

    // skipList.load_file();
    cout << "从跳表中查找元素“9”和“18”" << endl;
    skipList.search_element(9);
    
    if(!skipList.search_element(18)) 
        cout << "没找到元素18啊" << endl;

    cout << "展示跳表" << endl;
    skipList.display_list();

    cout << "删除元素3和7" << endl;
    skipList.delete_element(3);
    skipList.delete_element(7);

    std::cout << "skipList size:" << skipList.size() << std::endl;

    
   cout << "把数据读入文件中" << endl;
   skipList.dump_file();
}
