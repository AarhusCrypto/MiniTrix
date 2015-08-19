#ifndef UTF8_STRING
#define UTF8_STRING
#include "string.h"
#include "utf8string.h"

String UTF8_String_datanew(OE oe, Data data);

String UTF8_String_cstrnew(OE oe, const char * s);

void UTF8_String_destroy(String * s);

#endif
