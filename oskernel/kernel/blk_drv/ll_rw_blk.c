#include "../../include/linux/hd.h"
#include "../../include/linux/fs.h"

hd_request_t g_hd_request = {0};

static void make_request(int rw, buffer_head_t *bh) {
    g_hd_request.dev = bh->dev;
    g_hd_request.cmd = rw;
    g_hd_request.sector = bh->sector_from;
    g_hd_request.nr_sectors = bh->sector_count;
    g_hd_request.buffer = bh->data;
    g_hd_request.bh = bh;
}

void ll_rw_block(int rw, buffer_head_t *bh) {
    make_request(rw, bh);

    do_hd_request();
}