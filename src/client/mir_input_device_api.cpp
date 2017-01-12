/*
 * Copyright © 2016 Canonical Ltd.
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
 */

#include "mir_toolkit/mir_input_device.h"
#include "mir/input/input_devices.h"
#include "mir/require.h"
#include "handle_event_exception.h"

namespace mi = mir::input;
namespace mp = mir::protobuf;

namespace
{
inline auto as_device_infos(MirInputConfig const* config)
{
    mir::require(config);
    return reinterpret_cast<mp::InputDevices const*>(config);
}

inline auto as_device_infos(MirInputConfig* config)
{
    mir::require(config);
    return reinterpret_cast<mp::InputDevices*>(config);
}

inline auto as_device(mp::InputDeviceInfo* data)
{
    mir::require(data);
    return reinterpret_cast<MirInputDevice*>(data);
}

inline auto as_device(mp::InputDeviceInfo const* data)
{
    mir::require(data);
    return reinterpret_cast<MirInputDevice const*>(data);
}

inline auto as_device_info(MirInputDevice const* device)
{
    mir::require(device);
    return reinterpret_cast<mp::InputDeviceInfo const*>(device);
}

inline auto as_device_info(MirInputDevice* device)
{
    mir::require(device);
    return reinterpret_cast<mp::InputDeviceInfo*>(device);
}

inline auto as_pointer_config(MirPointerConfig const* config)
{
    mir::require(config);
    return reinterpret_cast<mp::PointerConfig const*>(config);
}

inline auto as_pointer_config(MirPointerConfig* config)
{
    mir::require(config);
    return reinterpret_cast<mp::PointerConfig*>(config);
}

inline auto as_touchpad_config(MirTouchpadConfig const* config)
{
    mir::require(config);
    return reinterpret_cast<mp::TouchpadConfig const*>(config);
}

inline auto as_touchpad_config(MirTouchpadConfig* config)
{
    mir::require(config);
    return reinterpret_cast<mp::TouchpadConfig*>(config);
}
}

size_t mir_input_config_device_count(MirInputConfig const* input_config) MIR_HANDLE_EVENT_EXCEPTION(
{
    mir::require(input_config);
    auto device_infos = as_device_infos(input_config);

    return device_infos->device_info_size();
})

MirInputDevice* mir_input_config_get_mutable_device(MirInputConfig* input_config, size_t index) MIR_HANDLE_EVENT_EXCEPTION(
{
    mir::require(input_config);
    auto device_infos = as_device_infos(input_config);

    mir::require(int(index) < device_infos->device_info_size());
    return as_device(device_infos->mutable_device_info(index));
})

MirInputDevice* mir_input_config_get_mutable_device_by_id(MirInputConfig* input_config, MirInputDeviceId id) MIR_HANDLE_EVENT_EXCEPTION(
{
    mir::require(input_config);

    auto device_infos = as_device_infos(input_config);
    for (auto& dev : *device_infos->mutable_device_info())
        if (dev.id() == id)
            return as_device(&dev);
    return nullptr;
})

MirInputDevice const* mir_input_config_get_device(MirInputConfig const* input_config, size_t index) MIR_HANDLE_EVENT_EXCEPTION(
{
    mir::require(input_config);
    auto device_infos = as_device_infos(input_config);

    mir::require(int(index) < device_infos->device_info_size());
    return as_device(&device_infos->device_info(index));
})

MirInputDevice const* mir_input_config_get_device_by_id(MirInputConfig const* input_config, MirInputDeviceId id) MIR_HANDLE_EVENT_EXCEPTION(
{
    mir::require(input_config);

    auto device_infos = as_device_infos(input_config);
    for (auto const& dev : device_infos->device_info())
        if (dev.id() == id)
            return as_device(&dev);
    return nullptr;
})

uint32_t mir_input_device_get_capabilities(MirInputDevice const* device) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_device_info(device)->capabilities();
})

