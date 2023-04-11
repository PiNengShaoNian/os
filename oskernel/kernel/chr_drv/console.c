#include "../../include/asm/io.h"
#include "../../include/linux/tty.h"
#include "../../include/string.h"

#define CRT_ADDR_REG 0x3D4 // CRT索引寄存器, 存取操作指令的地方
#define CRT_DATA_REG 0x3D5 // CRT数据寄存器, 读出数据的地方

#define CRT_START_ADDR_H 0xC // 显示内存起始位置 - 高位
#define CRT_START_ADDR_L 0xD // 显示内存起始位置 - 低位
#define CRT_CURSOR_H 0xE     // 光标位置 - 高位
#define CRT_CURSOR_L 0xF     // 光标位置 - 低位

#define MEM_BASE 0xB8000              // 显卡内存起始位置
#define MEM_SIZE 0x4000               // 显卡内存大小
#define MEM_END (MEM_BASE + MEM_SIZE) // 显卡内存结束位置
#define WIDTH 80                      // 屏幕文本列数
#define HEIGHT 25                     // 屏幕文本行数
#define ROW_SIZE (WIDTH * 2)          // 每行字节数
#define SCR_SIZE (ROW_SIZE * HEIGHT)  // 屏幕字节数

#define ASCII_NUL 0x00
#define ASCII_ENQ 0x05
#define ASCII_BEL 0x07 // \a
#define ASCII_BS 0x08  // \b
#define ASCII_HT 0x09  // \t
#define ASCII_LF 0x0A  // \n
#define ASCII_VT 0x0B  // \v
#define ASCII_FF 0x0C  // \f
#define ASCII_CR 0x0D  // \r
#define ASCII_DEL 0x7F

static uint screen; // 当前显示器开始的内存位置
static uint pos; // 记录当前光标的内存位置
static uint x, y; // 当前光标的坐标

// 设置当前显示器开始的位置
static void set_screen() {
    // 注意由于是两个字节为一个显示字，所以实际将地址发送到端口时
    // 需要将(screen - MEM_BASE)所得的值处以2才是真实的地址

    // 要进行设置显存地址高8位操作
    out_byte(CRT_ADDR_REG, CRT_START_ADDR_H);
    // 设置显存高地址8位(注意需要除以2,也就是多右移1位)
    out_byte(CRT_DATA_REG, ((screen - MEM_BASE) >> 9) & 0xff);
    // 要进行设置显存地址低8位操作
    out_byte(CRT_ADDR_REG, CRT_START_ADDR_L);
    // 设置显存低地址8位(注意需要除以2,也就是多右移1位)
    out_byte(CRT_DATA_REG, ((screen - MEM_BASE) >> 1) & 0xff);
}

static void set_cursor() {
    // 要进行设置光标高8位操作
    out_byte(CRT_ADDR_REG, CRT_CURSOR_H);
    // 设置光标高地址8位(注意需要除以2,也就是多右移1位)
    out_byte(CRT_DATA_REG, ((pos - MEM_BASE) >> 9) & 0xff);
    // 要进行设置光标低8位操作
    out_byte(CRT_ADDR_REG, CRT_CURSOR_L);
    // 设置光标地址低8位(注意需要除以2,也就是多右移1位)
    out_byte(CRT_DATA_REG, ((pos - MEM_BASE) >> 1) & 0xff);
}

void console_clear() {
    // 将屏幕和光标位置设为显存的起始位置
    screen = MEM_BASE;
    pos = MEM_BASE;
    x = y = 0;
    set_cursor();
    set_screen();

    u16 *ptr = (u16 *) MEM_BASE;
    while (ptr < MEM_END) {
        *ptr++ = 0x0720;
    }
}

void console_init(void) {
    console_clear();
}

// 将屏幕向上滚动一行
static void scroll_up() {
    if (screen + SCR_SIZE + ROW_SIZE < MEM_END) {
        // 还能向下滚动
        u32 *ptr = (u32 *) (screen + SCR_SIZE);
        // 将即将要从下部滚动进屏幕的那行值为空
        for (size_t i = 0; i < WIDTH; i++) {
            *ptr++ = 0x0720;
        }
        // 将屏幕显示内存向前移动一行的大小
        screen += ROW_SIZE;
        pos += ROW_SIZE;
    } else {
        // 不能向下滚动了，将当前屏幕的显示内容，移动到第一页
        memcpy(MEM_BASE, screen, SCR_SIZE);
        pos -= (screen - MEM_BASE);
        screen = MEM_BASE;
    }
    set_screen();
}


static void command_lf() {
    if (y + 1 < HEIGHT) {
        y++;
        pos += ROW_SIZE;
        return;
    }
    scroll_up();
}

static void command_cr() {
    pos -= (x << 1);
    x = 0;
}

static void command_bs() {
    if (x) {
        x--;
        pos -= 2;
        *(u16 *) pos = 0x0720;
    }
}

static void command_del() {
    *(u16 *) pos = 0x0720;
}

int console_write(char *buf, u32 count) {
    CLI

    int write_size = 0;
    char ch;
    char *ptr = (char *) pos;
    while (count--) {
        write_size++;
        ch = *buf++;
        switch (ch) {
            case ASCII_NUL:
                break;
            case ASCII_BEL:
                break;
            case ASCII_BS:
                command_bs();
                break;
            case ASCII_HT:
                break;
            case ASCII_LF:
                command_lf();
                command_cr();
                break;
            case ASCII_VT:
                break;
            case ASCII_FF:
                command_lf();
                break;
            case ASCII_CR:
                command_cr();
                break;
            case ASCII_DEL:
                command_del();
                break;
            default:
                if (x >= WIDTH) {
                    x -= WIDTH;
                    pos -= ROW_SIZE;
                    command_lf();
                }

                *ptr = ch;
                ptr++;
                *ptr = 0x07;
                ptr++;

                pos += 2;
                x++;
                break;
        }
    }
    set_cursor();

    STI

    return write_size;
}