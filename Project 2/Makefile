a.out: main.o
	g++ -g -std=c++17 main.o
main.o: KNN.h MNIST.h imgFeat.h main.cpp
	g++ -g -std=c++17 -c -DASYNC main.cpp
#	g++ -g -std=c++17 -c -DTHREADS main.cpp
#	g++ -g -std=c++17 -c -DPTHREAD main.cpp
clean:
	rm a.out core *.o #*.csv
