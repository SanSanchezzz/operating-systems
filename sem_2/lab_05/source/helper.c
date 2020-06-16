#include <stdio.h>

int main() {
    FILE *file = fopen("alphabet.txt", "w");
    const char *cort = {"Abcdefghijklmnopqrstuvwxyz"};
    int size = sizeof(cort);
    for (int i = 0; i < 26; i++) {
        fprintf(file, "%c", cort[i]);
    }
    fclose(file);
    return 0;
}
