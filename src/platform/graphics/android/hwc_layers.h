/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#ifndef MIR_GRAPHICS_ANDROID_HWC_LAYERS_H_
#define MIR_GRAPHICS_ANDROID_HWC_LAYERS_H_

#include "mir/graphics/renderable.h"
#include "mir/graphics/android/fence.h"
#include "mir/geometry/rectangle.h"
#include <hardware/hwcomposer.h>
#include <memory>
#include <vector>
#include <initializer_list>
#include <list>

namespace mir
{
namespace graphics
{

class Renderable;
class NativeBuffer;

namespace android
{

struct HWCLayer
{
    virtual ~HWCLayer() = default;

    HWCLayer(hwc_layer_1 *native_layer);
    HWCLayer& operator=(HWCLayer const& layer);

    NativeFence release_fence() const;
    bool needs_gl_render() const;

protected:
    HWCLayer(
        hwc_layer_1 *native_layer,
        int type,
        buffer_handle_t handle,
        geometry::Rectangle position,
        geometry::Size buffer_size,
        bool skip, bool alpha
    );


    HWCLayer(HWCLayer const& layer) = delete;
    hwc_rect_t visible_rect;
    hwc_layer_1 * hwc_layer;
};

//layer could be GLES rendered, or overlaid by hwc.
struct CompositionLayer : public HWCLayer
{
    CompositionLayer(Renderable const& renderable);
private:
    hwc_layer_1 native_layer;
};

//used as the target (lowest layer, fb)
struct FramebufferLayer : public HWCLayer
{
    FramebufferLayer();
    FramebufferLayer(NativeBuffer const&);
private:
    hwc_layer_1 native_layer;
};

//used during compositions when we want to restrict to GLES render only
struct ForceGLLayer : public HWCLayer
{
    ForceGLLayer();
    ForceGLLayer(HWCLayer const& fb_layer);
    ForceGLLayer(NativeBuffer const&);
private:
    hwc_layer_1 native_layer;
};

}
}
}

#endif /* MIR_GRAPHICS_ANDROID_HWC_LAYERS_H_ */
