#include <stdio.h>

// 计算数组元素之和
int sum(int arr[], int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        total += arr[i];   // 在此行设断点可观察 total 累加过程
    }
    return total;
}

// 判断是否为偶数
int is_even(int x) {
    return x % 2 == 0;
}

int main() {
    int data[] = {3, 7, 2, 9, 4, 6};
    int n = sizeof(data) / sizeof(data[0]);

    int s = sum(data, n);
    printf("sum = %d\n", s);

    for (int i = 0; i < n; i++) {
        if (is_even(data[i])) {
            printf("data[%d] = %d (even)\n", i, data[i]);
        } else {
            printf("data[%d] = %d (odd)\n", i, data[i]);
        }
    }

    return 0;
}
