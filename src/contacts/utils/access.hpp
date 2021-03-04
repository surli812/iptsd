#pragma once

#include "types.hpp"

#include <common/access.hpp>


namespace iptsd::common::access {

template<class V, class I, class T, class F>
[[gnu::always_inline]]
inline constexpr auto access(T const& data, F ravel, I shape, I i) -> V const&
{
    ensure(i, shape);

    return data[ravel(shape, i)];
}

template<class V, class I, class T, class F>
[[gnu::always_inline]]
inline constexpr auto access(T& data, F ravel, I shape, I i) -> V&
{
    ensure(i, shape);

    return data[ravel(shape, i)];
}

} /* namespace iptsd::common::access */
