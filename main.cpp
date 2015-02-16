#include <cstdio>
#include "handle/handle.h"

struct TMsg {
  TMsgID msg_id;
  union {
  };
};

template< class TObj, void (TObj::*method)( const TMsg&  ) >
struct TMsgCB {
  static void onMsg(void* obj_addr, const TMsg& msg) {
    TObj* obj = (TObj*)obj_addr;
    (obj->*method)(msg);
  }
};

struct TCallbackRegistry {
  uint32_t handle_type;
  void(*callback)(const TMsg& msg);
};

class MMsgsRegistry : public std::map < TMsgID, TCallbackRegistry > {
public:
  template<class TObj>
  void regMsg(TMsgID msg_id, void (TObj::*method)( const TMsg& msg)) {
    //(*this)[msg_id] = &TMsgCB<TObj, void(TObj::*method)(const TMsg&)>::onMsg;
    (*this)[msg_id] = &TMsgCB::onMsg;
  }
};

MMsgsRegistry msgs_register;

void TEntity::sendMsg(TMsgID msg_id) {
  


}



// A msg arrives to the entity, get msg_type, get list of handles affected, if handle type is affected
// and entity has that msg, call the component with that msg 


// -----------------------------------------
struct TCompName {
  char name[32];
  TCompName() {
    memset(name, 0x00, sizeof(name));
  }
  TCompName(const char* aname) {
    memset(name, 0x00, sizeof(name));
    strcpy_s(name, aname);
  }
};

// -----------------------------------------
struct TCompLife {
  int life;
  TCompLife() : life(100) {}
  TCompLife(int alife) : life(alife) {}
  ~TCompLife() {
    dbg("Life %p has been dtor when life is %d\n", this, life);
  }
  void onHello(const TMsg& msg);
};

// -----------------------------------------
DECLARE_HANDLE_MANAGER(TCompName, "name");
DECLARE_HANDLE_MANAGER(TCompLife, "life");
DECLARE_HANDLE_MANAGER(TEntity, "entity");

// -----------------------------------------
void createManagers() {
  getObjsManager<TEntity>()->allocateObjs(8);
  getObjsManager<TCompLife>()->allocateObjs(4);
  getObjsManager<TCompName>()->allocateObjs(4);
}

void initMsgs() {
  msgs_register.regMsg<TCompLife>(MSG_HELLO, &TCompLife::onHello);
}

// ------------------------------------------------------
void test() {
  auto on = getObjsManager<TCompLife>();

  dbg("getType(TEntity) = %d %s\n", getType<TEntity>(), getTypeName<TEntity>());
  dbg("getType(TCompName) = %d %s\n", getType<TCompName>(), getTypeName<TCompName>());
  dbg("getType(TCompLife) = %d %s\n", getType<TCompLife>(), getTypeName<TCompLife>());

  on->dumpInternals();

  dbg("allocating\n");
  THandle h1 = THandle::create<TCompLife>(101);

  on->dumpInternals();

  TCompLife *life1 = h1;
  if (h1) {
    dbg("h1 is valid\n");
    THandle h2(life1);
    if (h1 == h2) {
      dbg("h1(%08x) == h2(%08x)\n", h1.asUnsigned(), h2.asUnsigned());
    }
  }
  else {
    dbg("h1 is not valid\n");
  }
  dbg("allocating\n");
  THandle h1b = THandle::create<TCompLife>(1000);
  if (h1b)
    dbg("h1b is valid %08x\n", h1b.asUnsigned());
  TCompLife *life2 = h1b;
  life2->life = 202;
  on->dumpInternals();

  dbg("destroying obj0\n");
  h1b.destroy();
  on->dumpInternals();
  assert(!h1b.isValid());
  h1.destroy();
  assert(!h1.isValid());
  on->dumpInternals();

  dbg("Creating 4 objs\n");
  THandle h[4];
  for (int i = 0; i < 4; ++i) {
    dbg("Creating %d/%d obj\n", i, 4);
    h[i] = THandle::create<TCompLife>(1000 + i);
    on->dumpInternals();
  }
  assert(on->size() == on->capacity());
  dbg("Deleting one obj (#0)\n");
  h[3].destroy();
  on->dumpInternals();
  h[3].destroy();
  on->dumpInternals();
  h[0].destroy();
  h[0].destroy();
  on->dumpInternals();
  h[1].destroy();
  h[2].destroy();
}

void testEntities() {
  auto on = getObjsManager<TCompLife>();

  // Test entities
  TEntity* e1 = THandle::create<TEntity>("john");
  TCompLife* life = e1->add<TCompLife>(100);
  dbg("e1 is %s and has %d life points\n", e1->getName(), life->life);
  on->dumpInternals();
  TCompLife* life3 = e1->get<TCompLife>();
  assert(THandle(life3).getOwner() == THandle(e1));
  assert(life == life3 && life3 != nullptr);
  assert(e1->has<TCompLife>());
  bool deleted = e1->del< TCompLife>();
  assert(deleted);
  assert(!e1->has<TCompLife>());
  assert(!e1->get<TCompLife>().isValid());
  assert((TCompLife*)e1->get<TCompLife>() == nullptr);

  TCompLife* life4 = e1->addHandle(THandle::create<TCompLife>(23));
  TCompLife* life5 = e1->get("life");
  assert(life4 == life5);
  THandle(life4).destroy();
}


void testClone() {
  auto on = getObjsManager<TCompLife>();

  THandle h1 = THandle::create<TCompLife>(123);
  THandle h2 = h1.clone();
  TCompLife *c1 = h1;
  TCompLife *c2 = h2;
  dbg("c1.life is %d, c2.life is %d\n", c1->life, c2->life);

  on->dumpInternals();
}

void testMsgs() {

}


int main(int argc, char**argv) {
  createManagers();
  test();
  testEntities();
  testClone();
  testMsgs();
  return 0;
}