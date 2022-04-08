#include <iostream>

#include "myobject.h"
#include "object.h"

int main(void) {
  MyObject object("Hello World!");
  MyObject object2("Don't transform this object!");

  object.saySomethingElse(object.sayMessage());
object.saySomethingElse(object.sayMessage());

  object.oneVariable+1+object.anotherVariable;
object.oneVariable;

  object2.sayMessage();
  return 0;
}
