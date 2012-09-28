all: test

test:
	g++ -fplugin=../recordsize.so -fplugin-arg-recordsize-print-all -o /dev/null test1.h
