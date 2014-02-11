#include <osal.h>




int main(int c, char **a) {
  OE oe = OperatingEnvironment_LinuxNew();
  char o[64] = {0};
  uint fd = 0;

  if (c != 2) {
    oe->p("Exactly one argument is expected");
    return 0;
  }

  osal_sprintf(o,"file %s",a[1]);
  fd = oe->open(o);
  if (fd) {
    oe->p("File is open");
    oe->close(fd);

  } else {
    oe->p("Failed to open file");
  }
  OperatingEnvironment_LinuxDestroy(&oe);
  return 0;
}
