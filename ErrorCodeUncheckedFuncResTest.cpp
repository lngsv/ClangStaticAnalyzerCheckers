namespace std { typedef int error_code; }

std::error_code memcpy_s();

std::error_code foo() {
    memcpy_s(); // предупреждение

    std::error_code res = memcpy_s(); // инициализация
   
    std::error_code res2;
    res2 = memcpy_s(); // присваивание

    if (memcpy_s() == 3) {} // сравнение

    return memcpy_s(); // оператор возврата
}

