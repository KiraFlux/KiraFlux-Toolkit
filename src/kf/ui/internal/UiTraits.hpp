// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/ui/render/Render.hpp"

namespace kf::ui::internal {

struct UiTraitsTag {};

template<typename R, typename E> struct UiTraits final : UiTraitsTag {
    KF_CHECK_IMPL(R, ::kf::ui::render::RenderTag);
    using RenderImpl = R;

    using EventImpl = E;
};

}// namespace kf::ui::internal