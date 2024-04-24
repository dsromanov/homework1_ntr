#include <iostream>

int main() {
    int arr[10];
    for (int i = 0; i < 10; ++i) {
        arr[i] = i * i;
    }
    int *ptr = &arr[0];
    int fourthElement = *(ptr + 3);
    std::cout << "Четвертый элемент массива: " << fourthElement << std::endl;
    return 0;
}