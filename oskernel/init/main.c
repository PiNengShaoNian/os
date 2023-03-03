void kernel_main(void) {
    int a = 0;

    // 将字符写入显存地址，屏幕上的显示会自动刷新
    char *video = (char*)0xb8000;
    *video = 'G';
}