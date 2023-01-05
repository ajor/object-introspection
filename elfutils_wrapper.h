#pragma once

extern "C" {
#include <elfutils/libdwfl.h>
}

class ElfUtilsWrapper {
public:
  static int dwfl_module_getsymtab(Dwfl_Module *mod) {
    return ::dwfl_module_getsymtab(mod);
  }

  static const char *dwfl_module_getsym_info(Dwfl_Module *mod, int ndx, GElf_Sym *sym, GElf_Addr *addr, GElf_Word *shndxp, Elf **elfp, Dwarf_Addr *bias) {
    return ::dwfl_module_getsym_info(mod, ndx, sym, addr, shndxp, elfp, bias);
  }
};
