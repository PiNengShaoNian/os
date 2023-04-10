#include "../../include/linux/kernel.h"
#include "../../include/linux/traps.h"
#include "../../include/asm/io.h"
#include "../../include/linux/sched.h"
#include "../../include/linux/task.h"

// 定义PIT通道0、通道2和控制寄存器的地址
#define PIT_CHAN0_REG 0X40
#define PIT_CHAN2_REG 0X42
#define PIT_CTRL_REG 0X43

// 定义时钟频率为 100 Hz 和振荡器的频率 1193182 Hz
#define HZ 100
#define OSCILLATOR 1193182

// 每数CLOCK_COUNTER次产生一次中断，即1秒发生100次中断
#define CLOCK_COUNTER (OSCILLATOR / HZ)

// 定义中断的时间间隔，即 jiffy 的值为 10ms
#define JIFFY (1000 / HZ)

// 定义全局变量 jiffy 和 cpu_ticks
int jiffy = JIFFY;
int cpu_tickes = 0;

// 时钟初始化函数
void clock_init() {
    // bit 0 和 bit 1：计数器的工作模式，这里设置为0b00，表示工作在模式0（interrupt on terminal count）下，即计数器计数到零时触发中断；
    // bit 2：计数器的访问模式，这里设置为0b1，表示访问方式为"lo/hi byte"，即先写入低字节再写入高字节；
    // bit 3：计数器的工作方式，这里设置为0b1，表示工作在二进制计数方式下；
    // bit 4 和 bit 5：这两位是计数器的选择位，这里设置为0b00，表示选择计数器0；
    // bit 6 和 bit 7：这两位保留，必须设置为0。
    out_byte(PIT_CTRL_REG, 0b00110100);

    // 将 CLOCK_COUNTER 的低 8 位写入通道 0
    out_byte(PIT_CHAN0_REG, CLOCK_COUNTER & 0xff);

    // 将 CLOCK_COUNTER 的高 8 位写入通道 0
    out_byte(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff);
}

void clock_handler(int idt_index) {
    send_eoi(idt_index);

    cpu_tickes++;

    task_wakeup();
    do_timer();
}
