// CS210 Final Project 

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <list>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

struct CityKey {
    string countryCode;
    string cityName;

    bool operator==(const CityKey& other) const {
        return countryCode == other.countryCode && cityName == other.cityName;
    }
};

struct CityKeyHash {
    size_t operator()(const CityKey& key) const {
        return hash<string>()(key.countryCode + "_" + key.cityName);
    }
};

// Base Cache Class 
class Cache {
protected:
    size_t capacity;

public:
    Cache(size_t cap) : capacity(cap) {}
    virtual bool get(const CityKey& key, string& population) = 0;
    virtual void put(const CityKey& key, const string& population) = 0;
    virtual void printCache() = 0;
    virtual ~Cache() = default;
};

// LFU Cache
class LFUCache : public Cache {
private:
    unordered_map<CityKey, pair<string, int>, CityKeyHash> cache;
    
public:
    LFUCache(size_t cap) : Cache(cap) {}

    bool get(const CityKey& key, string& population) override {
        auto it = cache.find(key);
        if (it == cache.end()) return false;
        it->second.second++; // Increment frequency
        population = it->second.first;
        return true;
    }

    void put(const CityKey& key, const string& population) override {
        if (cache.size() == capacity) {
            auto leastFreq = min_element(cache.begin(), cache.end(), 
                [](const auto& a, const auto& b) {
                    return a.second.second < b.second.second;
                });
            cache.erase(leastFreq->first);
        }
        cache[key] = {population, 1};
    }

    void printCache() override {
        cout << "\n[LFU Cache - City: Population (Frequency)]\n";
        for (const auto& pair : cache) {
            cout << pair.first.cityName << ", " << pair.first.countryCode 
                 << " => " << pair.second.first << " (" << pair.second.second << ")\n";
        }
    }
};

// FIFO Cache
class FIFOCache : public Cache {
private:
    list<pair<CityKey, string>> fifoList;

public:
    FIFOCache(size_t cap) : Cache(cap) {}

    bool get(const CityKey& key, string& population) override {
        for (auto& entry : fifoList) {
            if (entry.first == key) {
                population = entry.second;
                return true;
            }
        }
        return false;
    }

    void put(const CityKey& key, const string& population) override {
        if (fifoList.size() == capacity) fifoList.pop_front();
        fifoList.emplace_back(key, population);
    }

    void printCache() override {
        cout << "\n[FIFO Cache - Oldest First]\n";
        for (const auto& pair : fifoList) {
            cout << pair.first.cityName << ", " << pair.first.countryCode 
                 << " => " << pair.second << endl;
        }
    }
};

// Random Replacement Cache
class RandomCache : public Cache {
private:
    unordered_map<CityKey, string, CityKeyHash> cache;
    vector<CityKey> keys;

public:
    RandomCache(size_t cap) : Cache(cap) {}

    bool get(const CityKey& key, string& population) override {
        auto it = cache.find(key);
        if (it == cache.end()) return false;
        population = it->second;
        return true;
    }

    void put(const CityKey& key, const string& population) override {
        if (cache.size() == capacity) {
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> dis(0, keys.size() - 1);
            size_t randomIndex = dis(gen);
            cache.erase(keys[randomIndex]);
            keys.erase(keys.begin() + randomIndex);
        }
        cache[key] = population;
        keys.push_back(key);
    }

    void printCache() override {
        cout << "\n[Random Cache - Random Order]\n";
        for (const auto& key : keys) {
            cout << key.cityName << ", " << key.countryCode << " => " << cache[key] << endl;
        }
    }
};

// Utility Functions
string toLower(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool searchCity(const string& filename, const CityKey& key, string& population) {
    ifstream file(filename);
    if (!file.is_open()) return false;

    string line;
    getline(file, line); // Skip header

    while (getline(file, line)) {
        stringstream ss(line);
        string code, city, pop;
        getline(ss, code, ',');
        getline(ss, city, ',');
        getline(ss, pop, ',');

        if (toLower(code) == key.countryCode && toLower(city) == key.cityName) {
            population = pop;
            return true;
        }
    }
    return false;
}

// Main Program
int main() {
    const string filename = "world_cities.csv";
    Cache* cache = nullptr;
    int choice;

    cout << "City Population Lookup\nChoose Caching Strategy:\n1. LFU\n2. FIFO\n3. Random\nYour choice: ";
    cin >> choice;
    cin.ignore(); // Clear input buffer

    switch (choice) {
        case 1: cache = new LFUCache(10); break;
        case 2: cache = new FIFOCache(10); break;
        case 3: cache = new RandomCache(10); break;
        default: 
            cout << "Invalid choice. Exiting...\n"; 
            return 0;
    }

    while (true) {
        string city, country;
        cout << "\nEnter city name (or 'exit' to quit): ";
        getline(cin, city);
        if (city == "exit") break;

        cout << "Enter country code: ";
        getline(cin, country);

        CityKey key{toLower(country), toLower(city)};
        string population;

        if (cache->get(key, population)) {
            cout << "Cache Hit: Population of " << city << ", " << country << " is " << population << endl;
        } else if (searchCity(filename, key, population)) {
            cout << "File Search: Population of " << city << ", " << country << " is " << population << endl;
            cache->put(key, population);
        } else {
            cout << "City not found in dataset.\n";
        }

        cache->printCache();
    }

    delete cache;
    return 0;
}
