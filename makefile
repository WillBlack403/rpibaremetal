ARM = /opt/arm-none-eabi/bin/arm-none-eabi
#ARM = /opt/aarch64-none-elf/bin/aarch64-none-elf

AARGS = --warn --fatal-warnings
#CARGS = -Wall -O3 -mcpu=cortex-a57+simd+fp
CARGS = -Wall -O3 -Ofast -mcpu=arm1176jzf-s -nostdlib -ffreestanding -nodefaultlibs
LINKARGS =
LIBGCC = /opt/arm-none-eabi/lib/gcc/arm-none-eabi/12.2.0/libgcc.a
INCLUDE = -I include

all: kernel.img

clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.img
	rm -f *.list
	rm -f *.i
	rm -f *.s
	rm -f *.c

gpio.o : src/GPIO/gpio.c
	$(ARM)-gcc $(CARGS) $(INCLUDE) -c src/GPIO/gpio.c -o gpio.o
timer.o : src/TIME/timer.c
	$(ARM)-gcc $(CARGS) $(INCLUDE) -c src/TIME/timer.c -o timer.o

_start.o :  src/_start.S
	$(ARM)-gcc $(CARGS) $(INCLUDE) -c  src/_start.S -o _start.o

interrupt.o : src/interrupt.c
	$(ARM)-gcc $(CARGS) $(INCLUDE) -c  src/interrupt.c -o interrupt.o

entry.o: src/entry.c
	$(ARM)-gcc $(CARGS) $(INCLUDE) -c src/entry.c -o entry.o

decimal.o: src/TIME/decimaltime.c
	$(ARM)-gcc $(CARGS) $(INCLUDE) -c src/TIME/decimaltime.c -o decimal.o

mailbox.o: src/mailbox.c
	$(ARM)-gcc $(CARGS) $(INCLUDE) -c src/mailbox.c -o mailbox.o
	$(ARM)-gcc $(CARGS) $(INCLUDE) -c src/mailbox.c -S -o mailbox.S

kernel.elf: link.ld _start.o entry.o gpio.o timer.o interrupt.o decimal.o mailbox.o
	$(ARM)-gcc $(CARGS) $(LINKARGS) _start.o entry.o gpio.o timer.o interrupt.o decimal.o mailbox.o -T link.ld -o kernel.elf
	$(ARM)-objdump -D kernel.elf > kernel.list

kernel.img:	 kernel.elf
	$(ARM)-objcopy kernel.elf -O binary kernel.img
