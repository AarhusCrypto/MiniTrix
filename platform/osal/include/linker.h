#ifndef LINKER_H
#define LINKER_H

typedef struct _int_name_map_ {
  uint value;
  char * name;
} IntNameMapEntry;

typedef enum {
  CLZ32,
  CLZ64
} ElfClass;

extern IntNameMapEntry elf_class_names[];

typedef enum {
  LITEND,
  BIGEND
} ElfEndian;

extern IntNameMapEntry elf_endian_names[];

typedef enum {
  SysV=0,
  HPUX=1,
  NetBSD=2,
  Linux=3,
  Solaris=4,
  AIX=7,
  IRIX=8,
  FreeBSD=9,
  OpenBSD=0x0c
} ElfABI;

//extern IntnameMapEntry elf_ABI_names[];

typedef enum {
  SPARC=2,
  X86=3,
  MIPS=0x08,
  POWERPC=0x14,
  ARM=0x28,
  IA64=0x32,
  x8664=0x3E,
} EType;

extern IntNameMapEntry elf_etype_names[];

typedef struct _elf_reader_ {
  
  ElfClass(*get_class)(void);
  ElfEndian(*get_endian)(void);
  uint (*get_version)(void);
  
  
} *Elf;

Elf Elf_new(byte * data, uint ldata);

#endif
