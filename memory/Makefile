

all:
	gcc -o conctests -W conctests.c -O0 -lpthread `pkg-config --cflags --libs glib-2.0`

hugepages:
	gcc readhp.c -o reader
	gcc writehp.c -o writer
