#include "../include/linux/fs.h"
#include "../include/linux/mm.h"
#include "../include/linux/task.h"
#include "../include/assert.h"

extern task_t *current;
extern task_t *wait_for_request;

// 注意：给硬盘发送identify请求不能用这个函数。这个请求硬盘响应太快，来不及执行task_block，会出问题
buffer_head_t *bread(int dev, int from, int count) {
    buffer_head_t *bh = kmalloc(sizeof(buffer_head_t));

    // 暂时一次最多读8个扇区，因为最多只能分配一个页的内存，即4096。
    if (count > 8) {
        panic("read sectors must less 8\n");
    }

    bh->data = kmalloc(512 * count);
    bh->dev = dev;
    bh->sector_from = from;
    bh->sector_count = count;

    ll_rw_block(READ, bh);

    wait_for_request = current;
    task_block(current);

    return bh;
}