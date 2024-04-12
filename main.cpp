#include <iostream>
#include <bitset>

using namespace std;

void invert_bits(uint8_t* ptr, size_t len, uint32_t bitmask) {
    std::bitset<8 * sizeof(uint8_t)> bits(static_cast<unsigned long>(bitmask));

    for (size_t i = 0; i < len; ++i) {
        uint8_t byte = *(ptr + i);
        auto inverted_byte = static_cast<uint8_t>(~((1 << bits.to_ulong()) & byte));
        *(ptr + i) = inverted_byte;
        bits >>= 8;
    }
}

void print_buffer(const uint8_t* buffer, size_t len) {
    cout << "Buffer Content:" << endl;
    for (size_t i = 0; i < len; ++i) {
        cout << hex << static_cast<int>(buffer[i]) << " ";
    }
    cout << dec << endl;
}

int main() {
    constexpr size_t len = 16;
    uint8_t buffer[len];
    fill(begin(buffer), end(buffer), 1);

    print_buffer(buffer, len);

    uint32_t mask = 0b11110111;

    invert_bits(buffer, len, mask);

    print_buffer(buffer, len);

    return 0;
}