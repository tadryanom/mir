/*
 * Copyright © 2013-2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#ifndef MIR_COMPOSITOR_RECENTLY_BOUND_CACHE_H_
#define MIR_COMPOSITOR_RECENTLY_BOUND_CACHE_H_

#include "mir/graphics/texture_cache.h"
#include "mir/graphics/texture.h"
#include "mir/graphics/buffer_id.h"
#include <unordered_map>

namespace mir
{
namespace compositor 
{
class RecentlyBoundCache : public graphics::TextureCache
{
public:
    std::shared_ptr<graphics::Texture> load_texture(graphics::Renderable const& renderable) override;
    void invalidate() override;
    void release_live_texture_resources() override;

private:
    struct CountedTexture
    {
        CountedTexture()
         : texture(std::make_shared<graphics::Texture>())
        {}
        std::shared_ptr<graphics::Texture> texture;
        graphics::BufferID last_bound_buffer;
        bool used{true};
        std::shared_ptr<graphics::Buffer> resource;
    };

    std::unordered_map<graphics::Renderable::ID, CountedTexture> textures;
};
}
}

#endif /* MIR_COMPOSITOR_RECENTLY_BOUND_CACHE_H_ */
