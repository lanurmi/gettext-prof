gettext-profiler.so: gettext-profiler.c
	$(CC) -ggdb -Wall -Wextra -shared -fPIC -o $@ $< -ldl

all: gettext-profiler.so

clean:
	rm gettext-profiler.so

