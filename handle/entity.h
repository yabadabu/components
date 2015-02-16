#ifndef INC_HANDLE_ENTITY_H_
#define INC_HANDLE_ENTITY_H_

// ------------------------------------------------------
class TEntity {
  THandle comps[THandle::max_types];
  char name[32];
public:
  const char* getName() const { return name; }
  TEntity() { name[0] = 0x00; }
  TEntity(const char* aname) { strcpy(name, aname); }
  
  // ---------------------------------------------------------
  ~TEntity() {

    // Destroy all the components we own
    for (int i = 0; i < THandle::max_types; ++i)
      if (comps[i])
        comps[i].destroy();
  
  }

  // Creates a new instance of object TObj
  template< class TObj, class... Args >
  THandle add(Args&&... args) {
    THandle new_comp = getObjsManager<TObj>()->createObj(args...);
    return addHandle(new_comp);
  }

  THandle addHandle(THandle new_comp) {
    unsigned comp_type = new_comp.getType();
    assert(!new_comp.getOwner().isValid());
    new_comp.setOwner(THandle(this));
    assert(!comps[comp_type]);
    comps[comp_type] = new_comp;
    return new_comp;
  }

  // -------------------------------------------------
  // Returns our current comp of that type
  template< class TObj >
  THandle get() const {
    unsigned comp_type = getType<TObj>();
    return comps[comp_type];
  }
  // Same using a const char*
  THandle get(const char* comp_name) const {
    unsigned comp_type = getTypeByName(comp_name);
    return comps[comp_type];
  }

  // Returns our current comp of that type
  template< class TObj >
  THandle has() const {
    unsigned comp_type = getType<TObj>();
    return comps[comp_type];
  }

  // Forward it to the del(THandle) method
  template< class TObj >
  bool del() {
    unsigned comp_type = getType<TObj>();
    return del(comps[comp_type]);
  }

  bool del(THandle comp) {
    unsigned comp_type = comp.getType();
    THandle h = comps[comp_type];
    comps[comp_type] = THandle();          // detach it from us
    return h.destroy();
  }

  void sendMsg(TMsgID msg_id);

};


#endif
