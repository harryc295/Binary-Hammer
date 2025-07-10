#include <string>
#include <cstdio>
#include <fstream>

#include "singleton.h"

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

    if (logfile.is_open())
      logfile.close();
  }

  void log(std::string string, std::string author = "Generic", bool noio = false)
  {
    if (!logfile.is_open()
      && !open_logfile()) {
      if (!noio)
        printf("[%s]: Unable send log, reason: Cannot open the logfile.\n");
      return;
    }

    if (!noio)
      printf("[%s]: %s\n", author.c_str(), string.c_str());
    logfile << "[" << author << "]: " << string << "\n";
  }

private:
  std::ofstream logfile;
  bool open_logfile()
  {
    if (logfile.is_open())
      return true;
    
    logfile.open(std::string(
        "C:\\Users\\panca\\Desktop\\binaryhammer.log"
      ));

    if (!logfile.is_open())
      return false;
  }
};