#include "console_handler.h"

#include "logger.h"

#include <vector>
#include <map>

void demo_callback()
{
  Logger::get()->log("Demo command called!", "Demo");
}

void exit_callback()
{
  exit(0);
}

void clear_callback()
{
  Logger::get()->clear_logs();
  Logger::get()->log("Cleared the console!", "Console");
}

typedef void (*func)();
struct command_t
{
  std::string short_description;
  func callback;

  std::vector<std::string> documentation{};
};

// name - command
std::map<std::string, command_t> commands = {
  {
    "exit", 
    {
      "Takes no arguments, exits BinaryHammer",
      exit_callback
    } 
  },
  {
    "clear",
    {
      "Clears the console",
      clear_callback
    }
  },
  {
    "demo",
    {
      "This command has no real use made for testing the command handler",
      demo_callback,
      {"This command takes no arguments and has no real use"}
    }
  }
};

void handle_console_command(std::string input)
{
  if (input.empty())
    return;

  if (input[0] == '.') {
    input = input.substr(1, input.length());
    size_t space = input.find(" ");

    std::string command = input.substr(0, space);
    std::vector<std::string> args;
    
    while (space != std::string::npos) {
      size_t next = input.find(" ", space + 1);
      if (next == std::string::npos)
        args.push_back(input.substr(space + 1, input.length() - 1));
      else {
        args.push_back(input.substr(space + 1, (next - 1) - (space)));
      }
      space = next;
    }

    if (command == "help") {
      if (args.empty()) {
        Logger::get()->log("Available Commands:", "Console");
        for (auto c : commands)
          Logger::get()->log("." + c.first + " - " + c.second.short_description);
        return;
      }
      
      Logger::get()->log("Documentation for: ." + args[0], "Console");
      if (commands[args[0]].documentation.empty()) {
        Logger::get()->log("- There is no documentation available for this command.", "Console");
      }
      else for (std::string doc : commands[args[0]].documentation)
        Logger::get()->log("- " + doc, "Console");

      return;
    }

    if (commands[command].callback) {
      commands[command].callback();
      return;
    }

    Logger::get()->log("Unknown command. Type \".help\" for help.", "Console");
    return;
  }
  else {
    Logger::get()->log(input, "Console");
  }
}