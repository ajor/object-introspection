#pragma once

#include "types.h"

class Visitor {
public:
  virtual ~Visitor() = default;

#define X(OI_TYPE_NAME) virtual void visit(OI_TYPE_NAME &) = 0;
OI_TYPE_LIST
#undef X
};
