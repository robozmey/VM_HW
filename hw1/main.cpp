#include <iostream>
#include <chrono>
#include <algorithm>
#include <random>
#include <vector>
#include <map>
#include <utility>
#include <unordered_map>
#include <set>

const int  SIZE = 1 << 23;
const int  MIN_STRIDE = 512;
const int  MAX_STRIDE = 1 << 16;
const int  MAX_ASSOCIATIVITY = 32;

const int  ASSOCIATIVITY_ITERATIONS = 20;
const int  LINE_SIZE_ITERATIONS = 20;

const int MEASURE_N = 1 << 20;

const double ASSOC_THRESHOLD = 1.6;
const double LINE_SIZE_THRESHOLD = 1.2;

uint32_t a[SIZE] alignas(8192);

std::random_device rd;
std::mt19937 g(rd());

void generate_chain(int spots, int stride0) {
    int stride = stride0 / sizeof(uint32_t);

    std::vector <int> b(spots, 0);

    a[0] = 0;
    for (int i = 1; i < spots; i++) {
        int next = g() % i;
        int prev = b[next];

        a[i * stride] = next * stride;
        a[prev * stride] = i * stride;
        b[next] = i;
        b[i] = prev;
    }
}

long long trash = 0;

double measure(int len) {

    long long sum = 0;
    trash = 0;

    int cur = 0;
    for (int i = 0; i < len; i++) {
        cur = a[cur];
    }
    trash ^= cur;

    cur = 0;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < MEASURE_N; i++) {
        cur = a[cur];
    }
    trash ^= cur;
    auto end = std::chrono::high_resolution_clock::now();
    sum += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    return double(sum) / MEASURE_N;

}


void get_assoc_it(int& assoc, int& cache_size) {
    int str_id = 0;

    std::unordered_map <long long, std::set<int>> size_jumps;
    std::unordered_map <long long, int> size_assoc;

    for (int stride = MIN_STRIDE; stride < MAX_STRIDE; stride*=2, str_id++) {

        double pre_time = -1;

        int pre_spots = 1;

        for (int spots = 0; spots < MAX_ASSOCIATIVITY; spots+=2) {
            int real_spots = spots;
            if (real_spots == 0)
                real_spots = 1;

            generate_chain(real_spots, stride);
            double time = measure(real_spots);

            double k = time / pre_time;

            int assoc0 = pre_spots;
            int cache_size0 = assoc0 * stride;

            if (k > ASSOC_THRESHOLD) {
                size_jumps[cache_size0].insert(assoc0);
                size_assoc[cache_size0] = assoc0;

                if (size_jumps[cache_size0 / 2].find(assoc0) != size_jumps[cache_size0 / 2].end()) {
                    size_assoc[cache_size0 / 2] = assoc0;
                }
            }

            pre_time = time;
            pre_spots = real_spots;
        }
    }

    assoc = -1;
    cache_size = -1;
    int max_jumps = -1;

    for (auto p : size_jumps) {
        long long current_cache_size = p.first;
        int current_jumps = p.second.size();

        if (max_jumps < current_jumps || max_jumps == current_jumps && current_cache_size < cache_size) {
            if (size_assoc.find(current_cache_size) != size_assoc.end()) {
                max_jumps = current_jumps;
                cache_size = current_cache_size;
                assoc = size_assoc[cache_size];
            }
        }
    }


}

void get_assoc(int& assoc, int& cache_size) {

    std::map<std::pair<int, int>, int> mp;

    for (int i = 0; i < ASSOCIATIVITY_ITERATIONS; i++) {
        get_assoc_it(assoc, cache_size);
        mp[{assoc, cache_size}]++;
    }

    assoc = 0;
    cache_size = 0;
    int mx = 0;
    for (auto [p, c] : mp) {
//        std::cout << p.first  << " " << p.second << " - " << c << std::endl;
        if (c > mx) {
            mx = c;
            assoc = p.first;
            cache_size = p.second;
        }
    }

}

int generate_chain_line(int assoc, int cache_size, int line_size) {

    int tag_offset = cache_size / assoc;
    int indices = tag_offset / line_size;

    int spots = indices * assoc * line_size / sizeof(uint32_t);

    std::vector<uint32_t> b(spots);

    for (int index = 0; index < indices; index++) {
        for (int tag = 0; tag < assoc; tag++) {
            for (int el = 0; el < line_size / sizeof(uint32_t); el++) {
                int field_index = index * line_size;
                int field_tag = (tag + index * assoc) * tag_offset; // + line_i * assoc * tag_offset;

                b[el + (tag + index * assoc) * line_size / sizeof(uint32_t)] = (field_index + field_tag) / sizeof(uint32_t) + el; // + line_i;
            }
        }
    }

    std::shuffle(b.begin()+1, b.end(), g);

    for (int i = 0; i < spots; i++) {
        a[b[i]] = b[(i+1) % spots];
        // std::cout << a[b[i % spots]] << " ";
    }
    // std::cout << std::endl;

    return spots;
}

int get_line_size_it(int assoc, int cache_size) {

    double pre_time = -1;

    for (int line_size = 8; line_size <= cache_size / assoc; line_size*=2) {

        int spots = generate_chain_line(assoc, cache_size, line_size);
        double time = measure(spots);

        double k = pre_time / time;

        if (k > LINE_SIZE_THRESHOLD) {
            return line_size * sizeof(uint32_t);
        }

        pre_time = time;
    }

    return -1;
}

int get_line_size(int assoc, int cache_size) {

    std::map<int, int> mp;

    for (int i = 0; i < LINE_SIZE_ITERATIONS; i++) {
        mp[get_line_size_it(assoc, cache_size)]++;
    }

    int mx = 0;
    int line_size = 0;
    for (auto [l, c] : mp) {
//        std::cout << l << " " << c << std::endl;
        if (c > mx && l != -1) {
            mx = c;
            line_size = l;
        }
    }
    return line_size;
}




int main() {

    int assoc = 0;
    int cache_size = 0;

    get_assoc(assoc, cache_size);

    std::cout << "Cache size: " << cache_size << std::endl;
    std::cout << "Assoc: " << assoc << std::endl;

    int line_size = get_line_size(assoc, cache_size);

    std::cout << "Line size: " << line_size << std::endl;


    return 0;
}