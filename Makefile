all:
	g++ -std=c++11 -Werror -Wall -O3 main.cpp -lcurl -lcryptopp -lpthread -o adler32test

clean:
	rm -f adler32test
