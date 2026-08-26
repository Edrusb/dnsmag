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
#include <time.h>
#include <stdarg.h>
#include <string.h>
}
#include <sstream>

#include "erreurs.hpp"
#include "tools.hpp"

static string tools_vprintf(const string & format, va_list ap);

string tools_printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    string output = "";
    try
    {
	output = tools_vprintf(format, ap);
    }
    catch(...)
    {
	va_end(ap);
	throw;
    }
    va_end(ap);
    return output;
}



static string tools_vprintf(const string & format, va_list ap)
{
    ostringstream res;
    string::const_iterator ptr = format.begin();
    string::const_iterator start = format.begin();

    do
    {
	while(ptr != format.end() && *ptr != '%')
	    ++ptr;

	res << string(start, ptr);

	if(ptr != format.end()) // thus (*ptr == '%') is true
	{
	    ++ptr;
	    switch(*ptr)
	    {
	    case '%':
		res << "%";
		break;
	    case 'd':
		res << va_arg(ap, signed int);
		break;
	    case 'u':
		res << va_arg(ap, unsigned int);
		break;
	    case 's':
		res << va_arg(ap, char *);
		break;
	    case 'c':
		res << static_cast<char>(va_arg(ap, signed int));
		break;
	    case 'S':
		res << *(va_arg(ap, string *));
		break;
	    default:
		throw erreur(tools_printf("%%%c is not implemented in tools_printf format argument", *ptr));
	    }
	    ++ptr;
	    start = ptr;
	}
    }
    while(ptr != format.end());

    return res.str();
}

string tools_truncate_subnet(const string & subnet)
{
    enum { byte, dot } status = dot;        ///< state machine for parsing IPv4 address
    unsigned int byte_num = 0;              ///< byte number of the IP@ currently parsed
    unsigned int val = 0;                   ///< value of the last byte read (when status is dot)
    string::const_iterator end_subnet_part = subnet.begin();
    string::const_iterator it = subnet.begin();
    string ret;

    try
    {
	while(it != subnet.end())
	{
	    switch(status)
	    {
	    case dot:
		if(*it < '0' || *it > '9')
		    throw erreur("");
		status = byte;
		++byte_num;
		val = 0;
		break;
	    case byte:
		if(*it >= '0' && *it <= '9')
		    val = val*10 + (*it - '0');
		else
		    if(*it == '.')
		    {
			if(val > 255)
			    throw erreur("");
			status = dot;
		    }
		    else
			throw erreur("");
		break;
	    default:
		BUG;
	    }

	    if(status == dot && byte_num == 3)
		end_subnet_part = it;

	    ++it;
	}

	if(status != byte || byte_num != 4 || val > 255)
	    throw erreur("");
    }
    catch(erreur & e)
    {
	if(e.get_message().empty())
	    throw erreur(tools_printf("Invalid IPv4 address given: %S", &subnet));
	else
	    throw;
    }

    return string(subnet.begin(), end_subnet_part);
}



string tools_compute_serial()
{
    return tools_printf("%d", time(nullptr));
}


string tools_arpa_format(const string & subnet)
{
    unsigned int first_dot;
    unsigned int second_dot;

    first_dot = subnet.find_first_of(".");
    if(first_dot == string::npos)
	BUG;

    second_dot = subnet.find_first_of(".", first_dot + 1);
    if(second_dot == string::npos)
	BUG;

    string tmp1 = string(subnet, second_dot + 1, subnet.length() - second_dot).c_str();
    string tmp2 = string(subnet, first_dot + 1 , second_dot - first_dot - 1  ).c_str();
    string tmp3 = string(subnet, 0             , first_dot                   ).c_str();

    return tools_printf("%S.%S.%S.IN-ADDR.ARPA.",
			&tmp1,
			&tmp2,
			&tmp3);
}

