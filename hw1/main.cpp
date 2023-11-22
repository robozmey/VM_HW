#include <iostream>
#include <chrono>
#include <algorithm>
#include <random>
#include <vector>
#include <map>
#include <utility>

const uint SIZE = 1 << 22;
const uint MIN_STRIDE = sizeof(uint);
const uint MAX_STRIDE = 1 << 16;
const uint MAX_ASSOCIATIVITY = 32;
const uint N = 100000;

const uint ASSOCIATIVITY_ITERATIONS = 10;
const uint LINE_SIZE_ITERATIONS = 200;

const double ASSOC_THRESHOLD = 1.2;
const double LINE_SIZE_THRESHOLD = 1.01;

uint a[SIZE];

void generate_chain(int spots, int stride0) {

    uint stride = stride0 / sizeof(uint);

    std::vector<uint> b(spots);
    std::iota(b.begin(), b.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(b.begin(), b.end(), g);

    for (int i = 0; i < spots; i++) {
        a[b[i % spots]*stride] = b[(i+1) % spots]*stride;
    }
}

long long sum = 0;

double measure(int len) {

    long long res = 0;

    unsigned int curr = 0;

    for (int i = 0; i < N; i++) {
        curr = a[curr];
//        sum = (curr + sum) % SIZE;
    }

    // for (int iteration = 0; iteration < ITERATIONS; iteration++) {

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) {
        curr = a[curr];
        sum = (curr + sum) % SIZE;
    }
    auto end = std::chrono::high_resolution_clock::now();

    res += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    // }

    return double(res) / N;

}


void get_assoc_it(uint& assoc, uint& cache_size) {
    int str_id = 0;

    std::vector<uint> assoc_count(MAX_ASSOCIATIVITY, 0);
    std::vector<uint> min_size(MAX_ASSOCIATIVITY, 0);

    for (int stride = MIN_STRIDE; stride < MAX_STRIDE; stride*=2, str_id++) {

        double pre_time = -1;

        for (int spots = 2; spots < MAX_ASSOCIATIVITY; spots+=2) {

            generate_chain(spots, stride);
            double time = measure(spots);

            generate_chain(spots-1, stride);
            // time += measure(spots-1);

            double k = time / pre_time;
            // std::cout << spots << " " << stride << " " << time << " " << k << std::endl;

            if (k > ASSOC_THRESHOLD) {
                uint assoc = spots;
                uint cache_size = assoc * stride;

                assoc_count[assoc]++;
                if (assoc_count[assoc] == 1) {
                    min_size[assoc] = cache_size;
                }

            }

            pre_time = time;            
        }

//        std::cout << str_id << " " << std_jumps << " " << sum.count() / spots_count * 10e11 << std::endl;
    }

    assoc = 0;
    cache_size = min_size[assoc];

    for (int i = 1; i < MAX_ASSOCIATIVITY; i++) {
        if (assoc_count[i] > assoc_count[assoc]) {
            assoc = i;
            cache_size = min_size[assoc];
        }
    }

}

void get_assoc(uint& assoc, uint& cache_size) {

    std::map<std::pair<uint, uint>, int> mp;

    for (int i = 0; i < ASSOCIATIVITY_ITERATIONS; i++) {
        get_assoc_it(assoc, cache_size);
        mp[{assoc, cache_size}]++;
    }

    assoc = 0;
    cache_size = 0;
    int mx = 0;
    for (auto [p, c] : mp) {
        if (c > mx) {
            mx = c;
            assoc = p.first;
            cache_size = p.second;
        }
    }

}

const uint CACHE_LINE_INDICES = 4;

void generate_chain_line(int assoc, int cache_size, int line_size) {

    int tag_offset = cache_size;
    int indices = cache_size / line_size;

    int spots = indices * assoc;

    std::vector<uint> b(spots);

    for (int i = 0; i < indices; i++) {
        for (int tag = 0; tag < assoc; tag++) {
            int line_index = i * line_size;
            int line_tag = tag * tag_offset + i % 2 * assoc * tag_offset;
            int line_i = 0;
            b[tag + i * assoc] = line_index + line_tag + line_i;
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(b.begin(), b.end(), g);

    for (int i = 0; i < spots; i++) {
        a[b[i]] = b[(i+1) % spots];
        // std::cout << a[b[i % spots]] << " ";
    }
    // std::cout << std::endl;

}

int get_line_size_it(int assoc, int cache_size) {

    double pre_time = -1;

    for (int line_size = MIN_STRIDE; line_size < cache_size; line_size*=2) {

        int spots = CACHE_LINE_INDICES * assoc;

        generate_chain_line(assoc, cache_size, line_size);
        double time = measure(spots);

        double k = time / pre_time;

        // std::cout << spots << " " << line_size << " " << time << " " << k << std::endl;

        if (k > LINE_SIZE_THRESHOLD) {
            return line_size;
        }

        pre_time = time;            

//        std::cout << str_id << " " << std_jumps << " " << sum.count() / spots_count * 10e11 << std::endl;
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
        std::cout << l << " " << c << std::endl;
        if (c > mx && l != -1) {
            mx = c;
            line_size = l;
        }
    }
    return line_size;
}




int main() {

    uint assoc = 0;
    uint cache_size = 0;

    get_assoc(assoc, cache_size);

    std::cout << "Cache size: " << cache_size << std::endl;
    std::cout << "Assoc: " << assoc << std::endl;

    int line_size = get_line_size(assoc, cache_size);

    std::cout << "Line size: " << line_size << std::endl;
    

    return 0;
}
