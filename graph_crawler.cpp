#include <iostream>
#include <queue>
#include <unordered_set>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>

// libcurl
#include <curl/curl.h>

// RapidJSON
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

// Helper for libcurl: write callback
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Fetch neighbors from API
std::string fetch_neighbors(const std::string& node) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    // Replace spaces with underscores for API call
    std::string encoded_node = node;
    for (char& c : encoded_node) {
        if (c == ' ') c = '_';
    }

    std::string url = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/" + encoded_node;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: "
                      << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

// Parse JSON response to extract neighbors
std::vector<std::string> parse_neighbors(const std::string& json) {
    std::vector<std::string> neighbors;
    rapidjson::Document doc;

    if (doc.Parse(json.c_str()).HasParseError()) {
        std::cerr << "JSON parse error: "
                  << rapidjson::GetParseError_En(doc.GetParseError())
                  << " (offset " << doc.GetErrorOffset() << ")\n";
        return neighbors;
    }

    if (doc.HasMember("neighbors") && doc["neighbors"].IsArray()) {
        for (auto& n : doc["neighbors"].GetArray()) {
            if (n.IsString()) {
                neighbors.push_back(n.GetString());
            }
        }
    }
    return neighbors;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <start_node> <depth>\n";
        return 1;
    }

    std::string start = argv[1];
    int maxDepth = std::stoi(argv[2]);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    auto start_time = std::chrono::high_resolution_clock::now();

    std::queue<std::pair<std::string, int>> q;
    std::unordered_set<std::string> visited;

    q.push({start, 0});
    visited.insert(start);

    while (!q.empty()) {
        auto [node, depth] = q.front(); q.pop();
        std::cout << node << " (depth " << depth << ")\n";

        if (depth < maxDepth) {
            std::string json = fetch_neighbors(node);
            auto neighbors = parse_neighbors(json);

            for (const auto& n : neighbors) {
                if (!visited.count(n)) {
                    visited.insert(n);
                    q.push({n, depth + 1});
                }
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cerr << "Execution time: " << elapsed.count() << "s\n";

    curl_global_cleanup();
    return 0;
}
