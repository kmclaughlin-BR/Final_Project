// CS210: Final Project

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <list>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

struct TrieNode {
    unordered_map<char, TrieNode*> children;
    unordered_map<string, string> cityData; // map of "countryCode_cityName" -> population
    bool isEndOfWord = false;
};

class Trie {
private:
    TrieNode* root;

public:
    Trie() : root(new TrieNode()) {}

    void insert(const string& cityName, const string& countryCode, const string& population) {
        TrieNode* node = root;
        for (char ch : cityName) {
            if (!node->children[ch])
                node->children[ch] = new TrieNode();
            node = node->children[ch];
        }
        node->isEndOfWord = true;
        string key = countryCode + "_" + cityName;
        node->cityData[key] = population;
    }

    bool search(const string& cityName, const string& countryCode, string& population) {
        TrieNode* node = root;
        for (char ch : cityName) {
            if (!node->children[ch])
                return false;
            node = node->children[ch];
        }
        if (node->isEndOfWord) {
            string key = countryCode + "_" + cityName;
            auto it = node->cityData.find(key);
            if (it != node->cityData.end()) {
                population = it->second;
                return true;
            }
        }
        return false;
    }
};

// Cache Strategies (LFU, FIFO, Random)
class Cache {
protected:
    size_t capacity;
public:
    Cache(size_t cap) : capacity(cap) {}
    virtual bool get(const string& key, string& value) = 0;
    virtual void put(const string& key, const string& value) = 0;
    virtual ~Cache() = default;
};

class LFUCache : public Cache {
private:
    unordered_map<string, pair<string, int>> cache;

public:
    LFUCache(size_t cap) : Cache(cap) {}

    bool get(const string& key, string& value) override {
        auto it = cache.find(key);
        if (it == cache.end()) return false;
        it->second.second++;
        value = it->second.first;
        return true;
    }

    void put(const string& key, const string& value) override {
        if (cache.size() == capacity) {
            auto leastFreq = min_element(cache.begin(), cache.end(),
                [](const auto& a, const auto& b) { return a.second.second < b.second.second; });
            cache.erase(leastFreq->first);
        }
        cache[key] = {value, 1};
    }
};

class FIFOCache : public Cache {
private:
    list<pair<string, string>> fifoList;

public:
    FIFOCache(size_t cap) : Cache(cap) {}

    bool get(const string& key, string& value) override {
        for (auto& entry : fifoList) {
            if (entry.first == key) {
                value = entry.second;
                return true;
            }
        }
        return false;
    }

    void put(const string& key, const string& value) override {
        if (fifoList.size() == capacity) fifoList.pop_front();
        fifoList.emplace_back(key, value);
    }
};

class RandomCache : public Cache {
private:
    unordered_map<string, string> cache;
    vector<string> keys;

public:
    RandomCache(size_t cap) : Cache(cap) {}

    bool get(const string& key, string& value) override {
        auto it = cache.find(key);
        if (it == cache.end()) return false;
        value = it->second;
        return true;
    }

    void put(const string& key, const string& value) override {
        if (cache.size() == capacity) {
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> dis(0, keys.size() - 1);
            cache.erase(keys[dis(gen)]);
        }
        cache[key] = value;
        keys.push_back(key);
    }
};

string toLower(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

void loadTrieFromFile(Trie& trie, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file.\n";
        return;
    }

    string line;
    getline(file, line);

    while (getline(file, line)) {
        stringstream ss(line);
        string countryCode, cityName, population;
        getline(ss, countryCode, ',');
        getline(ss, cityName, ',');
        getline(ss, population, ',');

        trie.insert(toLower(cityName), toLower(countryCode), population);
    }
    file.close();
}

int main() {
    Trie trie;
    loadTrieFromFile(trie, "world_cities.csv");

    Cache* cache = nullptr;
    int choice;

    cout << "Choose Caching Strategy:\n1. LFU\n2. FIFO\n3. Random\nYour choice: ";
    cin >> choice;
    cin.ignore();

    if (choice == 1) cache = new LFUCache(10);
    else if (choice == 2) cache = new FIFOCache(10);
    else if (choice == 3) cache = new RandomCache(10);
    else return 0;

    string city, country, population;
    while (true) {
        cout << "\nEnter city name (or 'exit' to quit): ";
        getline(cin, city);
        if (city == "exit") break;

        cout << "Enter country code: ";
        getline(cin, country);

        string key = toLower(country) + "_" + toLower(city);
        if (cache->get(key, population)) cout << "Cache Hit: " << population << endl;
        else if (trie.search(toLower(city), toLower(country), population)) {
            cout << "Trie Search: " << population << endl;
            cache->put(key, population);
        } else cout << "City not found.\n";
    }

    delete cache;
    return 0;
}
