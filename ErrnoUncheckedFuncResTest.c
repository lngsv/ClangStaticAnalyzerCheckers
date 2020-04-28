typedef int errno_t;

errno_t memcpy_s();

errno_t foo() {
    memcpy_s(); // return value of memcpy_s is unchecked

    errno_t res = memcpy_s();
   
    errno_t res2;
    res2 = memcpy_s();

    return memcpy_s();
}

