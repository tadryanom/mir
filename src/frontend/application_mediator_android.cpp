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
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#include "mir/frontend/application_mediator.h"
#include "mir/frontend/application_listener.h"
#include "mir/frontend/session.h"
#include "mir/exception.h"

void mir::frontend::ApplicationMediator::drm_auth_magic(
    google::protobuf::RpcController* /*controller*/,
    const mir::protobuf::DRMMagic* /*request*/,
    mir::protobuf::Void* /*response*/,
    google::protobuf::Closure* /*done*/)
{
    if (application_session.get() == nullptr)
        BOOST_THROW_EXCEPTION(std::runtime_error("Invalid application session"));

    listener->application_drm_auth_magic_called(application_session->get_name());

    BOOST_THROW_EXCEPTION(std::runtime_error("drm_auth_magic request not supported by the Android platform"));
}
