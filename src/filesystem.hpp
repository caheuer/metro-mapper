#include <string>
#include <vector>
#include <cstdint>

bool saveToFile(std::string filename, std::vector<uint8_t> data);
bool saveToFile(std::string filename, std::string data);
bool loadFromFile(std::string filetype, std::vector<uint8_t>* outData);
bool loadFromFile(std::string filetype, std::string* outData);