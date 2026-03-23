// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/ui/render/Render.hpp"

namespace kf::ui::internal {

struct UiTraitsTag {};

template<typename R, typename P, typename E> struct UiTraits final : UiTraitsTag {
    KF_CHECK_IMPL(R, ::kf::ui::render::RenderTag);
    using RenderImpl = R;
    using PageImpl = P;
    using EventImpl = E;
};

}// namespace ui::internal