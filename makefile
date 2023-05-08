# 将变量 CC 定义为编译 C++ 代码时所使用的编译器命令。
CC=g++  
# CXXFLAGS 被设置为 -std=c++0x，表示使用 C++11 标准进行编译。
CXXFLAGS = -std=c++0x
#CFLAGS 被设置为 -I，表示将一个目录添加到编译器的头文件搜索路径中。
CFLAGS=-I


skiplist: main.o 
	$(CC) -o ./bin/main main.o --std=c++11 -pthread 
	rm -f ./*.o
clean: 
	rm -f ./*.o