MirInputDeviceId mir_input_device_get_id(MirInputDevice const* device) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_device_info(device)->id();
})

char const* mir_input_device_get_name(MirInputDevice const* device) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_device_info(device)->name().c_str();
})

char const* mir_input_device_get_unique_id(MirInputDevice const* device) MIR_HANDLE_EVENT_EXCEPTION(
{
    auto device_info = as_device_info(device);
    return device_info->unique_id().c_str();
})

MirPointerConfig const* mir_input_device_get_pointer_configuration(MirInputDevice const* device) MIR_HANDLE_EVENT_EXCEPTION(
{
    auto device_info = as_device_info(device);
    if (device_info->has_pointer_configuration())
        return reinterpret_cast<MirPointerConfig const*>(&device_info->pointer_configuration());

    return nullptr;
})

MirPointerConfig const* mir_input_device_get_pointer_config(MirInputDevice const* device) MIR_HANDLE_EVENT_EXCEPTION(
{
    auto device_info = as_device_info(device);
    if (device_info->has_pointer_configuration())
        return reinterpret_cast<MirPointerConfig const*>(&device_info->pointer_configuration());

    return nullptr;
})

MirPointerAcceleration mir_pointer_config_get_acceleration(MirPointerConfig const* conf) MIR_HANDLE_EVENT_EXCEPTION(
{
    return static_cast<MirPointerAcceleration>(as_pointer_config(conf)->acceleration());
})

double mir_pointer_config_get_acceleration_bias(MirPointerConfig const* conf) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_pointer_config(conf)->acceleration_bias();
})

double mir_pointer_config_get_horizontal_scroll_scale(MirPointerConfig const* conf) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_pointer_config(conf)->horizontal_scroll_scale();
})

double mir_pointer_config_get_vertical_scroll_scale(MirPointerConfig const* conf) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_pointer_config(conf)->vertical_scroll_scale();
})

MirPointerHandedness mir_pointer_config_get_handedness(MirPointerConfig const* conf) MIR_HANDLE_EVENT_EXCEPTION(
{
    if (as_pointer_config(conf)->handedness() == mir_pointer_handedness_left)
        return mir_pointer_handedness_left;
    else
        return mir_pointer_handedness_right;
})

MirPointerConfig* mir_input_device_get_mutable_pointer_config(MirInputDevice* device) MIR_HANDLE_EVENT_EXCEPTION(
{
    auto device_info = as_device_info(device);

    if (device_info->has_pointer_configuration())
        return reinterpret_cast<MirPointerConfig*>(device_info->mutable_pointer_configuration());
    return nullptr;
})

void mir_pointer_config_set_acceleration(MirPointerConfig* conf, MirPointerAcceleration acceleration) MIR_HANDLE_EVENT_EXCEPTION(
{
    as_pointer_config(conf)->set_acceleration(acceleration);
})

void mir_pointer_config_set_acceleration_bias(MirPointerConfig* conf, double acceleration_bias) MIR_HANDLE_EVENT_EXCEPTION(
{
    as_pointer_config(conf)->set_acceleration_bias(acceleration_bias);
})

void mir_pointer_config_set_horizontal_scroll_scale(MirPointerConfig* conf,
                                                           double horizontal_scroll_scale) MIR_HANDLE_EVENT_EXCEPTION(
{
    as_pointer_config(conf)->set_horizontal_scroll_scale(horizontal_scroll_scale);
})

void mir_pointer_config_set_vertical_scroll_scale(MirPointerConfig* conf, double vertical_scroll_scale) MIR_HANDLE_EVENT_EXCEPTION(
{
    as_pointer_config(conf)->set_vertical_scroll_scale(vertical_scroll_scale);
})

void mir_pointer_config_set_handedness(MirPointerConfig* conf, MirPointerHandedness handedness) MIR_HANDLE_EVENT_EXCEPTION(
{
    as_pointer_config(conf)->set_handedness(handedness);
})

