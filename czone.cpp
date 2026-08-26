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

#include "czone.hpp"
#include "erreurs.hpp"
#include "tools.hpp"

czone::czone(const string & fqdn_zone_name,
	     const string & subnet_prefix):
    zone(fqdn_zone_name),
    subnet(subnet_prefix)
{
    clear();
}

void czone::clear()
{
    asso.clear();
    free_slots.clear();

	// we do not use neither 0 nor 255 values
    for(unsigned int i = 1; i < 255; ++i)
	free_slots.insert(i);

	// not using insert_range() from C++23 to
	// reduce constraints on supported compilers
}

string czone::add_record(const string & name)
{
    map<string, unsigned int>::iterator it = asso.find(name);
    unsigned int ip_byte;

    if(it != asso.end())
	throw erreur(tools_printf("Name %S already used in the %S zone", &name, &zone));

    if(free_slots.empty())
	throw erreur(tools_printf("Zone %S is full, no more IP available", &zone));

    ip_byte = *(free_slots.begin()); // using the smallest available IP
    free_slots.erase(ip_byte);

    asso[name] = ip_byte;

    return tools_printf("%S.%d", subnet, ip_byte);
}

void czone::del_record(const string & name)
{
    map<string, unsigned int>::iterator it = asso.find(name);

    if(it != asso.end())
    {
	free_slots.insert(it->second);
	asso.erase(it);
    }
    else
	throw erreur(tools_printf("the name %S does currently not exist in zone %S", &name, &zone));
}

string czone::generate_forward_records() const
 {
    map<string, unsigned int>::const_iterator it;
    string ret;

    for(it = asso.begin(); it != asso.end(); ++it)
	ret += tools_printf("%S\tIN\tA\t%S.%d\n",
			    it->first,
			    subnet,
			    it->second);

    return ret;
}

string czone::generate_reverse_records() const
{
    map<string, unsigned int>::const_iterator it;
    string ret;

    for(it = asso.begin(); it != asso.end(); ++it)
	ret += tools_printf("%d\tIN\tPTR\t%S.%S.\n",
			    it->second,
			    it->first,
			    zone);

    return ret;
}

deque<string> czone::get_listing() const
{
    deque<string> ret;
    string padding;

    for(map<string, unsigned int>::const_iterator it = asso.begin();
	it != asso.end();
	++it)
    {
	if(it->second < 10)
	    padding = "  ";
	else if(it->second < 100)
	    padding = " ";
	else
	    padding = "";
	ret.push_back(tools_printf("%S%d\t%S", &padding, it->second, &(it->first)));
    }
    sort(ret.begin(), ret.end());

    return ret;
}

void czone::load_json(const json & source)
{
    clear();

    try
    {
	json tableau = source.at(LABEL_ASSO);
	zone = source.at(LABEL_ZONE);
	subnet = source.at(LABEL_SUBNET);

	for(json::iterator it = tableau.begin();
	    it != tableau.end();
	    ++it)
	{
	    asso.emplace(it->at(LABEL_NAME), it->at(LABEL_IP));
	    free_slots.erase(it->at(LABEL_IP));
	}
    }
    catch(json::exception & e)
    {
	throw erreur("Unexecpted json structure found for a czone object");
    }
}

json czone::save_json() const
{
    json ret;
    json tableau;

    ret.emplace(LABEL_ZONE, zone);
    ret.emplace(LABEL_SUBNET, subnet);

    for(map<string, unsigned int>::const_iterator it = asso.begin();
	it != asso.end();
	++it)
    {
	json tmp;

	tmp.emplace(LABEL_NAME, it->first);
	tmp.emplace(LABEL_IP, it->second);
	tableau.push_back(tmp);
    }

    ret.emplace(LABEL_ASSO, tableau);

    return ret;
}
