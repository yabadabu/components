#include "handle.h"

TMsgsSubscriptions msg_subscriptions;

void TEntity::send(const TMsg& msg) {

  // Para todos los callbacks registrados para ese msg_type
  TMsgsSubscriptions::const_iterator i = msg_subscriptions.find(msg.id);
  while (i != msg_subscriptions.end() && i->first == msg.id) {

    // Si esta entity tiene el componente
    if (comps[i->second.comp_type]) {

      // Obtener el component a clase base
      THandle comp = comps[i->second.comp_type];

      // El method->execute es virtual, hace el upcast a la clase 
      // Tcomponent derivada y pasa el msg a la clase
      i->second.method->execute(comp, msg);
    }
    ++i;
  }

}