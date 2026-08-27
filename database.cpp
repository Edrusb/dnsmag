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

extern "C"
{
#include <fcntl.h>
#include <unistd.h>
}

#include <iostream>
#include <fstream>
#include "database.hpp"
#include "erreurs.hpp"
#include "tools.hpp"

database::database(const string & parent_zone_name,
		   const string & named_conf_file_path,
		   const string & zone_dir_path,
		   const string & ns_name,
		   const string & hostmaster_name,
		   const unsigned int refresh_time,
		   const unsigned int retry_time,
		   const unsigned int expire_time,
		   const unsigned int nttl_time):
    parent_zone(parent_zone_name),
    named_conf(named_conf_file_path),
    zone_dir(zone_dir_path),
    ns(ns_name),
    hostmaster(hostmaster_name),
    refresh(refresh_time),
    retry(retry_time),
    expire(expire_time),
    nttl(nttl_time)
{
	// nothing to be added here
}

void database::create_zone(const string & name,
			   const string & subnet)
{
    string subnet_part = tools_truncate_subnet(subnet);
    string zone_fqdn = name + "." + parent_zone;

    for(map<string, czone>::iterator it = zones.begin();
	it != zones.end();
	++it)
    {
	if(it->first == name)
	    throw erreur(tools_printf("Zone %S already exist, cannot create it", &name));

	if(it->second.get_subnet_prefix() == subnet_part)
	    throw erreur(tools_printf("Subnet %S already used by zone %S", &subnet, &(it->first)));
    }

    zones.emplace(name, czone(zone_fqdn, subnet_part));
    generate_forward_file(name);
    generate_reverse_file(name);
    generate_named_conf();
}

void database::delete_zone(const string & name)
{
    map<string, czone>::iterator it = zones.find(name);
    string resp;

    if(it == zones.end())
	throw erreur(tools_printf("Unknown zone %S, cannot delete it", &name));

    cout << "You are about to delete the whole zone named " << name << ", please type y to confirm" << endl;
    cin >> resp;

    if(resp == "y")
    {
	if(unlink(get_forward_zone_filename(name).c_str()) != 0)
	    cout << tools_printf("Failed removing forward zone file %s: %s\n",
				 get_forward_zone_filename(name).c_str(),
				 strerror(errno));
	if(unlink(get_reverse_zone_filename(name).c_str()) != 0)
	    cout << tools_printf("Failed removing reverse zone file %s: %s\n",
				 get_forward_zone_filename(name).c_str(),
				 strerror(errno));
	zones.erase(it);
	generate_named_conf();
    }
    else
	cout << "OK, database not modified" << endl;
}

string database::add_record(const string & zone,
			    const string & name)
{
    map<string, czone>::iterator it = zones.find(zone);
    string ret;

    if(it == zones.end())
	throw erreur(tools_printf("Unknown zone %S", &zone));

    ret = it->second.add_record(name);
    generate_forward_file(zone);
    generate_reverse_file(zone);

    return ret;
}

void database::del_record(const string & zone,
			  const string & name)
{
    map<string, czone>::iterator it = zones.find(zone);

    if(it == zones.end())
	throw erreur(tools_printf("Unknown zone %S", &zone));

    it->second.del_record(name);
    generate_forward_file(zone);
    generate_reverse_file(zone);
}

deque<string> database::list_zones() const
{
    deque<string> ret;

    for(map<string, czone>::const_iterator it = zones.begin();
	it != zones.end();
	++it)
	ret.push_back(it->first);

    return ret;
}

string database::show_zone_subnet(const string & zone) const
{
    map<string, czone>::const_iterator it = zones.find(zone);

    if(it == zones.end())
	throw erreur(tools_printf("Unknown zone %S", &zone));

    return it->second.get_subnet_prefix() + ".0";
}

deque<string> database::show_zone_listing(const string & zone) const
{
    map<string, czone>::const_iterator it = zones.find(zone);

    if(it == zones.end())
	throw erreur(tools_printf("Unknown zone %S", &zone));

    return it->second.get_listing();
}

void database::generate_named_conf() const
{
    string ret;
    map<string, czone>::const_iterator it = zones.begin();

    ret += "//\n";
    ret += "// WARNING AUTOMATICALLY GENERATED NAMED CONFIG FILE --- DO NOT EDIT\n";
    ret += "//\n";

    while(it != zones.end())
    {
	ret += tools_printf("\n");

	ret += tools_printf("zone \"%S.%S\" in {\n", &(it->first), &parent_zone);
	ret += tools_printf("type master;\n");
	ret += tools_printf("file \"%S\";\n", get_forward_zone_filename(it->first));
	ret += tools_printf("};\n");

	ret += tools_printf("\n");

	ret += tools_printf("zone \"%s\" in {\n", tools_arpa_format(it->second.get_subnet_prefix()).c_str());
	ret += tools_printf("type master;\n");
	ret += tools_printf("file \"%S\";\n", get_reverse_zone_filename(it->first));
	ret += tools_printf("};\n");

	++it;
    }

    write_to_file(named_conf, ret);
}

