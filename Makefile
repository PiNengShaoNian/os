BUILD=./build

all: ${BUILD}/boot/boot.o ${BUILD}/boot/setup.o

${BUILD}/boot/%.o: oskernel/boot/%.asm
	$(shell mkdir -p ${BUILD}/boot)
	nasm $< -o $@

clean:
	$(shell rm -rf ${BUILD})

bochs:
	bochs -q -f bochsrc

qemug:
	qemu-system-x86_64 -fda a.img -S -s

qemu:
	qemu-system-x86_64 -fda a.img
