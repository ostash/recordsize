all: recordsize

recordsize:
	g++ -shared -fpic -std=gnu++98 -O2 -march=native -Wall -fvisibility=hidden \
	-I `g++ --print-file-name=plugin`/include -o recordsize.so \
	recordsize.cpp FieldInfo.cpp
