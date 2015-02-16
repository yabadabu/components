#ifndef INC_MSGS_H_
#define INC_MSGS_H_

enum TMsgID {
  MSG_INVALID = 0
, MSG_HELLO
, MSG_DESTROYED
, MSG_COUNT
};

struct TMsg {
  TMsgID id;
  union {

    struct {
      float   points;
    } life;

    struct {
      unsigned h_killer;
      float    distance;
      bool     was_super;
    } kill;
  };

  TMsg() : id(MSG_INVALID) { }
  TMsg(TMsgID aid) : id(aid) { }

};

// -------------------------------------------------------
struct IFunctorBase {
  virtual ~IFunctorBase() { }
  virtual void execute(THandle handle, const TMsg &msg) = 0;
};

template< class TObj >
struct TFunctor : public IFunctorBase {

  // Puntero a un member fn de la clase TObj que devuelve void
  // y recibe como argumento un (const TMsg *)
  typedef void (TObj::*TMemberFn)(const TMsg &msg);

  // Una variable para guardar la direccion del metodo que queremos
  // llamar
  TMemberFn member;

  // El ctor guarda el member al que habra que llamar
  TFunctor(TMemberFn amember) : member(amember) { }

  // La implementacion del virtual, convierte el objeto al tipo 
  // del template y llama al metodo que se guardó pasando el msg
  // como argumento
  void execute(THandle h, const TMsg &msg) {
    TObj* obj_of_my_type = h;
    assert(obj_of_my_type);
    if (obj_of_my_type)
      (obj_of_my_type->*member)(msg);
  }
};

// The information stored for each msg registered
struct TComponentMsgHandler {
  uint32_t      comp_type;      // comp_id registered
  IFunctorBase* method;         // method of the class to be called
};

// A multimap to store who should be called when a msg type arrives
typedef std::multimap< TMsgID, TComponentMsgHandler > TMsgsSubscriptions;
extern TMsgsSubscriptions msg_subscriptions;

template< class TComp >
void subscribeTo(TMsgID msg_id, IFunctorBase* method) {
  std::pair<TMsgID, TComponentMsgHandler> e;
  e.first = msg_id;
  e.second.comp_type = getType<TComp>();
  e.second.method = method;
  msg_subscriptions.insert(e);
}

// Macro to simplify msg subscription
#define SUBSCRIBE(acomp,amsg,amethod) \
  subscribeTo<acomp>( amsg, new TFunctor< acomp>( &acomp::amethod ) );


#endif
