buildr:
	gcc *.c -o busybeaver.exe
run: buildr
	./busybeaver.exe
buildd:
	gcc -ggdb *.c -o debug.exe
debug: buildd
	gdb debug.exe
