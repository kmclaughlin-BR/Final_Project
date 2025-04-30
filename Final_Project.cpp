//
// Created by mclfl on 4/29/2025.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <list>
#include <algorithm>

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

class LRUCache {
    size_t capacity;
    list<pair<CityKey, string>> lruList;
    unordered_map<CityKey, list<pair<CityKey, string>>::iterator, CityKeyHash> cache;

public:
    LRUCache(size_t cap) : capacity(cap) {}

    bool get(const CityKey& key, string& population) {
        auto it = cache.find(key);
        if (it == cache.end()) return false;

        lruList.splice(lruList.begin(), lruList, it->second);
        population = it->second->second;
        return true;
    }

    void put(const CityKey& key, const string& population) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            it->second->second = population;
            lruList.splice(lruList.begin(), lruList, it->second);
        } else {
            if (lruList.size() == capacity) {
                auto last = lruList.back();
                cache.erase(last.first);
                lruList.pop_back();
            }
            lruList.emplace_front(key, population);
            cache[key] = lruList.begin();
        }
    }

    void printCache() {
        cout << "\n[Cache - Most Recent First]\n";
        for (const auto& pair : lruList) {
            cout << pair.first.cityName << ", " << pair.first.countryCode << " => " << pair.second << endl;
        }
    }
};

string toLower(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool searchCity(const string& filename, const CityKey& key, string& population) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file.\n";
        return false;
    }

    string line;
    getline(file, line);

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

int main() {
    const string filename = "world_cities.csv";
    LRUCache cache(10);

    cout << "City Population Lookup\n";

    while (true) {
        string city, country;
        cout << "\nEnter city name (or 'exit' to quit): ";
        getline(cin, city);
        if (city == "exit") break;

        cout << "Enter country code: ";
        getline(cin, country);

        CityKey key{toLower(country), toLower(city)};
        string population;

        if (cache.get(key, population)) {
            cout << "Cache Hit: Population of " << city << ", " << country << " is " << population << endl;
        } else {
            if (searchCity(filename, key, population)) {
                cout << "File Search: Population of " << city << ", " << country << " is " << population << endl;
                cache.put(key, population);
            } else {
                cout << "City not found in dataset.\n";
            }
        }

        cache.printCache();
    }

    return 0;
}
