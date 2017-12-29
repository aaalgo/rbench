#include <iostream>
#include <thread>
#include <fstream>
#include <chrono>
#include <boost/program_options.hpp>


using namespace std;

int main (int argc, char *argv[]) {
    int num_threads;
    int block_size;
    int num_tests;
    size_t max_gigs;
    string dev;

    {
        namespace po = boost::program_options;
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce help message.")
            ("threads,t", po::value(&num_threads)->default_value(1), "")
            ("block_size,b", po::value(&block_size)->default_value(4), " in KB")
            ("num_tests,n", po::value(&num_tests)->default_value(10000), "")
            ("max_gigs,G", po::value(&max_gigs)->default_value(100), " in GB")
            ("dev,f", po::value(&dev)->default_value("/dev/sda"), "")
            ;

        po::positional_options_description p;
        p.add("dev", 1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                         options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << "Usage:" << endl;
            cout << desc;
            cout << endl;
            return 0;
        }
    }
    srand(time(NULL));

    max_gigs *= 1024 * 1024 * 1024;
    block_size *= 1024;

    thread th[num_threads];

    auto start = chrono::high_resolution_clock::now();
    for (unsigned i = 0; i < num_threads; ++i) {
        th[i] = thread([&](){
                ifstream is(dev.c_str(), ios::binary);
                size_t nb = max_gigs / block_size;
                string buf;
                buf.resize(block_size);
                for (int i = 0; i < num_tests; ++i) {
                    size_t off = (rand() % nb) * block_size;
                    is.seekg(off, ios::beg);
                    is.read(&buf[0], block_size);
                    auto c = is.gcount();
                    if (c != block_size) {
                        cerr << c << " of " << block_size << " read at " << off << "." << endl;
                        throw 0;
                    }
                }
        });
    }
    for (auto &t: th) {
        t.join();
    }
    auto finish = chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(finish - start).count();
    double nmb = 1.0L * num_threads * num_tests * block_size / 1024/1024;
    double nmbps = nmb / elapsed;
    cout << nmb << " MB read in " << elapsed << "s" << endl;
    cout << nmbps << "MB/s." << endl;


    return 0;
}
