#include <string>
#include <fstream>
#include <vector>
#include <sstream>

#include "../logger.h"

class Binary
{
public:
  Binary() {}

  Binary(std::string path)
  {
    std::ifstream readin(path, std::ios::binary);
    if (!readin.is_open()) {
      Logger::get()->log("Fatal Exception: Could not open file.", "Binary Loader");
      __debugbreak();
      return;
    }
    
    readin.seekg(0, std::ios::end);
    size_t file_size = readin.tellg();
    readin.seekg(0, std::ios::beg);
    this->binary = std::vector<char>(file_size, '\0');
    readin.read(&this->binary[0], file_size);

    Logger::get()->log("Read " + std::to_string(file_size) + " bytes", "Binary Loader");

    this->init = true;
  }

  std::string get_readable_hexdump()
  {
    if (!init)
      return "";

    std::ostringstream oss;
    for (char c : binary)
      oss << std::hex << std::uppercase << (int)c << " ";
    return oss.str();
  }

  std::vector<char> get_binary()
  {
    return this->binary;
  }

private:
  bool init = false;
  std::vector<char> binary;
};

Binary open_binary;