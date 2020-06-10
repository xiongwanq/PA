#include <unistd.h>
#include <stdio.h>

int main() {
  write(1, "test direct-write\n", 18);
  return 0;
}
