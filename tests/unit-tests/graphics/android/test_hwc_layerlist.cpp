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

#include "mir_test_doubles/stub_renderable.h"
#include "mir_test_doubles/stub_buffer.h"
#include "src/platforms/android/server/hwc_layerlist.h"
#include "mir_test_doubles/mock_android_native_buffer.h"
#include "hwc_struct_helpers.h"
#include <gtest/gtest.h>

namespace mg=mir::graphics;
namespace mga=mir::graphics::android;
namespace mt=mir::test;
namespace mtd=mir::test::doubles;
namespace geom=mir::geometry;

namespace
{
struct LayerListTest : public testing::Test
{
    LayerListTest() :
        layer_adapter{std::make_shared<mga::IntegerSourceCrop>()},
        buffer1{std::make_shared<mtd::StubBuffer>()},
        buffer2{std::make_shared<mtd::StubBuffer>()},
        renderables{std::make_shared<mtd::StubRenderable>(buffer1),
                    std::make_shared<mtd::StubRenderable>(buffer2),
                    std::make_shared<mtd::StubRenderable>()}
    {

        mt::fill_hwc_layer(fbtarget, &visible_rect, disp_frame, *stub_fb, HWC_FRAMEBUFFER_TARGET, 0);
        mt::fill_hwc_layer(skip, &visible_rect, disp_frame, *stub_fb, HWC_FRAMEBUFFER, HWC_SKIP_LAYER);
    }

    std::shared_ptr<mga::LayerAdapter> layer_adapter;
    std::shared_ptr<mtd::StubBuffer> buffer1;
    std::shared_ptr<mtd::StubBuffer> buffer2;
    std::list<std::shared_ptr<mg::Renderable>> renderables;

    geom::Rectangle const disp_frame{{0,0}, {44,22}};
    std::shared_ptr<mtd::StubBuffer> stub_fb{
        std::make_shared<mtd::StubBuffer>(
            std::make_shared<testing::NiceMock<mtd::MockAndroidNativeBuffer>>(disp_frame.size), disp_frame.size)};
    hwc_layer_1_t fbtarget;
    hwc_layer_1_t skip;
    hwc_rect_t visible_rect;
};
}

TEST_F(LayerListTest, list_defaults)
{
    mga::LayerList layerlist{layer_adapter, {}};

    auto list = layerlist.native_list();
    EXPECT_EQ(-1, list->retireFenceFd);
    EXPECT_EQ(HWC_GEOMETRY_CHANGED, list->flags);
    EXPECT_NE(nullptr, list->dpy);
    EXPECT_NE(nullptr, list->sur);
    EXPECT_EQ(std::distance(layerlist.begin(), layerlist.end()), 2);
}

TEST_F(LayerListTest, list_iterators)
{
    size_t additional_layers = 2;
    mga::LayerList list(layer_adapter, {});
    EXPECT_EQ(std::distance(list.begin(), list.end()), additional_layers);

    additional_layers = 1;
    mga::LayerList list2(layer_adapter, renderables);
    EXPECT_EQ(std::distance(list2.begin(), list2.end()), additional_layers + renderables.size());

    mga::LayerList list3(std::make_shared<mga::Hwc10Adapter>(), renderables);
    EXPECT_EQ(std::distance(list3.begin(), list3.end()), renderables.size());
}

TEST_F(LayerListTest, keeps_track_of_needs_commit)
{
    size_t fb_target_size{1};
    mga::LayerList list(layer_adapter, renderables);

    auto i = 0;
    for (auto& layer : list)
    {
        if (i == 3)
            EXPECT_FALSE(layer.needs_commit);
        else
            EXPECT_TRUE(layer.needs_commit);
        i++;
    }

    mg::RenderableList list2{
        std::make_shared<mtd::StubRenderable>(buffer1),
        std::make_shared<mtd::StubRenderable>(buffer2),
        std::make_shared<mtd::StubRenderable>()
    };
    list.update_list(list2);

    i = 0;
    for (auto& layer : list)
    {
        if (i == 3)
            EXPECT_FALSE(layer.needs_commit);
        else
            EXPECT_TRUE(layer.needs_commit);
        i++;
    }

    ASSERT_THAT(list.native_list()->numHwLayers, testing::Eq(list2.size() + fb_target_size));
    list.native_list()->hwLayers[2].compositionType = HWC_OVERLAY;
    list.update_list(list2);

    i = 0;
    for (auto& layer : list)
    {
        if ((i == 2) || (i == 3))
            EXPECT_FALSE(layer.needs_commit);
        else
            EXPECT_TRUE(layer.needs_commit);
        i++;
    }
}

TEST_F(LayerListTest, setup_fb_hwc10)
{
    using namespace testing;
    mga::LayerList list(std::make_shared<mga::Hwc10Adapter>(), {});
    list.setup_fb(stub_fb);

    auto l = list.native_list();
    ASSERT_THAT(l->numHwLayers, Eq(1));
    EXPECT_THAT(l->hwLayers[l->numHwLayers-1], MatchesLegacyLayer(skip));
}

TEST_F(LayerListTest, setup_fb_without_skip)
{
    using namespace testing;
    mga::LayerList list(layer_adapter, renderables);
    list.setup_fb(stub_fb);

    auto l = list.native_list();
    ASSERT_THAT(l->numHwLayers, Eq(1 + renderables.size()));
    EXPECT_THAT(l->hwLayers[l->numHwLayers-1], MatchesLegacyLayer(fbtarget));
}

TEST_F(LayerListTest, setup_fb_with_skip)
{
    using namespace testing;
    mga::LayerList list(layer_adapter, {});
    list.setup_fb(stub_fb);
    auto l = list.native_list();
    ASSERT_THAT(l->numHwLayers, Eq(2));
    EXPECT_THAT(l->hwLayers[l->numHwLayers-2], MatchesLegacyLayer(skip));
    EXPECT_THAT(l->hwLayers[l->numHwLayers-1], MatchesLegacyLayer(fbtarget));
}

TEST_F(LayerListTest, generate_rejected_renderables)
{
    using namespace testing;
    mga::LayerList list(layer_adapter, renderables);

    auto l = list.native_list();
    ASSERT_THAT(l->numHwLayers, Eq(4));
    l->hwLayers[1].compositionType = HWC_OVERLAY;

    EXPECT_THAT(list.rejected_renderables(), ElementsAre(renderables.front(), renderables.back())); 
}

TEST_F(LayerListTest, swap_needed)
{
    using namespace testing;
    mga::LayerList list(layer_adapter, renderables);

    auto l = list.native_list();
    ASSERT_THAT(l->numHwLayers, Eq(4));
    for (auto i = 0u; i < 3; i++)
        l->hwLayers[i].compositionType = HWC_OVERLAY;
    l->hwLayers[3].compositionType = HWC_FRAMEBUFFER_TARGET;

    EXPECT_FALSE(list.needs_swap());

    l->hwLayers[1].compositionType = HWC_FRAMEBUFFER;

    EXPECT_TRUE(list.needs_swap());

    list.update_list({});
    EXPECT_TRUE(list.needs_swap());
}
