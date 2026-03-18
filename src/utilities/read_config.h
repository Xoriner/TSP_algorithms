#ifndef READ_CONFIG_H
#define READ_CONFIG_H

#include <map>
#include <string>
#include <fstream>
#include <sstream>

std::map<std::string, std::string> read_config(const std::string& filename);
#endif //READ_CONFIG_H
