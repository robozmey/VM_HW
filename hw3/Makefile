all: build/analisator

build/analisator: build/main.o build/byterun.o
	$(CXX) -g -m32 build/byterun.o build/main.o -o build/analisator

build/main.o: build src/main.cpp
	$(CXX) -O0 -std=c++20 -I include -I include -g -fstack-protector-all -m32 -c src/main.cpp -o build/main.o

build/bytefile.o: build src/byterun.c
	$(CXX) -O0 -w -I include -I include -g -fstack-protector-all -m32 -c src/byterun.c -o build/byterun.o

build:
	@mkdir -p build

clean:
	$(RM) -r *.a *.o *~ build