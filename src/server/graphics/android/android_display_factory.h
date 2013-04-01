/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#ifndef MIR_GRAPHICS_ANDROID_ANDROID_DISPLAY_FACTORY_H_
#define MIR_GRAPHICS_ANDROID_ANDROID_DISPLAY_FACTORY_H_

#include "display_factory.h"
#include <hardware/hwcomposer.h>

namespace mir
{
namespace graphics
{
namespace android
{

class HWCFactory;
class HWCDevice;
class DisplayAllocator;
class AndroidDisplayFactory : public DisplayFactory
{
public:
    AndroidDisplayFactory(std::shared_ptr<DisplayAllocator> const& fb_factory, std::shared_ptr<HWCFactory> const& hwc_factory);

    std::shared_ptr<Display> create_fb() const;

private:
    void setup_hwc_dev(const hw_module_t* module);

    std::shared_ptr<DisplayAllocator> fb_factory;
    std::shared_ptr<HWCFactory> hwc_factory;
    std::shared_ptr<hwc_composer_device_1> hwc_dev;
};

}
}
}

#endif /* MIR_GRAPHICS_ANDROID_ANDROID_DISPLAY_FACTORY_H_ */
