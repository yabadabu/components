#ifndef INC_HANDLE_OBJECTS_MANAGER_H_
#define INC_HANDLE_OBJECTS_MANAGER_H_

// ----------------------------------------------
template< class TObj >
class CObjsManager : public CHandleManager {
  TObj*  objs;

public:

  CObjsManager() : objs(nullptr) {
    obj_type_name = ::getTypeName< TObj >();
  }

  void allocateObjs(uint32_t amax_objs) {
    assert(objs == nullptr);
    registerAsHandleManager();
    assert(type > 0);
    createAllHandles(amax_objs);
    objs = new TObj[amax_objs];
  }

  // Ctor with multiple args
  template< class... Args >
  THandle createObj(Args&&... args) {

    assert(num_objs_in_use < max_objs_in_use);

    // Alloc a free handle. Using the next obj in the stack of objs
    uint32_t internal_idx = num_objs_in_use++;

    // The address we are going to use, based on the internal index
    TObj *obj = objs + internal_idx;

    // Call the original ctor of TObjs with the given args
    obj = ::new(obj)TObj(args...);

    // Find a free external index
    assert(next_free_handle_index != invalid_index);        // or we are full
    uint32_t external_idx = next_free_handle_index;

    // Update the next free handle for the next allocation
    next_free_handle_index = external_to_internal[external_idx].next_external_index;
    if (next_free_handle_index == invalid_index) {
      last_free_handle_index = invalid_index;
      assert(size() == capacity());
    }

    // Update external data
    TExternalData*d = external_to_internal + external_idx;
    d->next_external_index = invalid_index;
    d->internal_index = internal_idx;

    // How to get the handle from an internal index
    internal_to_external[internal_idx].external_index = external_idx;

    // Return the new handle
    return THandle(external_idx, d->current_age, getType());
  }

  // Destroy an object represented by the handle
  bool destroy(THandle h) override {
    assert(h.getType() == getType());

    // The handle is invalid
    if (h.getIndex() >= max_instances)
      return false;

    assert(h.getIndex() < max_instances);
    assert(external_to_internal);
    unsigned external_index_removed = h.getIndex();
    TExternalData* d = external_to_internal + external_index_removed;

    // The handle is old
    if (h.getAge() != d->current_age)
      return false;

    // Detach
    assert(d->next_external_index == invalid_index);

    // Call the dtor of the object
    TObj* obj = objs + d->internal_index;
    obj->~TObj();

    // If it's not the last object in the linear array of objects
    if (d->internal_index != size() - 1) {
      // Move the last object in the array to replace the deleted object
      // So all objects are still packed
      TObj* last_obj = objs + size() - 1;
      *obj = std::move(*last_obj);

      unsigned external_index_of_last_obj = internal_to_external[size() - 1].external_index;

      // Update the table internal to external. So the old last object keeps pointing to his external index
      // but now he is in space left by the deleted object
      internal_to_external[d->internal_index] = internal_to_external[size() - 1];

      // And the external table to point to the new position of the last object
      external_to_internal[external_index_of_last_obj].internal_index = d->internal_index;
    }

    // As we have removed the last object in the linear array of objects, his external_index is now invalid
    internal_to_external[size() - 1].external_index = invalid_index;

    // The object is gone
    assert(num_objs_in_use > 0);
    num_objs_in_use--;

    // The deleted handle will be the last to be removed
    // Link the current last to that handle, and update the current last
    if (next_free_handle_index != invalid_index) {
      TExternalData* dlast_free = external_to_internal + last_free_handle_index;
      dlast_free->next_external_index = external_index_removed;
      last_free_handle_index = external_index_removed;
    }
    else {
      // All objects were in use, so we have just one free handle
      next_free_handle_index = last_free_handle_index = external_index_removed;
    }

    // Once we are destroyed, we no longer have a real object associated
    d->internal_index = invalid_index;
    
    d->owner_handle = THandle();

    // This invalidates all existing copies of the old handle
    d->current_age++;

    return true;
  }

  TObj* getObjByHandle(THandle h) {
    // Nobody should ask me with the wrong type
    assert(h.getType() == getType());
    assert(h.getIndex() < max_instances);
    assert(external_to_internal);
    const TExternalData* d = external_to_internal + h.getIndex();
    assert(d);
    if (d->current_age == h.getAge())
      return objs + d->internal_index;
    return nullptr;
  }

  // Given an object pointer, return the handle associated to it if the object is valid
  inline THandle getHandleByObj(const TObj *obj) const {
    uintptr_t idx = obj - objs;
    if (idx < size()) {
      uint32_t external_index = internal_to_external[idx].external_index;
      const TExternalData* d = external_to_internal + external_index;
      return THandle(external_index, d->current_age, getType());
    }
    return THandle();
  }

  // ---------------------------------------------------------------
  // Use the copy ctor to generate a new object
  THandle clone(THandle h_src) {
    TObj *obj_src = getObjByHandle(h_src);
    if (!obj_src)
      return THandle();
    return createObj(*obj_src);
  }

};

template< class TObj >
CObjsManager< TObj >* getObjsManager();


#endif

