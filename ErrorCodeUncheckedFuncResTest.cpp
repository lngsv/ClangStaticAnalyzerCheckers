namespace std { typedef int error_code; }

std::error_code memcpy_s();

std::error_code foo() {
    memcpy_s();

    std::error_code res = memcpy_s();
   
    std::error_code res2;
    res2 = memcpy_s();

    if (memcpy_s() == 3) {}

    return memcpy_s();
}

