#include <rtti.hh>

struct UnregisteredParent {
    virtual ~UnregisteredParent() = default;
};

struct Child
    : UnregisteredParent
    , virtual RTTI::Enable {
    RTTI_DECLARE_TYPEINFO(Child, UnregisteredParent);
};

static_assert(!Child::TypeInfo::Name().empty());
