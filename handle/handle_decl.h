#ifndef INC_HANDLES_HANDLE_IMPL_
#define INC_HANDLES_HANDLE_IMPL_

// Public templates to be resolved in the future
template< class TObj > uint32_t getType();
template< class TObj > const char* getTypeName();
uint32_t getTypeByName(const char* name);

// ----------------------------------------------
struct THandle {
  static const unsigned n_index_bits = 12;    // 4096 instances
  static const unsigned n_type_bits = 7;
  static const unsigned n_counter_bits = 32 - n_index_bits - n_type_bits;
  static const unsigned max_types = 1 << n_type_bits;

  // ----------------------------------
  // Ctors
  THandle() : index(0), age(0), type(0) {}
  THandle(uint32_t aindex, uint32_t aage, uint32_t atype) : index(aindex), age(aage), type(atype) {}

  // ----------------------------------
  // Create a handle given the address on an obj
  template< class TObj >
  explicit THandle(const TObj *obj) {
    *this = getObjsManager<TObj>()->getHandleByObj(obj);
  }

  // Creates a new instance of object TObj
  template< class TObj, class... Args >
  static THandle create(Args&&... args) {
    return getObjsManager<TObj>()->createObj(args...);
  }

  bool destroy() const;

  THandle clone() const;

  // ----------------------------------
  // Conversion
  uint32_t asUnsigned() const { return *(uint32_t*)this; }
  operator bool() const { return isValid(); }

  // ----------------------------------
  // Given the handle, return the object pointer
  template< class TObj >
  inline operator TObj*() const {
    CObjsManager<TObj> *om = getObjsManager<TObj>();
    assert(om);
    if (type == 0)
      return nullptr;
    assert(::getType< TObj >() == type
      || fatal("Trying to convert handle of type '%s' to wrong type '%s'\n", om->getTypeName(), getTypeName<TObj>()));
    return om->getObjByHandle(*this);
  }

  // ----------------------------------
  // Operator
  bool operator==(THandle h) const {
    return index == h.index && age == h.age && type == h.type;
  }
  bool isValid() const;

  // ----------------------------------
  // Get's
  uint32_t getType() const { return type; }
  uint32_t getIndex() const { return index; }
  uint32_t getAge() const { return age; }

  // ----------------------------------
  // Owner
  THandle getOwner() const;
  bool setOwner(THandle new_owner) const;

private:
  uint32_t index : n_index_bits;
  uint32_t age : n_counter_bits;
  uint32_t type : n_type_bits;
};

#endif
