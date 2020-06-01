typedef int errno_t;

errno_t memcpy_s();

errno_t foo() {
    memcpy_s(); // предупреждение

    errno_t res = memcpy_s(); // инициализация
   
    errno_t res2;
    res2 = memcpy_s(); // присваивание

    if (memcpy_s() == 3) {} // сравнение

    return memcpy_s(); // оператор возврата
}

