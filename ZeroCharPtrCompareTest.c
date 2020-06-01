int foo() {
    char* cp;
    if (cp == '\0') // должно быть *cp == '\0' 
        return 1;
    return 0;
}
