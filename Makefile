release:
	gcc *.c -o busybeaver.exe
run: release
	./busybeaver.exe
debug:
	gcc -ggdb *.c -o debug.exe
rundebug: debug
	gdb debug.exe
