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
    this->m_binary = std::vector<char>(file_size, '\0');
    readin.read(&this->m_binary[0], file_size);

    Logger::get()->log("Read " + std::to_string(file_size) + " bytes", "Binary Loader");

    this->m_init = true;
  }

  std::string get_readable_hexdump()
  {
    if (!m_init)
      return "";

    std::ostringstream oss;
    for (char c : m_binary)
      oss << std::hex << std::uppercase << (int)c << " ";
    return oss.str();
  }

  std::vector<char> get_binary(size_t start, size_t size)
  {
    size_t _end = size - start - 1;
    if (start > m_binary.size() ||
        _end > m_binary.size()
      ) return {};

    return std::vector<char>(m_binary.begin() + start, m_binary.end() - _end);
  }

  size_t get_binary_size()
  {
    return m_binary.size();
  }

private:
  bool m_init = false;
  std::vector<char> m_binary;
};

Binary open_binary;