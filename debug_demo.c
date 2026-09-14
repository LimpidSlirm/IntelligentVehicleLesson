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

// 加法
int add(int a, int b) {
    return a + b;
}

// 减法
int sub(int a, int b) {
    return a - b;
}

// 乘法
int mul(int a, int b) {
    return a * b;
}

// 除法（整数除法，处理除零）
int divide(int a, int b) {
    if (b == 0) {
        printf("Error: divide by zero\n");
        return 0;
    }
    return a / b;
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

    // ===== 加减乘除测试 =====
    int a = 20, b = 4;
    printf("\n--- Arithmetic ---\n");
    printf("%d + %d = %d\n", a, b, add(a, b));
    printf("%d - %d = %d\n", a, b, sub(a, b));
    printf("%d * %d = %d\n", a, b, mul(a, b));
    printf("%d / %d = %d\n", a, b, divide(a, b));

    // 除零测试
    printf("%d / %d = %d\n", a, 0, divide(a, 0));

    return 0;
}
