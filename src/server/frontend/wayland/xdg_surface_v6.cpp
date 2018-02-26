/*
 * Copyright © 2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#include "xdg_surface_v6.h"

#include "xdg_positioner_v6.h"
#include "xdg_toplevel_v6.h"

#include "wayland_utils.h"
#include "basic_surface_event_sink.h"
#include "wl_seat.h"
#include "wl_surface.h"

#include "mir/scene/surface_creation_parameters.h"
#include "mir/frontend/surface_id.h"
#include "mir/frontend/shell.h"

namespace mf = mir::frontend;
namespace geom = mir::geometry;

namespace mir
{
namespace frontend
{

class XdgSurfaceV6EventSink : public BasicSurfaceEventSink
{
public:
    using BasicSurfaceEventSink::BasicSurfaceEventSink;

    XdgSurfaceV6EventSink(WlSeat* seat, wl_client* client, wl_resource* target, wl_resource* event_sink,
        std::shared_ptr<bool> const& destroyed) :
        BasicSurfaceEventSink(seat, client, target, event_sink),
        destroyed{destroyed}
    {
        auto const serial = wl_display_next_serial(wl_client_get_display(client));
        post_configure(serial);
    }

    void send_resize(geometry::Size const& new_size) const override
    {
        if (window_size != new_size)
        {
            auto const serial = wl_display_next_serial(wl_client_get_display(client));
            notify_resize(new_size);
            post_configure(serial);
        }
    }

    std::function<void(geometry::Size const& new_size)> notify_resize = [](auto){};

private:
    void post_configure(int serial) const
    {
        seat->spawn(run_unless(destroyed, [event_sink= event_sink, serial]()
            {
                wl_resource_post_event(event_sink, 0, serial);
            }));
    }

    std::shared_ptr<bool> const destroyed;
};

struct XdgPopupV6 : wayland::XdgPopupV6
{
    XdgPopupV6(struct wl_client* client, struct wl_resource* parent, uint32_t id) :
        wayland::XdgPopupV6(client, parent, id)
    {
    }

    void grab(struct wl_resource* seat, uint32_t serial) override
    {
        (void)seat, (void)serial;
        // TODO
    }

    void destroy() override
    {
        wl_resource_destroy(resource);
    }
};

}
}

mf::XdgSurfaceV6* mf::XdgSurfaceV6::get_xdgsurface(wl_resource* surface) const
{
    auto* tmp = wl_resource_get_user_data(surface);
    return static_cast<XdgSurfaceV6*>(static_cast<wayland::XdgSurfaceV6*>(tmp));
}

mf::XdgSurfaceV6::XdgSurfaceV6(wl_client* client, wl_resource* parent, uint32_t id, wl_resource* surface,
                               std::shared_ptr<mf::Shell> const& shell, WlSeat& seat)
    : wayland::XdgSurfaceV6(client, parent, id),
      WlAbstractMirWindow{client, surface, resource, shell},
      parent{parent},
      shell{shell},
      sink{std::make_shared<XdgSurfaceV6EventSink>(&seat, client, surface, resource, destroyed)}
{
    WlAbstractMirWindow::sink = sink;
}

mf::XdgSurfaceV6::~XdgSurfaceV6()
{
    auto* const mir_surface = WlSurface::from(surface);
    mir_surface->set_role(null_wl_mir_window_ptr);
}

void mf::XdgSurfaceV6::destroy()
{
    wl_resource_destroy(resource);
}

void mf::XdgSurfaceV6::get_toplevel(uint32_t id)
{
    new XdgToplevelV6{client, parent, id, shell, this};
    auto* const mir_surface = WlSurface::from(surface);
    mir_surface->set_role(this);
}

void mf::XdgSurfaceV6::get_popup(uint32_t id, struct wl_resource* parent, struct wl_resource* positioner)
{
    auto* tmp = wl_resource_get_user_data(positioner);
    auto const* const pos =  static_cast<XdgPositionerV6*>(static_cast<wayland::XdgPositionerV6*>(tmp));

    auto const session = get_session(client);
    auto& parent_surface = *get_xdgsurface(parent);

    params->type = mir_window_type_freestyle;
    params->parent_id = parent_surface.surface_id;
    if (pos->size.is_set()) params->size = pos->size.value();
    params->aux_rect = pos->aux_rect;
    params->surface_placement_gravity = pos->surface_placement_gravity;
    params->aux_rect_placement_gravity = pos->aux_rect_placement_gravity;
    params->aux_rect_placement_offset_x = pos->aux_rect_placement_offset_x;
    params->aux_rect_placement_offset_y = pos->aux_rect_placement_offset_y;
    params->placement_hints = mir_placement_hints_slide_any;

    new XdgPopupV6{client, parent, id};
    auto* const mir_surface = WlSurface::from(surface);
    mir_surface->set_role(this);
}

void mf::XdgSurfaceV6::set_window_geometry(int32_t x, int32_t y, int32_t width, int32_t height)
{
    geom::Rectangle const input_region{{x, y}, {width, height}};

    if (surface_id.as_value())
    {
        spec().input_shape = {input_region};
    }
    else
    {
        params->input_shape = {input_region};
    }
}

void mf::XdgSurfaceV6::ack_configure(uint32_t serial)
{
    (void)serial;
    // TODO
}


void mf::XdgSurfaceV6::set_title(std::string const& title)
{
    if (surface_id.as_value())
    {
        spec().name = title;
    }
    else
    {
        params->name = title;
    }
}

void mf::XdgSurfaceV6::move(struct wl_resource* /*seat*/, uint32_t /*serial*/)
{
    if (surface_id.as_value())
    {
        if (auto session = get_session(client))
        {
            shell->request_operation(session, surface_id, sink->latest_timestamp(), Shell::UserRequest::move);
        }
    }
}

