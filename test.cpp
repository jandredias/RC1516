#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(){
  int file = open("file1.pdf", O_RDONLY, S_IWRITE);
  perror("file");
  int fileDest = open("file2.pdf", O_WRONLY, S_IWRITE);
  perror("file2");
  int readStatus = 0;
  int writeStatus = 0;
  char buffer[1024];
  
  while(1){
    readStatus = read(file, buffer, 1024);
    if(readStatus == 0) break;
    writeStatus = write(fileDest, buffer, (readStatus < 1024) ? readStatus : 1024);
    if(writeStatus <= 0){ perror("writing: "); std::cout << "error writing on file " << std::endl; break; }
  }
  std::cout << "File written" << std::endl;

  return 0;
}
