/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#include "mir/frontend/application_manager.h"
#include "mir/compositor/buffer_bundle.h"
#include "mir/surfaces/surface_controller.h"
#include "mir/surfaces/surface_stack.h"
#include "mir/surfaces/surface.h"
#include "mir/compositor/buffer_swapper.h"
#include "mir/frontend/application_focus_selection_strategy.h"
#include "mir/frontend/application_focus_mechanism.h"
#include "mir/frontend/registration_order_focus_selection_strategy.h"
#include "mir/frontend/application_session_model.h"


#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "mir_test/gmock_fixes.h"
#include "mir_test/empty_deleter.h"
#include "mir_test/mock_application_surface_organiser.h"

namespace mc = mir::compositor;
namespace mf = mir::frontend;
namespace ms = mir::surfaces;

namespace
{

struct MockFocusMechanism: public mf::ApplicationFocusMechanism
{
  MOCK_METHOD2(focus, void(std::shared_ptr<mf::ApplicationSessionContainer>, std::shared_ptr<mf::ApplicationSession>));
};

}

TEST(TestApplicationManagerAndFocusSelectionStrategy, cycle_focus)
{
    using namespace ::testing;
    ms::MockApplicationSurfaceOrganiser organiser;
    std::shared_ptr<mf::ApplicationSessionModel> model(new mf::ApplicationSessionModel());
    mf::RegistrationOrderFocusSelectionStrategy strategy;
    MockFocusMechanism mechanism;
    std::shared_ptr<mf::ApplicationSession> new_session;

    mf::ApplicationManager app_manager(std::shared_ptr<ms::ApplicationSurfaceOrganiser>(&organiser, mir::EmptyDeleter()), 
                                       model,
                                       std::shared_ptr<mf::ApplicationFocusSelectionStrategy>(&strategy, mir::EmptyDeleter()),
                                       std::shared_ptr<mf::ApplicationFocusMechanism>(&mechanism, mir::EmptyDeleter()));
    
    EXPECT_CALL(mechanism, focus(_,_)).Times(3);

    auto session1 = app_manager.open_session("Visual Basic Studio");
    auto session2 = app_manager.open_session("Microsoft Access");
    auto session3 = app_manager.open_session("WordPerfect");
    
    {
      InSequence seq;
      EXPECT_CALL(mechanism, focus(_,session1)).Times(1);
      EXPECT_CALL(mechanism, focus(_,session2)).Times(1);
      EXPECT_CALL(mechanism, focus(_,session3)).Times(1);
    }
    
    app_manager.focus_next();
    app_manager.focus_next();
    app_manager.focus_next();
}

TEST(TestApplicationManagerAndFocusSelectionStrategy, closing_applications_transfers_focus)
{
    using namespace ::testing;
    ms::MockApplicationSurfaceOrganiser organiser;
    std::shared_ptr<mf::ApplicationSessionModel> model(new mf::ApplicationSessionModel());
    mf::RegistrationOrderFocusSelectionStrategy strategy;
    MockFocusMechanism mechanism;
    std::shared_ptr<mf::ApplicationSession> new_session;

    mf::ApplicationManager app_manager(std::shared_ptr<ms::ApplicationSurfaceOrganiser>(&organiser, mir::EmptyDeleter()), 
                                       model,
                                       std::shared_ptr<mf::ApplicationFocusSelectionStrategy>(&strategy, mir::EmptyDeleter()),
                                       std::shared_ptr<mf::ApplicationFocusMechanism>(&mechanism, mir::EmptyDeleter()));
    
    EXPECT_CALL(mechanism, focus(_,_)).Times(3);

    auto session1 = app_manager.open_session("Visual Basic Studio");
    auto session2 = app_manager.open_session("Microsoft Access");
    auto session3 = app_manager.open_session("WordPerfect");
    
    {
      InSequence seq;
      EXPECT_CALL(mechanism, focus(_,session2)).Times(1);
      EXPECT_CALL(mechanism, focus(_,session1)).Times(1);
    }
    
    app_manager.close_session(session3);
    app_manager.close_session(session2);
}
