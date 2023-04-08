all: vm/vm.c

vm/vm.c:
	$(MAKE) -C vm

clean:
	rm bin/vm
