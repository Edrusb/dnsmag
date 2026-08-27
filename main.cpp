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
#include <string.h>
#include <errno.h>
#include <stdlib.h>
}

#include <fstream>
#include <iostream>
#include <memory>
#include "database.hpp"
#include "tools.hpp"
#include "erreurs.hpp"

constexpr const char* database_config_path = "/usr/local/etc/dnsmag.json";
constexpr const char* database_environment_var = "DNSMAG_BASE_PATH";

void usage(const string & argv0, const char** env);

void save_database(const database & base, const string & filename);

void update_named();
void display_base_info(const char** env);

    /// returns value of the provided variable (=clef) if it exist, nullptr if not.
const char *get_from_env(const char** env, const char *clef);

int main(int argc, char* argv[], const char** env)
{
    const char* base_path = get_from_env(env, database_environment_var);

    if(base_path == nullptr)
	base_path = database_config_path;

    if(argc == 2 && string(argv[1]) == "help" || argc < 2)
	usage(argv[0], env);

    try
    {
	ifstream f(base_path);

	if(f.fail())
	    if(argc < 2 || strcmp("init", argv[1]) != 0)
	    {
		display_base_info(env);
		throw erreur(tools_printf("no database found at %s, use the init subcommand to create it", base_path));
	    }
	    else
	    {
		if(argc == 7)
		{
		    string parent_zone = argv[2];
		    string named_conf = argv[3];
		    string zone_dir = argv[4];
		    string ns = argv[5];
		    string hostmaster = argv[6];

		    database base(parent_zone,
				  named_conf,
				  zone_dir,
				  ns,
				  hostmaster);

		    save_database(base, base_path);
		}
		else
		    usage(argv[0], env);
	    }
	else
	{
	    json data = json::parse(f);
	    database base(data);
	    string verbe = argv[1];

	    f.close(); // need to be closed now to be able to update the database

	    if(verbe == "init")
		throw erreur(tools_printf("A database already exists at %s, will not erase it", base_path));
	    else if(argc == 4 && verbe == "add-zone")
	    {
		base.create_zone(argv[2], argv[3]);
		save_database(base, base_path);
		update_named();
		cout << tools_printf("Zone %s added", argv[2]) << endl;
	    }
	    else if(argc == 3 && verbe == "del-zone")
	    {
		base.delete_zone(argv[2]);
		save_database(base, base_path);
		update_named();
		cout << tools_printf("Zone %s removed", argv[2]) << endl;
	    }
	    else if(argc == 4 && verbe == "add-rec")
	    {
		string ip = base.add_record(argv[2], argv[3]);
		save_database(base, base_path);
		update_named();
		cout << tools_printf("The record %s in zone %s has been associated to the IP %S",
				     argv[3],
				     argv[2],
				     &ip) << endl;
	    }
	    else if(argc == 5 && verbe == "add-rec")
	    {
		unsigned int last_byte;

		try
		{
		    last_byte = stoi(string(argv[4]));
		}
		catch(...)
		{
		    throw erreur(tools_printf("last byte argument is not a integer: %s\n", argv[4]));
		}

		if(last_byte < 1 || last_byte > 254)
		    throw erreur(tools_printf("last byte should be in the range [1..254], while it was read as %u",
					      last_byte));

		base.add_record(argv[2], argv[3], last_byte);
		save_database(base, base_path);
		update_named();
		cout << tools_printf("The record %s in zone %s has been associated to %s.%u",
				     argv[3],
				     argv[2],
				     base.show_zone_subnet(argv[2]).c_str(),
				     last_byte) << endl;
	    }
	    else if(argc == 4 && verbe == "del-rec")
	    {
		base.del_record(argv[2], argv[3]);
		save_database(base, base_path);
		update_named();
		cout << tools_printf("Record %s has been removed from zone %s",
				     argv[3],
				     argv[2]) << endl;
	    }
	    else if(argc == 2 && verbe == "list")
	    {
		deque<string> list = base.list_zones();
		cout << tools_printf("List of created zones in the database:\n");
		for(deque<string>::iterator it = list.begin();
		    it != list.end();
		    ++it)
		    cout << tools_printf("    %S\n", &(*it));
		cout << endl << endl;
	    }
	    else if(argc == 3 && verbe == "show")
	    {
		deque<string> content = base.show_zone_listing(argv[2]);
		string subnet = base.show_zone_subnet(argv[2]) + ".0";

		cout << tools_printf("Information on zone: %s\n", argv[2]);
		cout << tools_printf(" - subnet used: %S\n", &subnet);
		cout << tools_printf(" - recorded entries:\n");

		for(deque<string>::iterator it = content.begin();
		    it != content.end();
		    ++it)
		    cout << tools_printf("     %S\n", &(*it));

		cout << endl << endl;
	    }
	    else
		usage(argv[0], env);
	}
    }
    catch(erreur & e)
    {
	cerr << "Program aborted due to: " << e.get_message() << endl;
    }
}


void usage(const string & argv0, const char** env)
{
    cout << endl;
    cout << tools_printf("usage: %S add-zone <zone-name> <subnet>\n", argv0);
    cout << tools_printf("       %S del-zone <zone-name>\n", argv0);
    cout << tools_printf("       %S add-rec  <zone-name> <record-name> [<last IP byte>]\n", argv0);
    cout << tools_printf("       %S del-rec  <zone-name> <record-name>\n", argv0);
    cout << tools_printf("       %S list\n", argv0);
    cout << tools_printf("       %S show     <zone-name>\n", argv0);
    cout << tools_printf("       %S init     <parent-zone-name> <named.conf-file> <zone-dir-path> <namesever-name>  <hostmaster>\n", argv0);
    cout << tools_printf("       %S help\n", argv0);
    cout << endl;
    display_base_info(env);
    cout << endl;
    exit(1);
}


void save_database(const database & base, const string & filename)
{
    json data = base.save_json();

    ofstream file(filename);
    if(file.fail())
	throw erreur(tools_printf("could not write to file %s: %s",
				  filename,
				  strerror(errno)));
    else
    {
	file << data;
	file.close();
    }
}

const char *get_from_env(const char** env, const char* clef)
{
    unsigned int index = 0;
    const char* ret = nullptr;

    if(env == nullptr || clef == nullptr)
        return nullptr;

    while(ret == nullptr && env[index] != nullptr)
    {
        unsigned int letter = 0;
        while(clef[letter] != '\0'
              && env[index][letter] != '\0'
              && env[index][letter] != '='
              && clef[letter] == env[index][letter])
            letter++;
        if(clef[letter] == '\0' && env[index][letter] == '=')
            ret = env[index]+letter+1;
        else
            index++;
    }

    return ret;
}

void update_named()
{
	// triggering the named process to reload its configuration
    if(system("sudo rndc reload") == 0)
	cout << "bind process has reloaded his configuration" << endl;
    else
	cout << "Failed to have bind reloading its configuration" << endl;
}

void display_base_info(const char** env)
{
    const char* base_path = get_from_env(env, database_environment_var);

    if(base_path != nullptr)
	cout << tools_printf("Database located at %s as indicated by the environment variable %s\n",
			     base_path,
			     database_environment_var);
    else
	cout << tools_printf("Databse located at %s, can be modified using environment variable %s\n",
			     database_config_path,
			     database_environment_var);
}
