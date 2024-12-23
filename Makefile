release:
	gcc *.c -o busybeaver.exe
run: buildr
	./busybeaver.exe
debug:
	gcc -ggdb *.c -o debug.exe
rundebug: buildd
	gdb debug.exe
