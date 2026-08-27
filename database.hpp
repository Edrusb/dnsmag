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

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "erreurs.hpp"
#include "jsoner.hpp"
#include "czone.hpp"


using namespace std;

    /// class database manages all user zones

class database: public jsoner
{
public:

	/// constructor from parameters
    database(const string & parent_zone_name,
	     const string & named_conf_file_path,
	     const string & zone_dir_path,
	     const string & ns_name,
	     const string & hostmaster_name,
	     const unsigned int refresh_time = 86400,
	     const unsigned int retry_time = 7200,
	     const unsigned int expire_time = 3500000,
	     const unsigned int nttl_time = 600
	);

	/// constructor from json data
    database(const json & data) { load_json(data); };

    database(const database &) = default;
    database(database &&) = default;
    database & operator = (const database &) = default;
    database & operator = (database &&) = default;
    virtual ~database() = default;

	/// \param[in] name is the name of the sub-domain to create
	/// \param[in] subnet this is an IPv4 address of the /24 subnet to use for that zone
    void create_zone(const string & name,
		     const string & subnet);

	/// deletes the zone provided in argument
    void delete_zone(const string & name);

	/// \returns the assigned IP to the name in given zone
	/// \note throw exception if no free IP could be found
	///  or name already assigned in that zone
    string add_record(const string & zone,
		      const string & name);

	/// create a new association beteween name and IP with last byte is provided

	/// \note if the name or the IP is already used, an exception is thrown
    void add_record(const string & zone,
		    const string & name,
		    unsigned int last_byte);

	/// deletes the provided record name from the designated zone
    void del_record(const string & zone,
		    const string & name);

	/// provide the list existing zones
    deque<string> list_zones() const;

	/// return the subnet used by the zone
    string show_zone_subnet(const string & zone) const;

	/// provide the list of recorded names in that zone
    deque<string> show_zone_listing(const string & zone) const;

	// inherited from jsoner
    virtual void load_json(const json & source) override;

	// inherited from jsoner
    virtual json save_json() const override;

private:
    string parent_zone;
    string named_conf;
    string zone_dir;
    string ns;
    string hostmaster;
    unsigned int refresh;
    unsigned int retry;
    unsigned int expire;
    unsigned int nttl;
    map<string, czone> zones; // associate a sub-zone name to a czone object

    void generate_named_conf() const;
    void generate_forward_file(const string & zone) const;
    void generate_reverse_file(const string & zone) const;

    string get_forward_zone_filename(const string & zone) const;
    string get_reverse_zone_filename(const string & zone) const;

    static constexpr const unsigned int db_version = 1;

    static constexpr const char* LABEL_VERSION = "db_version";
    static constexpr const char* LABEL_PARENT_ZONE = "parent-zone";
    static constexpr const char* LABEL_NAMED_CONF = "named.conf";
    static constexpr const char* LABEL_ZONE_DIR = "zones-path";
    static constexpr const char* LABEL_NS =  "nameserver";
    static constexpr const char* LABEL_HOSTMASTER = "hostmaster";
    static constexpr const char* LABEL_REFRESH = "refresh";
    static constexpr const char* LABEL_RETRY = "retry";
    static constexpr const char* LABEL_EXPIRE = "expire";
    static constexpr const char* LABEL_NTTL = "nttl";
    static constexpr const char* LABEL_ZONES = "zones";
    static constexpr const char* LABEL_ZONE_NAME = "zone-name";
    static constexpr const char* LABEL_ZONE_CONFIG = "zone-config";

    static void write_to_file(const string & filename, const string & data);
};

#endif
