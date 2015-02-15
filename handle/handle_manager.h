#ifndef INC_HANDLE_HANDLE_MANAGER_H_
#define INC_HANDLE_HANDLE_MANAGER_H_

// ----------------------------------------------
class CHandleManager {

protected:
  static const unsigned max_instances = 1 << THandle::n_index_bits;
  static const unsigned max_types = 1 << THandle::n_type_bits;
  static const unsigned invalid_index = 0x7fff;

  // Using the index of a handle (public index) we can get the internal index
  struct TExternalData {
    unsigned internal_index;       // In order to get the real object in the linear list of objects
    unsigned current_age;          // The valid handle associated to the external index
    unsigned next_external_index;
    THandle  owner_handle;
    TExternalData() : internal_index(invalid_index), current_age(0), next_external_index(invalid_index){}
  };

  struct TInternalData {
    unsigned external_index;      // To known which external index corresponds an obj
  };

  // Access using external_id to get the internal index, handle, and other goodies
  TExternalData* external_to_internal;

  // Access using the index in the array of objects (internal index) to obtain the external index
  TInternalData* internal_to_external;

  // How many objects do we have
  uint32_t       max_objs_in_use;
  uint32_t       num_objs_in_use;

  // The type of objects we store and their name
  uint32_t       type;
  const char*    obj_type_name;

  // Begin and tail of the linked list of free handles
  uint32_t       next_free_handle_index;
  uint32_t       last_free_handle_index;

  // -----------------------------------------------------------
  // Memory allocation
  void createAllHandles(uint32_t ninstances) {
    assert(ninstances <= max_instances);
    assert(external_to_internal == nullptr);
    num_objs_in_use = 0;
    max_objs_in_use = ninstances;
    external_to_internal = new TExternalData[ninstances];
    internal_to_external = new TInternalData[ninstances];
    // Everything is free
    for (uint32_t i = 0; i < ninstances; ++i) {
      TExternalData* ed = external_to_internal + i;
      ed->internal_index = invalid_index;
      ed->current_age = 1;
      // Link each handle with the next
      if (i + 1 < ninstances)
        ed->next_external_index = i + 1;
      else
        ed->next_external_index = invalid_index;

      internal_to_external[i].external_index = invalid_index;
    }
    next_free_handle_index = 0;
    last_free_handle_index = ninstances - 1;
  }

  void createAllHandles() {
    if (internal_to_external) delete[] internal_to_external, internal_to_external = nullptr;
    if (external_to_internal) delete[] external_to_internal, external_to_internal = nullptr;
  }

  struct TComparator {
    bool operator()(const char* a, const char*b) const {
       return strcmp(a, b) < 0;
    }
  };

  // Register methods
  static CHandleManager* all_handle_managers[max_types];
  typedef std::map<const char*, CHandleManager*, TComparator > MDictByName;
  static MDictByName     all_handle_managers_by_name;
  static unsigned        next_type;
  void registerAsHandleManager();
public:
  static CHandleManager* getByType(uint32_t atype);
  static CHandleManager* getByName(const char* aname);

public:
  CHandleManager()
    : external_to_internal(nullptr)
    , max_objs_in_use(0)
    , num_objs_in_use(0)
    , type(0)
    , obj_type_name(nullptr)
    , next_free_handle_index(invalid_index)
    , last_free_handle_index(invalid_index)
  {  }
  virtual ~CHandleManager() {}

  uint32_t getType() const {
    assert(type || fatal( "Handle manager of objects of type '%s' has not been initialized\n", obj_type_name ));
    return type;
  }
  const char* getTypeName() const { return obj_type_name; }
  size_t size() const { return num_objs_in_use; }
  size_t capacity() const { return max_objs_in_use; }

  // Validate if the given handle is valid
  bool isValid(THandle h) const {
    // Nobody should ask with the wrong type
    assert(h.getType() == getType());
    assert(h.getIndex() < max_instances);
    assert(external_to_internal);
    const TExternalData* d = external_to_internal + h.getIndex();
    assert(d);
    return h.getAge() == d->current_age;
  }

  // We need the virtual method because we don't know the type of objects to 'destroy'
  virtual bool destroy(THandle h) = 0;
  virtual THandle clone(THandle h) = 0;

  // Owner
  bool setOwner(THandle who, THandle new_owner);
  THandle getOwner(THandle who) const;

  // Quick dump
  void dumpInternals() const {
    dbg(" IDX   Int  Next  Age    Owner    External. next_free:%d Last:%d Used %d/%d\n", next_free_handle_index, last_free_handle_index, size(), capacity());
    for (unsigned i = 0; i < max_objs_in_use; ++i) {
      TExternalData* ed = external_to_internal + i;
      TInternalData* id = internal_to_external + i;
      dbg("%4d  %04x  %04x %04x %08x    %04x\n", i, ed->internal_index, ed->next_external_index, ed->current_age, ed->owner_handle, id->external_index);
    }
  }
};


#endif
