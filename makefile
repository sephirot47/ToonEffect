all: main
	./main.exe
main: main.cpp
	g++ main.cpp -o main.exe -I/usr/local/include -L/usr/local/lib -lsfml-system -lsfml-window -lsfml-graphics
clean:
	rm -p *.o *.exe 
