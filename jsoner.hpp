/*********************************************************************/
// dnsmag - a bind zone config generator
// Copyright (C) 2026 Denis Corbin
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//  You should have received a copy of the GNU General Public License
//  along with dnsmag.  If not, see <http://www.gnu.org/licenses/>
//
/*********************************************************************/

#ifndef JSONER_HPP
#define JSONER_HPP

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class jsoner
{
public:
    jsoner() = default;
    jsoner(const jsoner & ref) = default;
    jsoner(jsoner && ref) noexcept(false) = default;
    jsoner & operator = (const jsoner & ref) = default;
    jsoner & operator = (jsoner && ref) noexcept(false) = default;
    virtual ~jsoner() = default;

        /// setup the components from the json provided information

        /// \param[in] source json formated configuration to use for configuration
    virtual void load_json(const json & source) = 0;

        /// produce a json structure from the component configuration

        /// \return the current component configuration as a json object
    virtual json save_json() const = 0;
};

#endif
