CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS = -O2 -s
LIBS = lib/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11

water: src/water.c src/water.vs src/water.fs src/sky.vs src/sky.fs
	g++ $(CFLAGS) $< $(LIBS) $(LDFLAGS) -o $@

clean:
	rm water

run: water
	./water