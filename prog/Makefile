GCC = g++
SRC = main.cpp ihex.cpp row.cpp

prog: $(SRC)
	$(GCC) $(SRC) -lhidapi-libusb -o prog

clean:
	rm a.out

test: row.cpp
	$(GCC) row.cpp -o test

hexrange: ihex.cpp row.cpp
	$(GCC) ihex.cpp row.cpp -o hexrange
