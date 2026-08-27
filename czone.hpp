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

#ifndef CZONE_HPP
#define CZONE_HPP

#include <map>
#include <set>
#include <deque>
#include "jsoner.hpp"

using namespace std;

    /// class czone manages IP-name association within a C class IP subnet (/24)

class czone: public jsoner
{
public:
	/// constructor

	/// \param[in] zone_name is FQDN of the zone like sub.hpc.lab
	/// \param[in] subnet_prefix is expected of the form "192.168.22"
    czone(const string & fqdn_zone_name,
	  const string & subnet_prefix);

    czone(const czone &) = default;
    czone(czone &&) = default;
    czone & operator = (const czone &) = default;
    czone & operator = (czone &&) = default;
    virtual ~czone() = default;

	/// clear all name-IP association in the zone
    void clear();

	/// returns the assigned IP to the given name
    string add_record(const string & name);

	/// add and record a new IP-name pair, but throw an exception if IP is already used
    void add_record(const string & name, unsigned int last_byte);

	/// delete a record from this zone
    void del_record(const string & name);

	/// generate records for a forward zone
    string generate_forward_records() const;

	/// generate records for a reverse zone
    string generate_reverse_records() const;

	/// returns the subnet prefix associated to that zone
    string get_subnet_prefix() const { return subnet; };

	/// retunrs the list of records found in this zone
    deque<string> get_listing() const;

	// inherited from jsoner
    virtual void load_json(const json & source) override;

	// inherited from jsoner
    virtual json save_json() const override;

private:
    string zone;
    string subnet;
    map<string, unsigned int> asso; /// assiciate name to the last byte of its IP @
    set<unsigned int> free_slots;   /// contains the list of last byte of free IP @

    static constexpr const char* LABEL_ZONE = "zone";     // contains a value
    static constexpr const char* LABEL_SUBNET = "subnet"; // contains a value
    static constexpr const char* LABEL_ASSO = "pairs";    // contains an array of objects:
    static constexpr const char* LABEL_NAME = "name";     //    contains a value
    static constexpr const char* LABEL_IP = "ip";         //    contains a value

};

#endif
