#include "../include/linux/mm.h"
#include "../include/asm/system.h"

/*
 * 每一个bucket_desc管理着一个物理页
 * 分配时的步骤
 * 1. 在每次分配时会查找该分配大小所属的桶
 * 2. 判断先前该桶内由bucket_desc所管理的页是否还有空闲的空间以处理该次请求
 * 3. 如果没有空闲的空间则分配一个物理页并且从free_bucket_desc取出一个bucket_desc来管理这个新分配的物理页
 * 4. 将第三步中的bucket_desc加入到该桶的chain中
 *
 * 释放时的步骤
 * 1. 根据要释放的对象得出他所属的物理页
 * 2. 根据要释放对象的大小找出它所属的桶
 * 3. 找出该物理页是那一个bucket_desc所管理的
 * 4. 将bucket_desc的引用计数减去1
 * 5. 如果该bucket_desc的引用计数为0了则释放该物理页并将bucket_desc放入到free_bucket_desc中
 * 6. 如果计数不为0则重新设置该bucket_desc的freeptr为要释放的对象，并将该对象链接到先前的freeptr
 */

struct bucket_desc {    /* 16 bytes */
    void *page;          // 管理的物理页
    struct bucket_desc *next;          // 下一个bucket地址
    void *freeptr;       // 下一个可供分配的
    unsigned short refcnt;         // 引用计数，释放物理页时要用
    unsigned short bucket_size;    // 每个桶的大小
};

struct _bucket_dir {    /* 8 bytes */
    int size;
    struct bucket_desc *chain;
};

struct _bucket_dir bucket_dir[] = {
        {16,   (struct bucket_desc *) 0},
        {32,   (struct bucket_desc *) 0},
        {64,   (struct bucket_desc *) 0},
        {128,  (struct bucket_desc *) 0},
        {256,  (struct bucket_desc *) 0},
        {512,  (struct bucket_desc *) 0},
        {1024, (struct bucket_desc *) 0},
        {2048, (struct bucket_desc *) 0},
        {4096, (struct bucket_desc *) 0},
        {0,    (struct bucket_desc *) 0}
};   /* End of list marker */

/*
 * This contains a linked list of free bucket descriptor blocks
 */
struct bucket_desc *free_bucket_desc = (struct bucket_desc *) 0;

/*
 * This routine initializes a bucket description page.
 */
static inline void init_bucket_desc() {
    struct bucket_desc *bdesc, *first;
    int i;

    first = bdesc = (struct bucket_desc *) get_free_page();
    if (!bdesc)
        return NULL;
    for (i = PAGE_SIZE / sizeof(struct bucket_desc); i > 1; i--) {
        bdesc->next = bdesc + 1;
        bdesc++;
    }
    /*
     * This is done last, to avoid race conditions in case
     * get_free_page() sleeps and this routine gets called again....
     */
    bdesc->next = free_bucket_desc;
    free_bucket_desc = first;
}

void *kmalloc(size_t len) {
    struct _bucket_dir *bdir;
    struct bucket_desc *bdesc;
    void *retval;

    /*
     * First we search the bucket_dir to find the right bucket change
     * for this request.
     */
    for (bdir = bucket_dir; bdir->size; bdir++)
        if (bdir->size >= len)
            break;
    if (!bdir->size) {
        printk("malloc called with impossibly large argument (%d)\n",
               len);
        return NULL;
    }
    /*
     * Now we search for a bucket descriptor which has free space
     */
    CLI    /* Avoid race conditions */
    for (bdesc = bdir->chain; bdesc; bdesc = bdesc->next)
        if (bdesc->freeptr)
            break;
    /*
     * If we didn't find a bucket with free space, then we'll
     * allocate a new one.
     */
    if (!bdesc) {
        char *cp;
        int i;

        if (!free_bucket_desc)
            init_bucket_desc();
        bdesc = free_bucket_desc;
        free_bucket_desc = bdesc->next;
        bdesc->refcnt = 0;
        bdesc->bucket_size = bdir->size;
        bdesc->page = bdesc->freeptr = (void *) (cp = (char *) get_free_page());
        if (!cp)
            return NULL;
        /* Set up the chain of free objects */
        for (i = PAGE_SIZE / bdir->size; i > 1; i--) {
            *((char **) cp) = cp + bdir->size;
            cp += bdir->size;
        }
        *((char **) cp) = 0;
        bdesc->next = bdir->chain; /* OK, link it in! */
        bdir->chain = bdesc;
    }
    retval = (void *) bdesc->freeptr;
    bdesc->freeptr = *((void **) retval);
    bdesc->refcnt++;
    STI    /* OK, we're safe again */
    return (retval);
}

void kfree_s(void *obj, int size) {
    void *page;
    struct _bucket_dir *bdir;
    struct bucket_desc *bdesc, *prev;
    bdesc = prev = 0;
    /* Calculate what page this object lives in */
    page = (void *) ((unsigned long) obj & 0xfffff000);
    /* Now search the buckets looking for that page */
    for (bdir = bucket_dir; bdir->size; bdir++) {
        prev = 0;
        /* If size is zero then this conditional is always false */
        if (bdir->size < size)
            continue;
        for (bdesc = bdir->chain; bdesc; bdesc = bdesc->next) {
            if (bdesc->page == page)
                goto found;
            prev = bdesc;
        }
    }
    return;
    found:
    CLI
    *((void **) obj) = bdesc->freeptr;
    bdesc->freeptr = obj;
    bdesc->refcnt--;
    if (bdesc->refcnt == 0) {
        /*
         * We need to make sure that prev is still accurate.  It
         * may not be, if someone rudely interrupted us....
         */
        if ((prev && (prev->next != bdesc)) ||
            (!prev && (bdir->chain != bdesc)))
            for (prev = bdir->chain; prev; prev = prev->next)
                if (prev->next == bdesc)
                    break;
        if (prev)
            prev->next = bdesc->next;
        else {
            if (bdir->chain != bdesc)
                return;
            bdir->chain = bdesc->next;
        }
        free_page((unsigned long) bdesc->page);
        bdesc->next = free_bucket_desc;
        free_bucket_desc = bdesc;
    }
    STI
    return;
}
