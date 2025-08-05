#ifndef BINARY_H_
#define BINARY_H_
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../logger.h"

#ifdef WIN32
#include <Windows.h>
#else
#include "../win32_headers.h"
#endif

enum RESOURCE_TYPE : int {
  RESOURCE_TYPE_FORWARDER,
  RESOURCE_TYPE_EXPORT,
};

struct resource_t {
  std::string function;
  std::string value;
  int rva;
  RESOURCE_TYPE type;
};

class Binary {
private:
#ifdef _WIN32
  HANDLE m_file{};
  HANDLE m_mapping{};
  LPVOID m_view{};
#else

#endif

  size_t m_file_size{};

public:
  Binary() {}

  ~Binary() { close(); }

  Binary(std::string path) {
#ifdef _WIN32
    std::wstring tmp = std::wstring(path.begin(), path.end());

    m_file = CreateFileW(tmp.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (m_file == INVALID_HANDLE_VALUE) {
      Logger::get()->log("Error: Unable to get a handle to the file.",
                         "BinaryAnalyzer");
      return;
    }

    if (GetLastError() == ERROR_FILE_NOT_FOUND || !m_file) {
      Logger::get()->log("Error: File not found", "BinaryAnalyzer");
      throw 1;
    }

    LARGE_INTEGER file_size;
    GetFileSizeEx(m_file, &file_size);
    m_file_size = static_cast<size_t>(file_size.QuadPart);

    m_mapping =
        CreateFileMappingW(m_file, NULL, PAGE_READONLY, NULL, NULL, NULL);
    if (!m_mapping) {
      Logger::get()->log("Error: Unable to create a mapping to the file.",
                         "BinaryAnalyzer");
      throw 1;
    }

    m_view = MapViewOfFile(m_mapping, FILE_MAP_READ, NULL, NULL, NULL);
    if (!m_view) {
      Logger::get()->log("Error: Unable to create a view mapping to the file.",
                         "BinaryAnalyzer");
      throw 1;
    }

#else

#endif

    {
      std::string _ = "Read " + std::to_string(m_file_size) + " bytes.";
      Logger::get()->log(_, "BinaryAnalyzer");
    }

    const _IMAGE_DOS_HEADER *dos = get_dos();
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
      Logger::get()->log("Error: Not a valid PE binary. (DOS header)",
                         "BinaryAnalyzer");
      close();
      return;
    }

    const IMAGE_NT_HEADERS *nt_headers =
        get_ptr<IMAGE_NT_HEADERS>(dos->e_lfanew);
    if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
      Logger::get()->log("Error: Not a valid PE binary. (PE header)",
                         "BinaryAnalyzer");
      close();
      return;
    }

    {
      std::stringstream _;
      _ << "Magic: 0x" << std::hex << dos->e_magic;
      Logger::get()->log(_.str(), "BinaryAnalyzer");
    }

    {
      std::stringstream _;
      _ << "NT Signature: 0x" << std::hex << nt_headers->Signature;
      Logger::get()->log(_.str(), "BinaryAnalyzer");
    }

    {
      std::stringstream _;
      _ << "Entry Point Ptr: 0x" << std::hex
        << nt_headers->OptionalHeader.AddressOfEntryPoint;
      Logger::get()->log(_.str(), "BinaryAnalyzer");
    }
  }

  bool is_open() {
#ifdef _WIN32
    return m_view;
#else
    return false;
#endif
  }

  std::vector<resource_t> get_exports() {
    if (!is_open())
      return {};

    const auto nt = get_nt();
    if (!nt)
      return {};

#ifdef _WIN32
    IMAGE_DATA_DIRECTORY p_exports =
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    const IMAGE_EXPORT_DIRECTORY *exports =
        get_ptr<IMAGE_EXPORT_DIRECTORY>(p_exports.VirtualAddress);

    DWORD numberOfNames = exports->NumberOfNames;
    const DWORD *exportAddressTable =
        get_ptr<DWORD>(exports->AddressOfFunctions);
    const DWORD *nameOrdinalsPointer =
        get_ptr<DWORD>(exports->AddressOfNameOrdinals);
    const DWORD *exportNamePointerTable =
        get_ptr<DWORD>(exports->AddressOfNames);
#endif

    std::vector<resource_t> cleaned_exports;
    // FIXME once ur not sleep deprived and without the need to push the latest
    // codebase to the repo
    /*for (int i = 0; i < numberOfNames; ++i) {
      char* name = (char*)(reinterpret_cast<const uint8_t*>(m_view) +
    exportNamePointerTable[i]); resource_t resource; resource.function = name;
      cleaned_exports.push_back(resource);
    }*/

    return cleaned_exports;
  }

  void close() {
#ifdef _WIN32
    if (m_file)
      CloseHandle(m_file);
    m_file_size = 0;
    if (m_mapping)
      UnmapViewOfFile(m_mapping);
    m_view = nullptr;
#else

#endif
  }

  template <typename T> const T *get_ptr(size_t address) {
    if (!is_open())
      return {};

    if (address + sizeof(T) > m_file_size)
      return nullptr;

#ifdef _WIN32
    return reinterpret_cast<const T *>(
        reinterpret_cast<const uint8_t *>(m_view) + address);
#else
    return {};
#endif
  }

  std::vector<unsigned char> get_data(size_t address, size_t size) {
    if (!is_open() || address >= m_file_size || !size)
      return {};

    if (address + size > m_file_size)
      size = m_file_size - 1;

    std::vector<unsigned char> ret(size);
#ifdef _WIN32
    memcpy(ret.data(), static_cast<const unsigned char *>(m_view) + address,
           size);
#endif

    return ret;
  }

  const IMAGE_DOS_HEADER *get_dos() {
    if (!is_open())
      return {};

    auto dos = get_ptr<IMAGE_DOS_HEADER>(0);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
      Logger::get()->log("Error: Not a valid PE binary. (DOS header)",
                         "BinaryAnalyzer");
      close();
      return {};
    }
    return dos;
  }

  const IMAGE_NT_HEADERS *get_nt() {
    if (!is_open())
      return {};

    auto dos = get_ptr<_IMAGE_DOS_HEADER>(0);
    if (!dos) {
      Logger::get()->log("Error: Not a valid DOS binary.", "BinaryAnalyzer");
      close();
      return {};
    }

    const IMAGE_NT_HEADERS *nt = get_ptr<IMAGE_NT_HEADERS>(dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
      Logger::get()->log("Error: Not a valid PE binary. (NT header)",
                         "BinaryAnalyzer");
      close();
      return {};
    }
    return nt;
  }

  size_t get_entrypoint() {
    if (!is_open())
      return {};

    const auto *dos = get_dos();
    if (!dos)
      return {};

    const IMAGE_NT_HEADERS *nt = get_ptr<IMAGE_NT_HEADERS>(dos->e_lfanew);
    if (!nt || nt->Signature != IMAGE_NT_SIGNATURE)
      return 0;

    DWORD entry_point;
    if (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
      entry_point = reinterpret_cast<const IMAGE_NT_HEADERS32 *>(nt)
                        ->OptionalHeader.AddressOfEntryPoint;
    else if (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
      entry_point = reinterpret_cast<const IMAGE_NT_HEADERS64 *>(nt)
                        ->OptionalHeader.AddressOfEntryPoint;

    return {};
  }

  size_t get_binary_size() { return m_file_size; }
};

Binary open_binary;

#endif
