#ifndef INC_HANDLE_H_
#define INC_HANDLE_H_

#include <stdint.h>
#include <cassert>
#include <map>

void dbg(const char* fmt, ...);
bool fatal(const char* fmt, ...);

// ------------------------------------
struct TMsg;

#include "handle_decl.h"
#include "handle_manager.h"
#include "objs_manager.h"
#include "msgs.h"
#include "entity.h"

// Bind manager with the type_name
#define DECLARE_HANDLE_MANAGER(TObj, obj_type_name)                     \
  CObjsManager<TObj> om_##TObj;                                         \
  template<> uint32_t getType<TObj>() { return om_##TObj.getType(); }   \
  template<> const char* getTypeName<TObj>() { return obj_type_name; }  \
  template<> CObjsManager< TObj >* getObjsManager<TObj>() { return &om_##TObj; }  \

#endif
