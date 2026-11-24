CC = cc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic

vm: vm.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread vm.c -o vm $(LDFLAGS)

.PHONY: clean
clean:
	$(RM) vm
