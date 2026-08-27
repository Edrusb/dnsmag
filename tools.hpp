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

#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <string>
#include <map>

using namespace std;

    /// make printf-like formating to a std::string

    /// \param[in] format the format string
    /// \param[in] ... list of argument to use against the format string
    /// \return the resulting string
    /// \note the supported masks for the format are:
    /// - \%s \%c \%d \%o \%\%  (usual behavior)
    /// - \%x display an integer under hexadecimal notation
    /// - \%i (matches infinint *)
    /// - \%S (matches std::string *)
    /// .
extern string tools_printf(const char *format, ...);

template <class A, class B> typename map<A, B>::const_iterator tools_find_by_val(
    const map<A, B> & carte,
    const B & val)
{
    typename map<A, B>::const_iterator it = carte.begin();

    while(it != carte.end() && it->second != val)
	++it;

    return it;
};



    /// for a given IP address in doted notation returns a the notation without the last dot and following byte

    /// \example: tools_truncate_subnet("172.16.1.2") should return "172.16.1"
extern string tools_truncate_subnet(const string & subnet);


    /// compute a serial number based on current date & time
extern string tools_compute_serial();

    /// returns the DNS reverse zone expected ARPA format of subnet
extern string tools_arpa_format(const string & subnet);




#endif
