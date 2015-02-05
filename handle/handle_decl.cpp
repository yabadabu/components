#include "handle.h"

// -----------------------------------------
// Forward the query to the handle manager
bool THandle::isValid() const {
  CHandleManager* hm = CHandleManager::getByType(type);
  return hm && hm->isValid(*this);
}

bool THandle::destroy() const {
  CHandleManager* hm = CHandleManager::getByType(type);
  if (!hm)
    return false;
  return hm->destroy(*this);
}

THandle THandle::getOwner() const {
  CHandleManager* hm = CHandleManager::getByType(type);
  if (hm)
    return hm->getOwner(*this);
  return THandle();
}

bool THandle::setOwner(THandle new_owner) const {
  CHandleManager* hm = CHandleManager::getByType(type);
  if (hm)
    return hm->setOwner(*this, new_owner);
  return false;
}
