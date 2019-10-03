
test: test.cpp montecarlo.h
	g++ -g -Wall -pthread test.cpp -o test
	./test

simgame: simgame.cpp montecarlo.h
	#g++ -g -Wall -pthread $< -o $@
	g++ -g -Wall -O3 -pthread $< -o $@

playgame: playgame.cpp montecarlo.h
	g++ -g -Wall -O3 -pthread $< -o $@

clean:
	rm test simgame playgame
