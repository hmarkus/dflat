/*
Copyright 2012-2013, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

This file is part of D-FLAT.

D-FLAT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

D-FLAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdexcept>
#include "Driver.h"
#include "parser.h"

namespace parser {

Driver::Driver(Problem& problem, const std::string& input)
	: problem(problem), input(input)
{
}

Driver::~Driver()
{
}

void Driver::parse()
{
	scan_begin();
	::yy::Parser parser(*this);
	int res = parser.parse();
	scan_end();
	if(res != 0)
		throw std::runtime_error("Parse error");
}

void Driver::error(const yy::location& l, const std::string& m)
{
	std::ostringstream ss;
	ss << "Parse error." << std::endl << l << ": " << m;
	throw std::runtime_error(ss.str());
}

void Driver::reportFact(const std::string& predicate, const Terms* arguments)
{
	problem.parsedFact(predicate, arguments);
}

} // namespace parser