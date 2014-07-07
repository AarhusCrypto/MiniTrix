#include <coo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct _mycar_ {
  double fuel;
  void (*drive)(uint km);
} * MyCar;

COO_DCL(MyCar, void, drive, uint km);
COO_DEF_NORET_ARGS(MyCar, drive, uint km;, km) {
  if (this->fuel > 0.0) {
    this->fuel -= 0.5 + 0.2*km;
  }
}

MyCar MyCar_new(double fuel) {
  MyCar res = (MyCar)malloc(sizeof(*res));
  uint i = 0;
  for(i = 0;i < sizeof(*res); ++i) {
    ((char*)res)[i] = 0;
  }
  res->fuel = fuel;
  COO_ATTACH(MyCar, res, drive);
  return res;
}

int main(int c, char ** a) {
  
  Memory mem = LinuxMemoryNew();
  Memory specialMem = LinuxSpecialMemoryNew(mem);
  MyCar car = 0;

  coo_init(specialMem);
  car = MyCar_new(10.0);
  car->drive(10);

  coo_end();
  
}

