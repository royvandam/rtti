# TODO

Proposed improvements to the limitations documented in the
[README](README.md#casting-limitations). Keep C++17 support and operation
without compiler-provided RTTI when evaluating these changes.

## Suggested order

Start with a null-safe pointer helper and a decision on copy/move support.
Handle full source-aware casting as a separate change with correctness and
performance measurements. The remaining items involve API or compatibility
choices and can be evaluated independently.

## Null-safe pointer casts

- [ ] Add a free function such as `RTTI::cast<T>(ptr)`.

Return `nullptr` for a null source pointer. Preserve the source's static type
and constness so identity casts and unambiguous upcasts can use a direct
conversion, including when the complete object contains repeated bases.
Delegate other cases to runtime traversal.

Calling a member through a null pointer must remain invalid; the free function
provides a safe entry point. Cover const targets, virtual and non-virtual
diamonds, missing targets, and both type-ID modes.

## Copy and move support

- [ ] Decide whether RTTI-enabled classes should support copying and moving
  by default or through an opt-in API.

`RTTI::Enable` currently deletes move assignment, suppressing implicit copying
and moving throughout derived hierarchies. Protected, defaulted constructors
and assignment operators can allow derived classes to choose their own value
semantics while preventing assignment through the public `Enable` interface.

This changes an intentional policy. Update the existing restriction tests and
verify type IDs, adjusted casts, and object state after copy/move construction
and assignment. Derived classes must retain any restrictions imposed by their
own members and bases.

## Source-aware casting through repeated bases

- [ ] Preserve the source subobject's type and address during runtime casts.

The current member API dispatches through the shared `RTTI::Enable` base and
resolves targets from the most-derived registered type. It loses which repeated
base subobject the caller started from. A pointer helper can recover simple
identity casts and upcasts; full source-aware downcasts and cross-casts require
a traversal redesign.

Use the source subobject to identify a unique valid target. Reject genuinely
ambiguous targets rather than returning the first matching parent. Account for
virtual bases reached through multiple paths without treating the same
subobject as multiple targets. Evaluate changes to the member API and virtual
cast interface, including compatibility with existing consumers.

Preserve `is<T>()` as an inheritance query: a successful check does not imply
that a conversion is unambiguous. Consider a separate castability query if
needed. Compare supported cases with
[native dynamic-cast rules](https://eel.is/c++draft/expr.dynamic.cast), and
measure runtime cost and generated code size before changing traversal.

## Declaration macro access levels

- [ ] Consider an explicit ending access level or an additional macro variant.

Both declaration macros currently finish in `private:`. Making the ending
access level configurable can reduce surprises. Preserve existing behavior
unless a compatibility change is intentional: changing the default to public
could expose members in existing classes. The macros cannot automatically
detect and restore the access level active before invocation.

## Missing RTTI declarations in subclasses

- [ ] Add diagnostics where the concrete type is known, and investigate ways
  to reduce declaration boilerplate.

A subclass that inherits its parent's RTTI overrides identifies as that parent,
including for `typeId()` and `cast<void>()`. Under the current C++17 design with
native RTTI disabled, each participating subclass must supply its identity.
Automatic discovery would require a different registration or construction
design.

Where a concrete type is available to a template or factory, check that its
`TypeInfo` describes that type rather than an inherited parent. Such checks
cannot detect every unannotated subclass created outside those entry points.

## Hash collision handling

- [ ] Evaluate wider hashes and richer identity information for checked casts.

The default 32-bit ID alone cannot guarantee uniqueness. Wider hashes reduce
collision risk but do not eliminate it, and changing hash width affects the
public `TypeId` representation and stored IDs. Pointer IDs already avoid hash
collisions within a process, subject to shared-library symbol visibility.

Collision detection or disambiguation needs additional identity information.
The typed APIs could carry a richer key, but `isById(id)` cannot distinguish
types sharing the same bare numeric ID. Decide whether collisions should be
rejected or resolved, and add deliberately colliding test cases. Any added
identity check must distinguish types rather than relying on another hash.

## Stable application-defined identities

- [ ] Consider an opt-in API for explicit type names or IDs.

Compiler-generated names can vary with the compiler and its configuration;
pointer IDs are valid only within a running process. Neither representation
provides a portable persistent identity by itself.

Application-defined names or IDs could support persistence and communication
between components built with different toolchains. Define how explicit
identities interact with the existing modes, and document that applications
own uniqueness, versioning, and compatibility. Explicit names still need a
collision policy if they are reduced to hashes.

## Feasibility checks completed

Temporary prototypes tested a null-safe pointer helper with identity and
unambiguous upcasts, and protected defaulted special members enabling derived
copy/move operations. Both worked in C++17 with compiler-provided RTTI disabled
and in both type-ID modes. Copy/move checks also verified object state, type
IDs, and adjusted casts.

These prototypes were isolated from the repository. The implementation tasks
above remain open and need regression coverage appropriate to each change.
