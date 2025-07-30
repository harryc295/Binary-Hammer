#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>
#include <cstdio>
#include <fstream>
#include <vector>

#include "singleton.h"

struct log_t
{
  std::string message{};
  std::string owner{};
};

class Logger: public singleton<Logger>
{
public:
  Logger()
  {
    open_logfile();

    time_t now = time(0);
    struct tm* local_time = localtime(&now);
    char* date_time = asctime(local_time);

    log("Session started at " + std::string(date_time), "Session Manager", true);
  }

  ~Logger()
  {
    time_t now = time(0);
    struct tm* local_time = localtime(&now);
    char* date_time = asctime(local_time);

    log("Session ended at " + std::string(date_time), "Session Manager", true);
  }

  void log(std::string message, std::string author = "Generic", bool noio = false)
  {
    if (!m_logfile.is_open()
      && !open_logfile()) {
      if (!noio)
        printf("[%s]: Unable send log, reason: Cannot open the logfile.\n", author);
      return;
    }

    if (!noio)
      printf("[%s]: %s\n", author.c_str(), message.c_str());
    m_logfile << "[" << author << "]: " << message << "\n";
    m_logs.push_back({ message, author });
  }

  std::vector<log_t> get_logs()
  {
    return this->m_logs;
  }

  /*
  * Clears the in-memory commands, not the logs inside the logfile
  */
  void clear_logs()
  {
    m_logs = std::vector<log_t>(0);
  }

private:
  std::vector<log_t> m_logs;
  std::ofstream m_logfile;

  bool open_logfile()
  {
    if (m_logfile.is_open())
      return true;
    
    m_logfile.open(std::string(
        "./binaryhammer.log"
      ));

    if (!m_logfile.is_open())
      return false;
  }
};

#endif // !LOGGER_H_