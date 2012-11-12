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
 * Authored by: Robert Carr <robert.carr@canonical.com>
 */
#ifndef __EVENT_FACTORY_H
#define __EVENT_FACTORY_H

namespace mir
{
namespace input
{
namespace synthesis
{
enum class EventType
{
    Key
};
enum class KeyEventAction
{
    Down, Up
};
class EventParameters
{
public:
    virtual ~EventParameters();
    virtual EventType get_event_type() = 0;
protected:    
    EventParameters() = default;
    
};
class KeyParameters : public EventParameters
{
public:    
    KeyParameters();

    KeyParameters& from_device(int device_id);
    KeyParameters& of_scancode(int scancode);
    KeyParameters& with_action(KeyEventAction action);

    EventType get_event_type();
    
    int device_id;
    int scancode;
    KeyEventAction action;
};
}   
}
}

#endif /* __EVENT_FACTORY_H */
