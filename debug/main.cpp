#include <iostream>
#include <fstream>
#include <string>
#include <signal.h>

bool isRunning = true;

void exit_handler(int signum)
{
  //if (signum == SIGINT || signum == SIGTERM || signum == SIGKILL) 
  //{
    isRunning = false;
  //}
}

int main(int argc, char** argv) {
  std::string file(argv[1]); 
  std::string line;
  std::ofstream out(file.c_str());
    
  signal(SIGINT, exit_handler);
  signal(SIGKILL, exit_handler);
  signal(SIGTERM, exit_handler);


  while (std::getline(std::cin, line) && isRunning) {
    std::cout << line << '\n';
//    std::cout.flush();
    out << line << '\n';
//    out.flush();
  } 

  return 0;
}


