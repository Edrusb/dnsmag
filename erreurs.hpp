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

#ifndef ERREURS_HPP
#define ERREURS_HPP

#include "tools.hpp"

class erreur
{
public:
    erreur(const string & msg) { err = msg; };

    void prepend_msg(const string & msg) { err = msg + err; };
    string get_message() const { return err; };

private:
    string err;
};

#define BUG throw erreur(tools_printf("BUG in file %s at line %d", __FILE__, __LINE__))

#endif
