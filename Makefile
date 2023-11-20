output: main.o 
	g++ -g -std=c++11 -Wall -Wextra -v main.o -o output

main.o: main.cpp
	g++ -c -std=c++11 -g main.cpp

clean:
	rm *.o output

         
