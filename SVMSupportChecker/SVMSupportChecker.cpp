#include <stdio.h>
#include <intrin.h>

int main() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 0x80000001);
    int ecx = cpuInfo[2];

    if (ecx & (1 << 2)) {
        printf("SVM is supported by this AMD CPU.\n");
    }
    else {
        printf("SVM is not supported.\n");
    }

    return 0;
}
