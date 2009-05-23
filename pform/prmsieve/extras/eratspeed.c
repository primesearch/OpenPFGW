#include "../erat/erat.h"

int main(int argc, char**argv)
{
  erat_speed(argv[1]?strtoul(argv[1], 0, 0):1000000000);
  return 0;
}

