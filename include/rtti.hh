/**
 * MIT License
 *
 * Copyright (c) 2020 Roy van Dam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

namespace RTTI {
    using TypeId = uintptr_t;

    template <typename T>
    struct TypeInfo {
        /**
         * Returns the type identifier of the type T.
         * @returns Type identifier
         */
        static TypeId Id() {
            return reinterpret_cast<TypeId>(&Id);
        }
    };

    template <typename This>
    struct Base {
        virtual ~Base() {}

        /**
         * Returns the type identifier of the object.
         * @returns Type identifier
         */
        virtual TypeId typeId() const {
            return TypeInfo<This>::Id();
        }

        /**
         * Checks whether the object is a direct or derived instance of
         * the type identified by the passed identifier.
         * @returns True in case a match has been found.
         */
        virtual bool isA(TypeId typeId) const {
            return TypeInfo<This>::Id() == typeId;
        }

        /**
         * Dynamically cast the object to the passed type. Attempts to
         * find the type by its identifier in the dependency chain of the object.
         * When found casts the top level `this` pointer to an instance the
         * passed type using a static_cast. The pointer offset for types
         * dependent on multiple inheritance is hance resolved at compile-time.
         * In essence we are always up-casting from the most specialized type.
         *
         * @tparam T Pointer type to case the object into.
         * @returns A valid pointer to an instance of the passed type. Nullptr
         * incase the object instance is not a direct descendence of the passed
         * type.
         */
        template <typename T>
        T cast() {
            // Invoke the _dynamic_cast from the most specialized type in the
            // dependency chain by calling into the virtual _cast method that is
            // overloaded by each derivation of RTTI::Extends<>.
            return static_cast<T>(_cast(TypeInfo<typename std::remove_pointer<T>::type>::Id()));
        }

    protected:
        virtual void *_cast(TypeId typeId) {
            return _dynamic_cast(typeId, static_cast<This *>(this));
        }

        template <typename T>
        static void *_dynamic_cast(TypeId typeId, T *ptr) {
            if (TypeInfo<This>::Id() == typeId) {
                return static_cast<This *>(ptr);
            }
            return nullptr;
        }
    };

    template <typename This, typename... Parents>
    struct Extends : public Parents... {
        /**
         * Get the type identifier of the object.
         * @returns Type identifier
         */
        virtual TypeId typeId() const override {
            return TypeInfo<This>::Id();
        }

        /**
         * Checks whether the object is a direct or derived instance of
         * the type identified by the passed identifier.
         * @returns True in case a match has been found.
         */
        virtual bool isA(TypeId typeId) const override {
            return (TypeInfo<This>::Id() == typeId) || (... || Parents::isA(typeId));
        }

    protected:
        virtual void *_cast(TypeId typeId) override {
            return _dynamic_cast(typeId, static_cast<This *>(this));
        }

        template <typename T>
        void *_dynamic_cast(TypeId typeId, T *ptr) {
            // Check whether the current type matches the requested type.
            if (TypeInfo<This>::Id() == typeId) {
                // Cast the passed pointer in to the current type and stop
                // the recursion.
                return static_cast<This *>(ptr);
            }

            // Current type does not match, recursively invoke the method
            // for all directly related parent types.
            std::array<void *, sizeof...(Parents)> ptrs = {Parents::_dynamic_cast(typeId, ptr)...};

            // Check whether the traversal up the dependency chain returned
            // a pointer that is not null.
            for (auto ptr : ptrs) {
                if (ptr != nullptr) {
                    return ptr;
                }
            }

            // No match found.
            return nullptr;
        }
    };
}  // namespace RTTI
