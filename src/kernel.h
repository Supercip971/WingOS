
void _start(struct stivale_struct *bootloader_data) ;

inline void * memzero(void * s, uint64_t n) {
    for (uint64_t i = 0; i < n; i++) ((uint8_t*)s)[i] = 0;
    return s;
}