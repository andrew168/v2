#pragma once
#include "..\vk-all.h"
#include "string"

namespace aux
{

#define NOT_IMPLEMENTED() \
  MessageBox(NULL, "Not implemented", "error", MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2); \
  DebugBreak();

#define DEPRECIATEDBY(msg) \
  MessageBox(NULL, "replaced by "msg, "Depreciated", MB_ICONWARNING | MB_CANCELTRYCONTINUE); \
  DebugBreak();

#define DEPRECIATED() \
  MessageBox(NULL, "Depreciated", "Depreciated", MB_ICONWARNING | MB_CANCELTRYCONTINUE); \
  DebugBreak();

#define TODO(msg)   printf(msg); 


}