// sass.hpp must go before all system headers to get the
// __EXTENSIONS__ fix on Solaris.
#include "sass.hpp"

#include <iostream>
#include <typeinfo>
#include <string>

#include "listize.hpp"
#include "context.hpp"
#include "backtrace.hpp"
#include "error_handling.hpp"

namespace Sass {

  Listize::Listize()
  {  }

  Expression* Listize::operator()(Selector_List* sel)
  {
    List_Obj l = SASS_MEMORY_NEW(List, sel->pstate(), sel->length(), SASS_COMMA);
    l->from_selector(true);
    for (size_t i = 0, L = sel->length(); i < L; ++i) {
      if (!sel->at(i)) continue;
      l->append(sel->at(i)->perform(this));
    }
    if (l->length()) return l.detach();
    return SASS_MEMORY_NEW(Null, l->pstate());
  }

  Expression* Listize::operator()(Compound_Selector* sel)
  {
    std::string str;
    for (size_t i = 0, L = sel->length(); i < L; ++i) {
      Expression* e = (*sel)[i]->perform(this);
      if (e) str += e->to_string();
    }
    return SASS_MEMORY_NEW(String_Quoted, sel->pstate(), str);
  }

  Expression* Listize::operator()(Complex_Selector* sel)
  {
    List_Obj l = SASS_MEMORY_NEW(List, sel->pstate(), 2);
    l->from_selector(true);
    Compound_Selector_Obj head = sel->head();
    if (head && !head->is_empty_reference())
    {
      Expression* hh = head->perform(this);
      if (hh) l->append(hh);
    }

    std::string reference = ! sel->reference() ? ""
      : sel->reference()->to_string();
    switch(sel->combinator())
    {
      case Complex_Selector::PARENT_OF:
        l->append(SASS_MEMORY_NEW(String_Quoted, sel->pstate(), ">"));
      break;
      case Complex_Selector::ADJACENT_TO:
        l->append(SASS_MEMORY_NEW(String_Quoted, sel->pstate(), "+"));
      break;
      case Complex_Selector::REFERENCE:
        l->append(SASS_MEMORY_NEW(String_Quoted, sel->pstate(), "/" + reference + "/"));
      break;
      case Complex_Selector::PRECEDES:
        l->append(SASS_MEMORY_NEW(String_Quoted, sel->pstate(), "~"));
      break;
      case Complex_Selector::ANCESTOR_OF:
      break;
      default: break;
    }

    Complex_Selector_Obj tail = sel->tail();
    if (tail)
    {
      Expression_Obj tt = tail->perform(this);
      if (List* ls = Cast<List>(tt))
      { l->concat(ls); }
    }
    if (l->length() == 0) return 0;
    return l.detach();
  }

}
