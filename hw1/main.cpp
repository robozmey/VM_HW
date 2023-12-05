#include <iostream>
#include <chrono>
#include <algorithm>
#include <random>
#include <vector>
#include <map>
#include <utility>

const int  SIZE = 1 << 23;
const int  MIN_STRIDE = 512;
const int  MAX_STRIDE = 1 << 16;
const int  MAX_ASSOCIATIVITY = 32;

const int  ASSOCIATIVITY_ITERATIONS = 10;
const int  LINE_SIZE_ITERATIONS = 100;

const int MEASURE_N = 1 << 20;

const double ASSOC_THRESHOLD = 1.2;
const double LINE_SIZE_THRESHOLD = 1.2;

uint32_t a[SIZE];

std::random_device rd;
std::mt19937 g(rd());

void generate_chain(int spots, int stride0) {
    int stride = stride0 / sizeof(uint32_t);

    std::vector<int> b(spots);
    std::iota(b.begin(), b.end(), 0);

    std::shuffle(b.begin()+1, b.end(), g);

    for (int i = 0; i < spots; i++) {
        a[b[i % spots]*stride] = b[(i+1) % spots]*stride;
    }
}

long long sum = 0;

double measure(int len) {

    int curr = 0;

    int count = MEASURE_N;

    // Load into cache
    for (int i = 0; i < len; i++) {
        curr = a[curr];
        sum = (curr + sum) % SIZE;
    }

    curr = 0;

    // for (int iteration = 0; iteration < ITERATIONS; iteration++) {

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < count; i++) {
        curr = a[curr];
        sum += curr;
    }
    auto end = std::chrono::high_resolution_clock::now();

    long long res = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    // }

    return double(res) / count;

}


void get_assoc_it(int& assoc, int& cache_size) {
    int str_id = 0;

    std::vector<int> assoc_count(MAX_ASSOCIATIVITY, 0);
    std::vector<int> min_size(MAX_ASSOCIATIVITY, 0);

    for (int stride = MIN_STRIDE; stride < MAX_STRIDE; stride*=2, str_id++) {

        double pre_time = -1;

        int pre_spots = 1;

        for (int spots = 0; spots < MAX_ASSOCIATIVITY; spots+=2) {
            int real_spots = spots;
            if (real_spots == 0)
                real_spots = 1;

            generate_chain(real_spots, stride);
            double time = measure(real_spots);

//            generate_chain(real_spots-1, stride);
            // time += measure(real_spots-1);

            double k = time / pre_time;
//             std::cout << real_spots << " " << stride << " " << time << " " << k << std::endl;

            if (k > ASSOC_THRESHOLD) {
                int assoc = pre_spots;
                int cache_size = assoc * stride;

                assoc_count[assoc]++;
                if (assoc_count[assoc] == 1) {
                    min_size[assoc] = cache_size;
                }

            }

            pre_time = time;
            pre_spots = real_spots;
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

    int spots = indices * assoc;

    std::vector<uint32_t> b(spots);

    for (int index = 0; index < indices; index++) {
        for (int tag = 0; tag < assoc; tag++) {
            int field_index = index * line_size;
            int field_tag = (tag + index * assoc) * tag_offset; // + line_i * assoc * tag_offset;

//            if ((field_index | field_tag) != (field_index + field_tag)) {
//                std::cout << i << " " << tag << " - " << line_size << " " << tag_offset << " - " << (field_index | field_tag) << " " << (field_index + field_tag) << std::endl;
//                exit(1);
//            }

            b[tag + index * assoc] = (field_index + field_tag) >> sizeof(uint32_t); // + line_i;
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

    double max_k = -1;
    int max_line_size = -1;

    for (int line_size = 8; line_size <= cache_size / assoc; line_size*=2) {

        int spots = generate_chain_line(assoc, cache_size, line_size);
        double time = measure(spots);

        double k = pre_time / time;

        // std::cout << spots << " " << line_size << " " << time << " " << k << std::endl;

        if (k > max_k) {
            max_k = k;
            max_line_size = line_size;
        }

        if (k > LINE_SIZE_THRESHOLD) {
            return line_size;
        }

        pre_time = time;            

//        std::cout << str_id << " " << std_jumps << " " << sum.count() / spots_count * 10e11 << std::endl;
    }

    std::cout << max_k << " " << max_line_size << std::endl;

    return max_line_size;
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

    int assoc = 0;
    int cache_size = 0;

    get_assoc(assoc, cache_size);

    std::cout << "Cache size: " << cache_size << std::endl;
    std::cout << "Assoc: " << assoc << std::endl;

    int line_size = get_line_size(assoc, cache_size);

    std::cout << "Line size: " << line_size << std::endl;
    

    return 0;
}
