#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

bool flag = false;

void fine() {
    flag = true;
}

int64_t test1();
int64_t test2();
bool test3(bool);
void test4(int64_t *i);
void test5();

int main() {
    assert(test1() == 66);
    puts("Test 1 passed.");
    assert(test2() == 4);
    puts("Test 2 passed.");
    assert(test3(true));
    puts("Test 3a passed.");
    assert(!test3(false));
    puts("Test 3b passed.");
    int64_t i = 0x13374269c0dec001;
    test4(&i);
    assert(i == 42);
    puts("Test 4 passed.");
    test5();
    assert(flag);
    puts("Test 5 passed.");
    return 0;
}