MirTouchpadConfig const* mir_input_device_get_touchpad_config(MirInputDevice const* device) MIR_HANDLE_EVENT_EXCEPTION(
{
    auto device_info = as_device_info(device);

    if (device_info->has_touchpad_configuration())
        return reinterpret_cast<MirTouchpadConfig const*>(&device_info->touchpad_configuration());
    return nullptr;
})

MirTouchpadClickModes mir_touchpad_config_get_click_modes(MirTouchpadConfig const* conf) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_touchpad_config(conf)->click_modes();
})

MirTouchpadScrollModes mir_touchpad_config_get_scroll_modes(MirTouchpadConfig const* conf) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_touchpad_config(conf)->scroll_modes();
})

int mir_touchpad_config_get_button_down_scroll_button(MirTouchpadConfig const* conf) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_touchpad_config(conf)->button_down_scroll_button();
})

bool mir_touchpad_config_get_tap_to_click(MirTouchpadConfig const* conf) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_touchpad_config(conf)->tap_to_click();
})

bool mir_touchpad_config_get_middle_mouse_button_emulation(MirTouchpadConfig const* conf) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_touchpad_config(conf)->middle_mouse_button_emulation();
})

bool mir_touchpad_config_get_disable_with_mouse(MirTouchpadConfig const* conf) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_touchpad_config(conf)->disable_with_mouse();
})

bool mir_touchpad_config_get_disable_while_typing(MirTouchpadConfig const* conf) MIR_HANDLE_EVENT_EXCEPTION(
{
    return as_touchpad_config(conf)->disable_while_typing();
})

MirTouchpadConfig* mir_input_device_get_mutable_touchpad_config(MirInputDevice* device) MIR_HANDLE_EVENT_EXCEPTION(
{
    auto device_info = as_device_info(device);

    if (device_info->has_touchpad_configuration())
        return reinterpret_cast<MirTouchpadConfig*>(device_info->mutable_touchpad_configuration());
    return nullptr;
})

void mir_touchpad_config_set_click_modes(MirTouchpadConfig* conf, MirTouchpadClickModes modes) MIR_HANDLE_EVENT_EXCEPTION(
{
    as_touchpad_config(conf)->set_click_modes(modes);
})

void mir_touchpad_config_set_scroll_modes(MirTouchpadConfig* conf, MirTouchpadScrollModes modes) MIR_HANDLE_EVENT_EXCEPTION(
{
    as_touchpad_config(conf)->set_scroll_modes(modes);
})

void mir_touchpad_config_set_button_down_scroll_button(MirTouchpadConfig* conf, int button) MIR_HANDLE_EVENT_EXCEPTION(
{
    as_touchpad_config(conf)->set_button_down_scroll_button(button);
})

void mir_touchpad_config_set_tap_to_click(MirTouchpadConfig* conf, bool tap_to_click) MIR_HANDLE_EVENT_EXCEPTION(
{
    as_touchpad_config(conf)->set_tap_to_click(tap_to_click);
})

void mir_touchpad_config_set_middle_mouse_button_emulation(MirTouchpadConfig* conf, bool middle_emulation) MIR_HANDLE_EVENT_EXCEPTION(
{
    auto touchpad_info = as_touchpad_config(conf);
    touchpad_info->set_middle_mouse_button_emulation(middle_emulation);
})

void mir_touchpad_config_set_disable_with_mouse(MirTouchpadConfig* conf, bool active) MIR_HANDLE_EVENT_EXCEPTION(
{
    auto touchpad_info = as_touchpad_config(conf);
    touchpad_info->set_disable_with_mouse(active);
})

void mir_touchpad_config_set_disable_while_typing(MirTouchpadConfig* conf, bool active) MIR_HANDLE_EVENT_EXCEPTION(
{
    auto touchpad_info = as_touchpad_config(conf);
    touchpad_info->set_disable_while_typing(active);
})
