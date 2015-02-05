#include <cstdio>
#include "handle/handle.h"
#include "handle/entity.h"

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
DECLARE_HANDLE_MANAGER(TCompName, "name");

// -----------------------------------------
struct TCompLife {
  int life;
  TCompLife() : life(100) {}
  TCompLife(int alife) : life(alife) {}
  ~TCompLife() {
    dbg("Life %p has been dtor when life is %d\n", this, life);
  }
};
DECLARE_HANDLE_MANAGER(TCompLife, "life");

// -----------------------------------------
void createManagers() {
  getObjsManager<TEntity>()->allocateObjs(8);
  getObjsManager<TCompLife>()->allocateObjs(4);
  getObjsManager<TCompName>()->allocateObjs(4);
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

  //dbg("allocating 3\n");
  //THandle h1c = THandle::create<TCompLife>(1001);
  //on->dumpInternals();
  // clonar
}


int main(int argc, char**argv) {
  createManagers();
  test();
  return 0;
}