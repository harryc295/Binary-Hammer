#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstddef>
#include <iostream>

#include "../logger.h"
#include "headers.h"

#ifdef WIN32
#include <Windows.h>
#endif

class Binary
{
private:
  HANDLE m_file{};
  size_t m_file_size{};
  HANDLE m_mapping{};
  LPVOID m_view{};
public:
  Binary() {}

  Binary(std::string path)
  {
    std::wstring tmp = std::wstring(path.begin(), path.end());
    
    m_file = CreateFileW(
      tmp.c_str(),
      GENERIC_READ,
      FILE_SHARE_READ,
      NULL,
      OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL,
      NULL
    );

    if (m_file == INVALID_HANDLE_VALUE) {
      Logger::get()->log("Error: Unable to get a handle to the file.", "BinaryAnalyzer");
      throw 1;
    }
    
    if (GetLastError() == ERROR_FILE_NOT_FOUND || !m_file) {
      Logger::get()->log("Error: File not found", "BinaryAnalyzer");
      throw 1;
    }

    LARGE_INTEGER file_size;
    GetFileSizeEx(m_file, &file_size);
    m_file_size = static_cast<size_t>(file_size.QuadPart);

    m_mapping = CreateFileMappingW(
      m_file,
      NULL,
      PAGE_READONLY,
      NULL, NULL, NULL
    );
    if (!m_mapping) {
      Logger::get()->log("Error: Unable to create a mapping to the file.", "BinaryAnalyzer");
      throw 1;
    }

    m_view = MapViewOfFile(m_mapping, FILE_MAP_READ, NULL, NULL, NULL);
    if (!m_view) {
      Logger::get()->log("Error: Unable to create a view mapping to the file.", "BinaryAnalyzer");
      throw 1;
    }

    {
      std::string _ = "Read " + std::to_string(m_file_size) + " bytes.";
      Logger::get()->log(_, "BinaryAnalyzer");
    }
  }

  template<typename T>
  const T* get_ptr(size_t address) const {  // Note the added 'const' before T*
    if (!m_view || address + sizeof(T) > m_file_size) return nullptr;
    return reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(m_view) + address);
  }


  std::vector<unsigned char> get_data(size_t address, size_t size)
  {
    if (!m_view || address > m_file_size) return {};

    if (address + size > m_file_size)
      size = m_file_size - address;

    std::vector<unsigned char> ret;
    ret.resize(size);
    memcpy(ret.data(), static_cast<const char*>(m_view) + address, size);
    return ret;
  }

  const _IMAGE_DOS_HEADER* get_dos()
  {
    return get_ptr<_IMAGE_DOS_HEADER>(0);
  }

  /*std::string get_readable_hexdump()
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
    if (start + size > m_binary.size())
      return {};

    return std::vector<char>(m_binary.begin() + start, m_binary.begin() + size + start);
  }

  size_t get_binary_size()
  {
    return m_binary.size();
  }*/


};

Binary open_binary;