#include "handle.h"

// -----------------------------------------
// Register of handle manager. given a type, return the HM pointer
CHandleManager*             CHandleManager::all_handle_managers[max_types];
CHandleManager::MDictByName CHandleManager::all_handle_managers_by_name;
unsigned        CHandleManager::next_type = 1;
void CHandleManager::registerAsHandleManager() {
  type = next_type;
  all_handle_managers[type] = this;
  all_handle_managers_by_name[getTypeName()] = this;
  ++next_type;
}

// Given the type, get the manager pointer
CHandleManager* CHandleManager::getByType(uint32_t atype) {
  assert(atype < next_type);
  return all_handle_managers[atype];
}

// Given the name, get the manager pointer
CHandleManager* CHandleManager::getByName(const char* aname) {
  MDictByName::iterator it = all_handle_managers_by_name.find(aname);
  if (it != all_handle_managers_by_name.end())
    return it->second;
  return nullptr;
}

// Given the name, get the type
uint32_t getTypeByName(const char* name) {
  CHandleManager* hm = CHandleManager::getByName(name);
  if (hm)
    return hm->getType();
  dbg("Warning, components of name '%s' are not registered\n", name);
  return 0;
}

// -----------------------------------------
bool CHandleManager::setOwner(THandle who, THandle new_owner) {
  assert(who.getIndex() < max_instances);
  assert(who.getType() == getType());
  TExternalData* ed = external_to_internal + who.getIndex();
  // Maybe who is invalid
  if (ed->current_age != who.getAge())
    return false;
  // Each handle is only owned by another handle
  ed->owner_handle = new_owner;
  return true;
}

THandle CHandleManager::getOwner(THandle who) const {
  assert(who.getIndex() < max_instances);
  assert(who.getType() == getType());
  const TExternalData* ed = external_to_internal + who.getIndex();
  // Maybe who is invalid
  if (ed->current_age != who.getAge())
    return THandle();
  return ed->owner_handle;
}

