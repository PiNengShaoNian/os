BUILD=./build

CFLAGS:= -m32 # 32 位的程序
CFLAGS+= -masm=intel
CFLAGS+= -fno-builtin	# 不需要 gcc 内置函数
CFLAGS+= -nostdinc		# 不需要标准头文件
CFLAGS+= -fno-pic		# 不需要位置无关的代码  position independent code
CFLAGS+= -fno-pie		# 不需要位置无关的可执行程序 position independent executable
CFLAGS+= -nostdlib		# 不需要标准库
CFLAGS+= -fno-stack-protector	# 不需要栈保护
CFLAGS:=$(strip ${CFLAGS})

DEBUG:= -g

HD_IMG_NAME:= "hd.img"

all: ${BUILD}/boot/boot.o ${BUILD}/boot/setup.o ${BUILD}/system.bin
	$(shell rm -rf $(BUILD)/$(HD_IMG_NAME))
	bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $(BUILD)/$(HD_IMG_NAME)
	dd if=${BUILD}/boot/boot.o of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=0 count=1 conv=notrunc
	dd if=${BUILD}/boot/setup.o of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=1 count=2 conv=notrunc
	dd if=${BUILD}/system.bin of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=3 count=60 conv=notrunc

${BUILD}/system.bin: ${BUILD}/kernel.bin
	objcopy -O binary ${BUILD}/kernel.bin ${BUILD}/system.bin
	nm ${BUILD}/kernel.bin | sort > ${BUILD}/system.map

${BUILD}/kernel.bin: ${BUILD}/boot/head.o ${BUILD}/init/main.o ${BUILD}/kernel/asm/io.o ${BUILD}/kernel/chr_drv/console.o \
	${BUILD}/lib/string.o ${BUILD}/kernel/vsprintf.o ${BUILD}/kernel/printk.o ${BUILD}/kernel/gdt.o ${BUILD}/kernel/idt.o \
	${BUILD}/kernel/asm/interrupt_handler.o ${BUILD}/kernel/traps.o ${BUILD}/kernel/chr_drv/keyboard.o ${BUILD}/kernel/exception.o \
	${BUILD}/kernel/asm/clock_handler.o ${BUILD}/kernel/chr_drv/clock.o ${BUILD}/mm/memory.o ${BUILD}/kernel/kernel.o \
	${BUILD}/mm/mm_101012.o ${BUILD}/kernel/task.o ${BUILD}/kernel/sched.o ${BUILD}/mm/malloc.o ${BUILD}/kernel/asm/sched.o \
	${BUILD}/kernel/asm/kernel.o ${BUILD}/kernel/system_call.o ${BUILD}/lib/write.o ${BUILD}/lib/error.o ${BUILD}/kernel/system_call.o \
	${BUILD}/lib/stdio.o ${BUILD}/lib/stdlib.o ${BUILD}/kernel/asm/system_call.o ${BUILD}/lib/unistd.o ${BUILD}/kernel/asm/unistd.o \
	${BUILD}/kernel/kernel_thread.o ${BUILD}/kernel/assert.o ${BUILD}/kernel/shell.o
	ld -m elf_i386 $^ -o $@ -Ttext 0x1200

${BUILD}/mm/%.o: oskernel/mm/%.c
	$(shell mkdir -p ${BUILD}/mm)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/kernel/%.o: oskernel/kernel/%.c
	$(shell mkdir -p ${BUILD}/kernel)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/lib/%.o: oskernel/lib/%.c
	$(shell mkdir -p ${BUILD}/lib)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/kernel/chr_drv/%.o: oskernel/kernel/chr_drv/%.c
	$(shell mkdir -p ${BUILD}/kernel/chr_drv)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/kernel/asm/%.o: oskernel/kernel/asm/%.asm
	$(shell mkdir -p ${BUILD}/kernel/asm)
	nasm -f elf32 -g $< -o $@

${BUILD}/init/main.o: oskernel/init/main.c
	$(shell mkdir -p ${BUILD}/init)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/boot/head.o: oskernel/boot/head.asm
	nasm -f elf32 -g $< -o $@

${BUILD}/boot/%.o: oskernel/boot/%.asm
	$(shell mkdir -p $(BUILD)/boot)
	nasm $< -o $@

clean:
	$(shell rm -rf ${BUILD})

bochs: all
	bochs -q -f bochsrc

qemug: all
	qemu-system-i386 \
    -m 32M \
    -boot c \
    -hda ./build/hd.img \
    -s -S

qemu: all
	qemu-system-i386 \
	-m 32M \
	-boot c \
	-hda ./build/hd.img

# 生成的内核镜像给VBox、VMware用
vmdk: $(BUILD)/master.vmdk

$(BUILD)/master.vmdk: ./build/hd.img
	qemu-img convert -O vmdk $< $@
