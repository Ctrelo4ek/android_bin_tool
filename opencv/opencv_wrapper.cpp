#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <getopt.h>
#include <numeric>
#include <sstream>
#include <fstream>
#include <iterator>
#include <stdexcept>

using namespace std;
using namespace cv;

// 版本信息
const string VERSION = "1.0.0";
const string BUILD_DATE = __DATE__;
const string BUILD_TIME = __TIME__;

// 模式枚举
enum class Mode { BLACKSCREEN, DIFF, INVALID };

// 配置结构体
struct Config {
    Mode mode = Mode::INVALID;
    vector<string> input_files;
    string sample_file;
    double threshold = -1.0;
    uint32_t target_pixel = 0xFF000000; // ARGB默认黑色
    int width = 0;
    int height = 0;
    string format;
    string merge = "no";
    bool verbose = false;
};

// 函数声明
void print_help();
void print_build_config();
Mode parse_mode(const string& mode_str);
Mat load_image(const string& path, const Config& config, bool is_sample);
double calculate_black_ratio(const Mat& img, uint32_t target);
double calculate_similarity(const Mat& img1, const Mat& img2);

int main(int argc, char** argv) {
    // 参数解析
    Config config;
    int opt;
    static struct option long_options[] = {
        {"mode", required_argument, 0, 'm'},
        {"input", required_argument, 0, 'i'},
        {"sample", required_argument, 0, 's'},
        {"threshold", required_argument, 0, 't'},
        {"pixel", required_argument, 0, 'p'},
        {"width", required_argument, 0, 'w'},
        {"height", required_argument, 0, 'h'},
        {"size", required_argument, 0, 0},
        {"format", required_argument, 0, 'f'},
        {"merge", required_argument, 0, 0},
        {"verbose", no_argument, 0, 'v'},
        {"buildconfig", no_argument, 0, 0},
        {"help", no_argument, 0, 0},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "m:i:s:t:p:w:h:f:v", long_options, nullptr)) != -1) {
        if (opt == -1) break;

        switch (opt) {
        case 'm': 
            config.mode = parse_mode(optarg);
            break;
        case 'i': {
            istringstream iss(optarg);
            string file;
            while (getline(iss, file, ','))
                config.input_files.push_back(file);
            break;
        }
        case 's': 
            config.sample_file = optarg;
            break;
        case 't': 
            config.threshold = stod(optarg);
            break;
        case 'p': 
            config.target_pixel = stoul(optarg, nullptr, 16);
            break;
        case 'w': 
            config.width = stoi(optarg);
            break;
        case 'h': 
            config.height = stoi(optarg);
            break;
        case 'f': 
            config.format = optarg;
            break;
        case 'v': 
            config.verbose = true;
            break;
        case 0: {
            string option = long_options[optind].name;
            if (option == "size") {
                string size = optarg;
                size_t pos = size.find('x');
                if (pos != string::npos) {
                    config.width = stoi(size.substr(0, pos));
                    config.height = stoi(size.substr(pos+1));
                }
            } else if (option == "merge") {
                config.merge = optarg;
            } else if (option == "buildconfig") {
                print_build_config();
                return 0;
            } else if (option == "help") {
                print_help();
                return 0;
            }
            break;
        }
        default: 
            cerr << "Invalid argument" << endl;
            return 1;
        }
    }

    // 模式校验
    if (config.mode == Mode::INVALID) {
        cerr << "Must specify valid mode" << endl;
        return 1;
    }

    // 加载样本图像（DIFF模式）
    Mat sample;
    if (config.mode == Mode::DIFF) {
        if (config.sample_file.empty()) {
            cerr << "Sample file required for diff mode" << endl;
            return 1;
        }
        sample = load_image(config.sample_file, config, true);
    }

    // 处理输入文件
    vector<double> results;
    for (const auto& file : config.input_files) {
        Mat input = load_image(file, config, false);
        double value = 0.0;

        switch (config.mode) {
        case Mode::BLACKSCREEN:
            value = calculate_black_ratio(input, config.target_pixel);
            break;
        case Mode::DIFF:
            value = calculate_similarity(input, sample);
            break;
        default: break;
        }

        results.push_back(value);
        
        // 非合并模式直接输出
        if (config.merge == "no") {
            if (config.threshold >= 0) {
                cout << (value >= config.threshold ? "1" : "0");
                if (config.verbose) cout << " " << file;
                cout << endl;
            } else {
                cout << value;
                if (config.verbose) cout << " " << file;
                cout << endl;
            }
        }
    }

    // 合并模式处理
    if (config.merge != "no") {
        double final = 0.0;
        if (config.merge == "max") {
            final = *max_element(results.begin(), results.end());
        } else if (config.merge == "min") {
            final = *min_element(results.begin(), results.end());
        } else if (config.merge == "avg") {
            final = accumulate(results.begin(), results.end(), 0.0) / results.size();
        }

        if (config.threshold >= 0) {
            cout << (final >= config.threshold ? "1" : "0") << endl;
            if (config.verbose) {
                for (size_t i = 0; i < results.size(); ++i) {
                    cout << (results[i] >= config.threshold ? "1" : "0") 
                         << " " << config.input_files[i] << endl;
                }
            }
        } else {
            cout << final << endl;
            if (config.verbose) {
                for (size_t i = 0; i < results.size(); ++i) {
                    cout << results[i] << " " << config.input_files[i] << endl;
                }
            }
        }
    }

    return 0;
}

