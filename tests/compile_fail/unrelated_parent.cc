#include <rtti.hh>

struct Unrelated {};

struct Child {
    RTTI_DECLARE_TRIVIAL(Child, Unrelated);
};

static_assert(!Child::TypeInfo::Name().empty());
