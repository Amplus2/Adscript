#include <chrono>
#include <iostream>
#include <stdint.h>

extern "C" int64_t ads_fib(int64_t);

int64_t cxx_fib(int64_t n) {
        if (n < 2) return n;

        return cxx_fib(n - 1) + cxx_fib(n - 2);
}

// not used currently
int64_t cxx_ifib(int64_t n) {
        if (n <= 0) return 0;
        else if (n == 1) return 1;

        int64_t a = 0, b = 1, c;
        for (int64_t i = 0; i < n; i++) {
                c = a + b;
                a = b;
                b = c;
        }
        return a;
}

using std::chrono::time_point;
using std::chrono::microseconds;
using std::chrono::high_resolution_clock;


template <class c, class d>
void print_result(const std::string& name, const time_point<c, d> &start, const time_point<c, d> &end) {
        const auto res = end - start;
        std::cout << name << res.count() << " µs" << std::endl;
}

int main() {
        const int64_t fr = 45;

        std::cout << "Calculating fib of " << fr << std::endl;

        const auto cxx_start = high_resolution_clock::now();
        std::cout << "C++ result:\t\t" << cxx_fib(fr) << std::endl;
        const auto cxx_end = high_resolution_clock::now();

        const auto ads_start = high_resolution_clock::now();
        std::cout << "Adscript result:\t" << ads_fib(fr) << std::endl;
        const auto ads_end = high_resolution_clock::now();

        print_result("Fib C++\t\t\t", cxx_start, cxx_end);
        print_result("Fib Adscript\t\t", ads_start, ads_end);
}
