all:
	gcc src/*.c -I include/ -lncurses -o out/manga-organizer
clean:
	rm -rf out/*
run:
	out/manga-organizer
