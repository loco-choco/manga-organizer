all:
	gcc src/*.c -I include/ -lpanel -lform -lmenu -lncurses -o out/manga-organizer
clean:
	rm -rf out/*
run:
	out/manga-organizer
