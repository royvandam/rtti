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

    /**
     * Abstract base interface for types part of an open RTTI hierarchy.
     */
    struct Type {
        virtual ~Type() {};

        /**
         * Returns the type identifier of the object.
         * @returns Type identifier
         */
        virtual TypeId typeId() const noexcept =0;

        /**
         * Checks whether the object is a direct or derived instance of
         * the type identified by the passed identifier.
         * @tparam The identifier to compare with.
         * @returns True in case a match was found.
         */
        virtual bool isById(TypeId typeId) const noexcept =0;

        /**
         * Checks whether the object is a direct or derived instance of
         * the type identified by the passed identifier.
         * @tparam The type to compare the identifier with.
         * @returns True in case a match was found.
         */
        template <typename T>
        bool is() const noexcept {
            return isById(TypeInfo<T>::Id());
        }

        /**
         * Dynamically cast the object to the passed type. Attempts to find the
         * type by its identifier in the dependency hierarchy of the object.
         * When found casts the top level `this` pointer to an instance the
         * passed type using a static_cast. The pointer offset for types
         * dependent on multiple inheritance is hence resolved at compile-time.
         * In essence we are always up-casting from the most specialized type.
         *
         * @tparam T Pointer type to case the object into.
         * @returns A valid pointer to an instance of the passed type. Nullptr
         * incase the object instance is not a direct descendence of the passed
         * type.
         */
        template <typename T>
        T* cast() noexcept {
            return static_cast<T*>(_cast(TypeInfo<T>::Id()));
        }

        template <typename T>
        T const* cast() const noexcept {
            return static_cast<T const*>(_cast(TypeInfo<T>::Id()));
        }

    protected:

        /**
         * Used to invoke the _dynamic_cast from the most specialized type in the
         * dependency hierarchy by overloaded this function in each derivation of
         * RTTI::Extends<>.
         * @param typeId The identifier of the type to cast the object into.
         * @returns Valid pointer to instance of requested type if the object is a
         * direct descendance of the type identified by the passed type id. Otherwise
         * the value returned is a nullptr.
         */
        virtual void *_cast(TypeId typeId) noexcept =0;
        virtual void const*_cast(TypeId typeId) const noexcept =0;
    };

    /**
     * Parent type for types at the base of an open RTTI hierarchy
     */
    template<typename This>
    struct Base : virtual Type {
        virtual ~Base() {}

        virtual TypeId typeId() const noexcept {
            return TypeInfo<This>::Id();
        }

        virtual bool isById(TypeId typeId) const noexcept {
            return TypeInfo<This>::Id() == typeId;
        }

    protected:
        virtual void *_cast(TypeId typeId) noexcept override {
            return const_cast<void *>(_dynamic_cast(typeId, static_cast<This const*>(this)));
        }
        virtual void const* _cast(TypeId typeId) const noexcept override {
            return _dynamic_cast(typeId, static_cast<This const*>(this));
        }

        /**
         * Walks the dependency hierarchy of the object in search of the type identified
         * by the passed type id. In case found casts the passed pointer into the passed
         * type. If no match can be found the function returns a nullptr.
         * @tparam T The type of the most specialized type in the dependency hierarchy.
         * @param typeId The identifier of the type to cast the object into.
         * @returns Valid pointer to instance of requested type if the object is a
         * direct descendance of the type identified by the passed type id. Otherwise
         * the value returned is a nullptr.
         */
        template <typename T>
        static void const* _dynamic_cast(TypeId typeId, T const* ptr) noexcept {
            if (TypeInfo<This>::Id() == typeId) {
                return static_cast<This const*>(ptr);
            }
            return nullptr;
        }
    };

    /**
     * Parent type for types part of an open RTTI hierarchy that are not types at the base
     * of the hierarchy.
     */
    template <typename This, typename... Parents>
    struct Extends : public Parents... {
        virtual TypeId typeId() const noexcept override {
            return TypeInfo<This>::Id();
        }

        virtual bool isById(TypeId typeId) const noexcept override {
            return (TypeInfo<This>::Id() == typeId) || (... || Parents::isById(typeId));
        }

    protected:
        virtual void *_cast(TypeId typeId) noexcept override {
            return const_cast<void *>(_dynamic_cast(typeId, static_cast<This const*>(this)));
        }
        virtual void const*_cast(TypeId typeId) const noexcept override {
            return _dynamic_cast(typeId, static_cast<This const*>(this));
        }

        template <typename T>
        static void const*_dynamic_cast(TypeId typeId, T const* ptr) noexcept {
            // Check whether the current type matches the requested type.
            if (TypeInfo<This>::Id() == typeId) {
                // Cast the passed pointer in to the current type and stop
                // the recursion.
                return static_cast<This const*>(ptr);
            }

            // Current type does not match, recursively invoke the method
            // for all directly related parent types.
            std::array<void const*, sizeof...(Parents)> ptrs = {Parents::_dynamic_cast(typeId, ptr)...};

            // Check whether the traversal up the dependency hierarchy returned
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
