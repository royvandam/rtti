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

#include <algorithm>
#include <array>
#include <cstdint>
#include <type_traits>

#include "hash.hh"

namespace RTTI {
    template <typename T>
    constexpr std::string_view TypeName();
    
    template <>
    constexpr std::string_view TypeName<void>()
    { return "void"; }

    namespace Detail { 
        template <typename T>
        constexpr std::string_view WrappedTypeName()  {
        #ifdef __clang__
            return __PRETTY_FUNCTION__;
        #elif defined(__GNUC__)
            return __PRETTY_FUNCTION__;
        #else
            #error "Unsupported compiler"
        #endif
        }

        constexpr std::size_t WrappedTypeNamePrefixLength() { 
            return WrappedTypeName<void>().find(TypeName<void>()); 
        }
        
        constexpr std::size_t WrappedTypeNameSuffixLength() { 
            return WrappedTypeName<void>().length() 
                - WrappedTypeNamePrefixLength() 
                - TypeName<void>().length();
        }
    }

    template <typename T>
    constexpr std::string_view TypeName() {
        constexpr auto wrappedTypeName = Detail::WrappedTypeName<T>();
        constexpr auto prefixLength = Detail::WrappedTypeNamePrefixLength();
        constexpr auto suffixLength = Detail::WrappedTypeNameSuffixLength();
        constexpr auto typeNameLength = wrappedTypeName.length() - prefixLength - suffixLength;
        return wrappedTypeName.substr(prefixLength, typeNameLength);
    }

    /// TypeId type definition
    using TypeId = std::uint32_t;

    /// Forward declaration of the Enable base.
    struct Enable;

    /**
     * Static typeinfo structure for registering types and accessing their information.
     */
    template <typename This, typename... Parents>
    struct TypeInfo {
        using T = std::remove_const_t<std::remove_reference_t<This>>;

        /// Ensure all passed parents are basses of this type.
        static_assert((... && std::is_base_of<Parents, This>::value),
                      "One or more parents are not a base of this type.");

        /// Ensure all passed parent hierarchies have RTTI enabled.
        static_assert((... && std::is_base_of<Enable, Parents>::value),
                      "One or more parent hierarchies is not based on top of RTTI::Enable.");

        /**
         * Returns the type string of the type T.
         * @returns Type string
         */
        [[nodiscard]] static constexpr std::string_view Name() noexcept {
            return TypeName<T>();
        }

        /**
         * Returns the type identifier of the type T.
         * @returns Type identifier
         */
        [[nodiscard]] static constexpr TypeId Id() noexcept {
            return Hash::FNV1a(Name());
        }

        /**
         * Checks whether the passed type is the same or a parent of the type.
         * @tparam The type to compare the identifier with.
         * @returns True in case a match was found.
         */
        [[nodiscard]] static constexpr bool Is(TypeId typeId) noexcept {
            return (Id() == typeId) || (... || (Parents::TypeInfo::Is(typeId)));
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
        [[nodiscard]] static void const* DynamicCast(TypeId typeId, T const* ptr) noexcept {
            // Check whether the current type matches the requested type.
            if (Id() == typeId) {
                // Cast the passed pointer in to the current type and stop
                // the recursion.
                return static_cast<This const*>(ptr);
            }

            // The current type does not match, recursively invoke the method
            // for all directly related parent types.
            std::array<void const*, sizeof...(Parents)> ptrs = {
                Parents::TypeInfo::DynamicCast(typeId, ptr)...};

            // Check whether the traversal up the dependency hierarchy returned a pointer
            // that is not null.
            auto it = std::find_if(ptrs.begin(), ptrs.end(), [](void const* ptr) {
                return ptr != nullptr;
            });
            return (it != ptrs.end()) ? *it : nullptr;
        }
    };

    /**
     * Parent type for types at the base of an open RTTI hierarchy
     */
    struct Enable {
        virtual ~Enable() = default;

        /**
         * Returns the type identifier of the object.
         * @returns Type identifier
         */
        [[nodiscard]] virtual TypeId typeId() const noexcept = 0;

        /**
         * Checks whether the object is a direct or derived instance of
         * the type identified by the passed identifier.
         * @tparam The identifier to compare with.
         * @returns True in case a match was found.
         */
        [[nodiscard]] virtual bool isById(TypeId typeId) const noexcept = 0;

        /**
         * Checks whether the object is an instance of child instance of
         * the passed type.
         * @tparam The type to compare the identifier with.
         * @returns True in case a match was found.
         */
        template <typename T>
        [[nodiscard]] bool is() const noexcept {
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
        [[nodiscard]] T* cast() noexcept {
            return reinterpret_cast<T*>(const_cast<void*>(_cast(TypeInfo<T>::Id())));
        }

        template <typename T>
        [[nodiscard]] T const* cast() const noexcept {
            return reinterpret_cast<T const*>(_cast(TypeInfo<T>::Id()));
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
        [[nodiscard]] virtual void const* _cast(TypeId typeId) const noexcept = 0;
    };
}  // namespace RTTI

/**
 * Macro to be called in the body of each type declaration that is to be part of an
 * open hierarchy RTTI structure. The type itself or one or more parents of the type
 * need to have been derived from RTTI::Enable.
 * @param T The type it self.
 * @param Parents Variadic number of direct parrent types of the type
 */
#define RTTI_DECLARE_TYPEINFO(T, ...)                                                      \
public:                                                                                    \
    using TypeInfo = RTTI::TypeInfo<T, ##__VA_ARGS__>;                                     \
    [[nodiscard]] virtual RTTI::TypeId typeId() const noexcept override {                  \
        return TypeInfo::Id();                                                             \
    }                                                                                      \
    [[nodiscard]] virtual bool isById(RTTI::TypeId typeId) const noexcept override {       \
        return TypeInfo::Is(typeId);                                                       \
    }                                                                                      \
                                                                                           \
protected:                                                                                 \
    [[nodiscard]] virtual void const* _cast(RTTI::TypeId typeId) const noexcept override { \
        return TypeInfo::Is(typeId) ? TypeInfo::DynamicCast(typeId, this) : nullptr;       \
    }
