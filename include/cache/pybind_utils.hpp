#pragma once

#include <pybind11/pybind11.h>
#include <experimental/optional>

namespace pybind11 { namespace detail {
    template <typename T>
    struct type_caster<std::experimental::optional<T>> {
        using value_type = std::experimental::optional<T>;
        using T_caster = make_caster<T>;

        static handle cast(const value_type &src, return_value_policy policy, handle parent) {
            if (!src) return none().release();
            return T_caster::cast(*src, policy, parent);
        }

        bool load(handle src, bool convert) {
            if (src.is_none()) {
                value = std::experimental::nullopt;
                return true;
            }
            T_caster loader;
            if (loader.load(src, convert)) {
                value = loader.operator T&();
                return true;
            }
            return false;
        }

        PYBIND11_TYPE_CASTER(value_type, _("Optional[") + T_caster::name + _("]"));
    };
}}