void mf::XdgSurfaceV6::resize(struct wl_resource* /*seat*/, uint32_t /*serial*/, uint32_t edges)
{
    if (surface_id.as_value())
    {
        if (auto session = get_session(client))
        {
            MirResizeEdge edge = mir_resize_edge_none;

            switch (edges)
            {
            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP:
                edge = mir_resize_edge_north;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM:
                edge = mir_resize_edge_south;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_LEFT:
                edge = mir_resize_edge_west;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP_LEFT:
                edge = mir_resize_edge_northwest;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM_LEFT:
                edge = mir_resize_edge_southwest;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_RIGHT:
                edge = mir_resize_edge_east;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP_RIGHT:
                edge = mir_resize_edge_northeast;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM_RIGHT:
                edge = mir_resize_edge_southeast;
                break;

            default:;
            }

            shell->request_operation(
                session,
                surface_id,
                sink->latest_timestamp(),
                Shell::UserRequest::resize,
                edge);
        }
    }
}

void mf::XdgSurfaceV6::set_notify_resize(std::function<void(geometry::Size const& new_size)> notify_resize)
{
    sink->notify_resize = notify_resize;
}

void mf::XdgSurfaceV6::set_parent(optional_value<SurfaceId> parent_id)
{
    if (surface_id.as_value())
    {
        spec().parent_id = parent_id;
    }
    else
    {
        params->parent_id = parent_id;
    }
}

void mf::XdgSurfaceV6::set_max_size(int32_t width, int32_t height)
{
    if (surface_id.as_value())
    {
        if (width == 0) width = std::numeric_limits<int>::max();
        if (height == 0) height = std::numeric_limits<int>::max();

        auto& mods = spec();
        mods.max_width = geom::Width{width};
        mods.max_height = geom::Height{height};
    }
    else
    {
        if (width == 0)
        {
            if (params->max_width.is_set())
                params->max_width.consume();
        }
        else
            params->max_width = geom::Width{width};

        if (height == 0)
        {
            if (params->max_height.is_set())
                params->max_height.consume();
        }
        else
            params->max_height = geom::Height{height};
    }
}

void mf::XdgSurfaceV6::set_min_size(int32_t width, int32_t height)
{
    if (surface_id.as_value())
    {
        auto& mods = spec();
        mods.min_width = geom::Width{width};
        mods.min_height = geom::Height{height};
    }
    else
    {
        params->min_width = geom::Width{width};
        params->min_height = geom::Height{height};
    }
}

void mf::XdgSurfaceV6::set_maximized()
{
    if (surface_id.as_value())
    {
        spec().state = mir_window_state_maximized;
    }
    else
    {
        params->state = mir_window_state_maximized;
    }
}

void mf::XdgSurfaceV6::unset_maximized()
{
    if (surface_id.as_value())
    {
        spec().state = mir_window_state_restored;
    }
    else
    {
        params->state = mir_window_state_restored;
    }
}