// 图像加载函数
Mat load_image(const string& path, const Config& config, bool is_sample) {
    Mat img;
    if (config.format.empty() || config.format == "jpeg" || 
       config.format == "png" || config.format == "bmp") {
        img = imread(path, IMREAD_UNCHANGED);
        if (img.empty()) throw runtime_error("Failed to load image: " + path);
        
        // 统一转换为4通道ARGB
        if (img.channels() == 3) cvtColor(img, img, COLOR_BGR2BGRA);
        if (img.channels() == 1) cvtColor(img, img, COLOR_GRAY2BGRA);
    } else {
        // RAW格式处理
        if (config.width == 0 || config.height == 0)
            throw runtime_error("Size required for raw format");

        ifstream file(path, ios::binary);
        vector<uint8_t> buffer((istreambuf_iterator<char>(file)), 
                        {});
        
        if (config.format == "RGB") {
            img = Mat(config.height, config.width, CV_8UC3, buffer.data());
            cvtColor(img, img, COLOR_RGB2BGRA);
        } else if (config.format == "ARGB") {
            img = Mat(config.height, config.width, CV_8UC4, buffer.data());
        } else if (config.format == "YUV420") {
            // YUV420转BGR逻辑
            Mat yuv(config.height + config.height/2, config.width, CV_8UC1, buffer.data());
            cvtColor(yuv, img, COLOR_YUV2BGR_I420);
            cvtColor(img, img, COLOR_BGR2BGRA);
        }
    }
    return img;
}

// 黑屏检测计算
double calculate_black_ratio(const Mat& img, uint32_t target) {
    uint8_t a = (target >> 24) & 0xFF;
    uint8_t r = (target >> 16) & 0xFF;
    uint8_t g = (target >> 8) & 0xFF;
    uint8_t b = target & 0xFF;

    int count = 0;
    for (int i = 0; i < img.rows; ++i) {
        for (int j = 0; j < img.cols; ++j) {
            Vec4b pixel = img.at<Vec4b>(i, j);
            if (pixel[0] == b && pixel[1] == g && 
                pixel[2] == r && pixel[3] == a) {
                ++count;
            }
        }
    }
    return static_cast<double>(count) / (img.rows * img.cols);
}

// 相似度计算
double calculate_similarity(const Mat& img1, const Mat& img2) {
    Mat gray1, gray2;
    cvtColor(img1, gray1, COLOR_BGRA2GRAY);
    cvtColor(img2, gray2, COLOR_BGRA2GRAY);
    
    Mat diff;
    matchTemplate(gray1, gray2, diff, TM_CCOEFF_NORMED);
    return diff.at<float>(0, 0);
}

// 工具函数
Mode parse_mode(const string& mode_str) {
    if (mode_str == "0" || mode_str == "blackscreen") return Mode::BLACKSCREEN;
    if (mode_str == "1" || mode_str == "diff") return Mode::DIFF;
    return Mode::INVALID;
}

void print_build_config() {
    cout << "Version: " << VERSION 
         << "\nBuild: " << BUILD_DATE << " " << BUILD_TIME
         << "\nOpenCV Version: " << CV_VERSION 
         << "\nBuild Config:" << getBuildInformation() << endl;
}

void print_help() {
    cout << "Version " << VERSION << "\n"
         << "Compiled: " << __DATE__ << " " << __TIME__ << "\n"
         << "OpenCV Version" << CV_VERSION << "\n"
         << "Usage: opencv_wrapper.bin [options]\n"
         << "Options:\n"
         << "  -m, --mode MODE        Operation mode (0/blackscreen, 1/diff)\n"
         << "  -i, --input FILE       Input file path(s), comma separated\n"
         << "  -t, --threshold THRES  Set comparison threshold\n"
         << "  -h, --height HEIGHT    Set image height\n"
         << "  -w, --width WIDTH      Set image width\n"
         << "  --size WxH             Set width and height\n"
         << "  -p, --pixel PIXEL      Target pixel value (0xAARRGGBB)\n"
         << "  -f, --format FORMAT    Input format (RGB/ARGB/YUV444/YUV422/YUV420/jpeg/png/bmp)\n"
         << "  -v, --verbose          Enable verbose logging\n"
         << "  -s, --sample FILE      Sample image for diff mode\n"
         << "  --buildconfig          Print build configuration\n"
         << "  --help                 Show this help message\n"
         << "  --merge MODE           Result merge mode (no/max/min/avg)\n\n" << endl;
}