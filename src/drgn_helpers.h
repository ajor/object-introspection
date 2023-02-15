#pragma once

/*
 * This file is not intended to be permanent. As part of the transition to
 * type-graph based CodeGen, we need to break dependencies on legacy OICodeGen
 * from other parts of OI.
 *
 * Parts of OICodeGen used by other parts of OI, but  which only return drgn
 * data can be moved here.
 */

#include <string>

extern "C" {
#include <drgn.h>
}

namespace drgn_helpers {

void getDrgnArrayElementType(drgn_type *type, drgn_type **outElemType,
                             size_t &outNumElems);
std::string typeToName(drgn_type *type);

} // namespace drgn_helpers