void database::generate_forward_file(const string & zone) const
{
    string ret;
    string serial = tools_compute_serial();
    map<string, czone>::const_iterator it = zones.find(zone);

    if(it == zones.end())
	throw erreur(tools_printf("Unknow zone %S", &zone));

    ret += ";\n";
    ret += "; WARNING AUTOMATICALLY GENERATED ZONE FILE --- DO NOT EDIT\n";
    ret += ";\n";
    ret += tools_printf("$ORIGIN %S.%S.\n", &zone, parent_zone);
    ret += tools_printf("$TTL    %d\n", refresh);
    ret += tools_printf("@         IN    SOA    %S.  %S. (\n", &ns, &hostmaster);
    ret += tools_printf("             %S  ; Serial\n", serial);
    ret += tools_printf("             %d  ; Refresh\n", refresh);
    ret += tools_printf("             %d  ; Retry\n", retry);
    ret += tools_printf("             %d  ; Expire\n", expire);
    ret += tools_printf("             %d) ; Neg cache TTL\n", nttl);
    ret += tools_printf("\n");
    ret += tools_printf("          IN    NS     %S.\n", &ns);
    ret += tools_printf("\n");
    ret += it->second.generate_forward_records();

    write_to_file(get_forward_zone_filename(zone), ret);
}



void database::generate_reverse_file(const string & zone) const
{
    string ret;
    string arpa_format;
    string serial = tools_compute_serial();
    map<string, czone>::const_iterator it = zones.find(zone);

    if(it == zones.end())
	throw erreur(tools_printf("Unknow zone %S", &zone));

    arpa_format = tools_arpa_format(it->second.get_subnet_prefix());

    ret += ";\n";
    ret += "; WARNING AUTOMATICALLY GENERATED ZONE FILE --- DO NOT EDIT\n";
    ret += ";\n";
    ret += tools_printf("$ORIGIN %S\n", &arpa_format);
    ret += tools_printf("$TTL    %d\n", refresh);
    ret += tools_printf("@         IN    SOA    %S.  %S. (\n", &ns, &hostmaster);
    ret += tools_printf("             %S  ; Serial\n", serial);
    ret += tools_printf("             %d  ; Refresh\n", refresh);
    ret += tools_printf("             %d  ; Retry\n", retry);
    ret += tools_printf("             %d  ; Expire\n", expire);
    ret += tools_printf("             %d) ; Neg cache TTL\n", nttl);
    ret += tools_printf("\n");
    ret += tools_printf("          IN    NS     %S.\n", &ns);
    ret += tools_printf("\n");
    ret += it->second.generate_reverse_records();

    write_to_file(get_reverse_zone_filename(zone), ret);
}

void database::load_json(const json & source)
{
    unsigned int reading_version = 0;

    zones.clear();

    try
    {
	reading_version = source.at(LABEL_VERSION);
    }
    catch(json::exception & e)
    {
	    // before first release the version field was not present
	    // in the json database, though this is the #1 version
	reading_version = 1;
    }

    try
    {
	json tableau = source.at(LABEL_ZONES);

	parent_zone = source.at(LABEL_PARENT_ZONE);
	named_conf = source.at(LABEL_NAMED_CONF);
	zone_dir = source.at(LABEL_ZONE_DIR);
	ns = source.at(LABEL_NS);
	hostmaster = source.at(LABEL_HOSTMASTER);
	refresh = source.at(LABEL_REFRESH);
	retry = source.at(LABEL_RETRY);
	expire = source.at(LABEL_EXPIRE);
	nttl = source.at(LABEL_NTTL);

	for(json::iterator it = tableau.begin();
	    it != tableau.end();
	    ++it)
	{
	    czone tmp("", "");

	    tmp.load_json(it->at(LABEL_ZONE_CONFIG));
	    zones.emplace(it->at(LABEL_ZONE_NAME), tmp);
	}
    }
    catch(json::exception & e)
    {
	throw erreur("unexpected json structure found for a database object");
    }
}

json database::save_json() const
{
    json ret;
    json tableau;

    ret.emplace(LABEL_VERSION, db_version);
    ret.emplace(LABEL_PARENT_ZONE, parent_zone);
    ret.emplace(LABEL_NAMED_CONF, named_conf);
    ret.emplace(LABEL_ZONE_DIR, zone_dir);
    ret.emplace(LABEL_NS, ns);
    ret.emplace(LABEL_HOSTMASTER, hostmaster);
    ret.emplace(LABEL_REFRESH, refresh);
    ret.emplace(LABEL_RETRY, retry);
    ret.emplace(LABEL_EXPIRE, expire);
    ret.emplace(LABEL_NTTL, nttl);

    for(map<string, czone>::const_iterator it = zones.begin();
	it != zones.end();
	++it)
    {
	json tmp;

	tmp.emplace(LABEL_ZONE_NAME, it->first);
	tmp.emplace(LABEL_ZONE_CONFIG, it->second.save_json());
	tableau.push_back(tmp);
    }

    ret.emplace(LABEL_ZONES, tableau);

    return ret;
}


string database::get_forward_zone_filename(const string & zone) const
{
    return tools_printf("%S/db.%S", zone_dir, zone);
}

string database::get_reverse_zone_filename(const string & zone) const
{
    map<string, czone>::const_iterator it = zones.find(zone);

    if(it == zones.end())
	BUG;

    return tools_printf("%S/db.%S", zone_dir, it->second.get_subnet_prefix());
}

void database::write_to_file(const string & filename, const string & data)
{
    ofstream file(filename);
    if(file.fail())
	throw erreur(tools_printf("could not write to file %S", &filename));
    else
    {
	file << data;
	file.close();
    }
}
