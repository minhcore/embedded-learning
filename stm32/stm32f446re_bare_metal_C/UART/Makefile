
CC = arm-none-eabi-gcc
CFLAGS = -c -mcpu=cortex-m4 -mthumb -std=gnu11 -DSTM32F446xx -IInc -Ichip_headers/CMSIS/Device/ST/STM32F4xx/Include -Ichip_headers/CMSIS/Include
LDFLAGS = -nostdlib -T stm32_ls.ld -Wl,-Map=gpio.map

SRCS = $(wildcard Src/*.c)
OBJS = $(SRCS:Src/%.c=%.o)
OBJS += stm32f446_startup.o

final: out.elf

%.o: Src/%.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
    
out.elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
    
flash:
	openocd -f board/st_nucleo_f4.cfg -c "program out.elf verify reset exit"
	
clean:
	rm -f *.o *.elf *.map
