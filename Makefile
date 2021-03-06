CC=gcc -g -std=c11 -msse4
CXX=g++ -g -std=c++17 -msse4
OPT=-O2 -DNDEBUG
BMARK=$(CXX) $(OPT) -lbenchmark
FOLLY_BMARK=$(CXX) $(OPT) -lfollybenchmark -lgflags
CXXTEST=$(CXX) -lgtest -lgtest_main

.PHONY: all build runbenchmarks buildbenchmarks runtests buildtests clean

all: runbenchmarks runtests

build: buildbenchmarks buildtests

runbenchmarks: buildbenchmarks
	./minposBenchmark.out
	./minposFollyBenchmark.out
	./HeapBenchmark.out
	./HeapMapBenchmark.out
	./MergeBenchmark.out
	./Sort8Benchmark.out

buildbenchmarks: minposBenchmark.out minposFollyBenchmark.out HeapBenchmark.out HeapMapBenchmark.out MergeBenchmark.out Sort8Benchmark.out

minposBenchmark.out: minposBenchmark.cpp minpos.h
	$(BMARK) minposBenchmark.cpp -o minposBenchmark.out

minposFollyBenchmark.out: minposFollyBenchmark.cpp minpos.h
	$(FOLLY_BMARK) minposFollyBenchmark.cpp -o minposFollyBenchmark.out

HeapBenchmark.out: HeapBenchmark.cpp StdMinHeap.hpp Heap8.hpp H8.hpp minpos.h v128.h align.h h8.h h8.o
	$(FOLLY_BMARK) h8.o HeapBenchmark.cpp -o HeapBenchmark.out

HeapMapBenchmark.out: HeapMapBenchmark.cpp Heap8Aux.hpp Heap8Embed.hpp StdMinHeapMap.hpp StdMinHeap.hpp FirstCompare.hpp U48.hpp minpos.h v128.h align.h
	$(FOLLY_BMARK) HeapMapBenchmark.cpp -o HeapMapBenchmark.out

MergeBenchmark.out: MergeBenchmark.cpp Heap8Aux.hpp Heap8Embed.hpp StdMinHeapMap.hpp StdMinHeap.hpp FirstCompare.hpp minpos.h v128.h align.h
	$(BMARK) -lbenchmark_main MergeBenchmark.cpp -o MergeBenchmark.out

Sort8Benchmark.out: Sort8Benchmark.cpp Sort8.hpp Sort8.o
	$(FOLLY_BMARK) Sort8.o Sort8Benchmark.cpp -o Sort8Benchmark.out

runtests: buildtests
	./minposTest.out
	./U48Test.out
	./h8Test.out
	./HeapTest.out
	./HeapMapTest.out
	./Sort8Test.out

buildtests: minposTest.out U48Test.out h8Test.out HeapTest.out HeapMapTest.out Sort8Test.out

U48Test.out: U48Test.cpp U48.hpp
	$(CXXTEST) U48Test.cpp -o U48Test.out

minposTest.out: minposTest.cpp v128.h minpos.h
	$(CXXTEST) minposTest.cpp -o minposTest.out

h8Test.out: h8Test.cpp minpos.h h8.h h8.dbg.o
	$(CXXTEST) h8.dbg.o h8Test.cpp -o h8Test.out

HeapTest.out: HeapTest.cpp H8.hpp Heap8.hpp StdMinHeap.hpp Heap8Aux.hpp Heap8Embed.hpp StdMinHeapMap.hpp FirstCompare.hpp U48.hpp minpos.h v128.h align.h h8.h h8.dbg.o
	$(CXXTEST) h8.dbg.o HeapTest.cpp -o HeapTest.out

HeapMapTest.out: HeapMapTest.cpp Heap8Aux.hpp Heap8Embed.hpp StdMinHeapMap.hpp StdMinHeap.hpp FirstCompare.hpp U48.hpp minpos.h v128.h align.h
	$(CXXTEST) HeapMapTest.cpp -o HeapMapTest.out

Sort8Test.out: Sort8Test.cpp Sort8.hpp v128.h Sort8.dbg.o
	$(CXXTEST) Sort8.dbg.o Sort8Test.cpp -o Sort8Test.out

h8.o: h8.c h8.h v128.h minpos.h align.h
	$(CC) $(OPT) -c h8.c

h8.dbg.o: h8.c h8.h v128.h minpos.h align.h
	$(CC) -c h8.c -o h8.dbg.o

Sort8.o: Sort8.cpp Sort8.hpp minpos.h
	$(CXX) $(OPT) -c Sort8.cpp

Sort8.dbg.o: Sort8.cpp Sort8.hpp minpos.h
	$(CXX) -c Sort8.cpp -o Sort8.dbg.o

clean:
	rm -f *.o *.out
	rm -rf *.out.*
