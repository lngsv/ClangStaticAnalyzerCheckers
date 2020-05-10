typedef int errno_t;

errno_t memcpy_s();

errno_t foo() {
    memcpy_s();

    errno_t res = memcpy_s();
   
    errno_t res2;
    res2 = memcpy_s();

    if (memcpy_s() == 3) {}

    return memcpy_s();
}

