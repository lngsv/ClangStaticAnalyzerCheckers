struct Buffer {
    std::vector<char> m_data;
};

void foo() {
    Buffer a;
    memset(&a, 0, sizeof(Buffer));
}
