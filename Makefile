
test: test.cpp
	g++ -g -Wall -pthread test.cpp -o test
	./test

simgame: simgame.cpp
	g++ -g -Wall -pthread $< -o $@

playgame: playgame.cpp
	g++ -g -Wall -O3 -pthread $< -o $@

clean:
	rm test simgame playgame
