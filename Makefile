#######################################################################
## dnsmag - a bind zone config generator
## Copyright (C) 2026 Denis Corbin
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
##
##  You should have received a copy of the GNU General Public License
##  along with dnsmag.  If not, see <http://www.gnu.org/licenses/>
##
#######################################################################

all: dnsmag

clean:
	rm -f *.o dnsmag

objects = tools.o czone.o database.o main.o

dnsmag: $(objects)
	$(CXX) $(LDFLAGS) $(objects) -o $@

czone.o: czone.cpp czone.hpp erreurs.hpp tools.hpp jsoner.hpp
tools.o: tools.cpp tools.hpp erreurs.hpp
database.o: database.cpp database.hpp erreurs.hpp tools.hpp jsoner.hpp
main.o: main.cpp database.hpp erreurs.hpp tools.hpp


install: dnsmag
	install -m 0755 -d $(DESTDIR)/usr/local/bin
	install -m 0755 -s -t $(DESTDIR)/usr/local/bin dnsmag

uninstall:
	rm -f $(DESTDIR)/usr/local/bin/dnsmag
