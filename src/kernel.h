
void _start(struct stivale_struct *bootloader_data) ;

inline void * memzero(void * s, uint64_t n) {
    for (int i = 0; i < n; i++) ((uint8_t*)s)[i] = 0;
}