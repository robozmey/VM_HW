all: build/interpretator

build/interpretator: build/main.o build/runtime.o build/gc_runtime.o build/bytefile.o build/interpretator.o
	$(CXX) -g -m32 build/runtime.o build/gc_runtime.o build/bytefile.o build/interpretator.o build/main.o -o build/interpretator

build/main.o: build src/main.cpp
	$(CXX) -O3 -std=c++20 -I include -I include -g -fstack-protector-all -m32 -c src/main.cpp -o build/main.o

build/interpretator.o: build src/interpretator.cpp
	$(CXX) -O3 -std=c++20 -I include -I include -g -fstack-protector-all -m32 -c src/interpretator.cpp -o build/interpretator.o

build/bytefile.o: build src/bytefile.cpp
	$(CXX) -O3 -w -I include -I 3rd-party/include -g -fstack-protector-all -m32 -c src/bytefile.cpp -o build/bytefile.o

build/gc_runtime.o: build src/gc_runtime.s
	$(CC) -O3 -w -I src/include -g -fstack-protector-all -m32 -c 3rd-party/src/gc_runtime.s -o build/gc_runtime.o

build/runtime.o: build src/runtime.c
	$(CC) -O3 -w -I include -I include -g -fstack-protector-all -m32 -c 3rd-party/src/runtime.c -o build/runtime.o

build:
	@mkdir -p build

clean:
	$(RM) -r *.a *.o *~ build