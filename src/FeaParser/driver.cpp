/*
 * Copyright (c) 2015-2020 Amine Anane. http: //digitalkhatt/license
 * This file is part of DigitalKhatt.
 *
 * DigitalKhatt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DigitalKhatt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with DigitalKhatt. If not, see
 * <https: //www.gnu.org/licenses />.
*/

#include <fstream>
#include <sstream>

#include "driver.h"
#include "scanner.h"
#include <filesystem>



namespace feayy {

	Driver::Driver(class FeaContext& _fea)
		: trace_scanning(false),
		trace_parsing(false),
		context(_fea)
	{
	}

	bool Driver::parse_stream(std::istream& in, const std::string& sname)
	{
		streamname = sname;

		Scanner scanner(&in);
		scanner.set_debug(trace_scanning);

		auto prevLexer = this->lexer;
		this->lexer = &scanner;

		Parser parser(*this);
		parser.set_debug_level(trace_parsing);

		auto ret = parser.parse();

		this->lexer = prevLexer;


		return (ret == 0);
	}

	bool Driver::parse_file(const std::string &filename)
	{
		std::filesystem::path path(filename);

		if (path.is_absolute()) {
			includeNames.push_back(path);
		} else {
			if(!includeNames.empty()){
				auto topPath = includeNames.front();
				auto dir = topPath.parent_path();
				path = dir / path;
			}
			
			includeNames.push_back(path);
		}

		bool ret = false;

		std::ifstream in(path);
		if (in.good()){
			ret = parse_stream(in, path);
		}

		includeNames.pop_back();

		return ret;
	}

	bool Driver::parse_string(const std::string &input, const std::string& sname)
	{
		std::istringstream iss(input);
		return parse_stream(iss, sname);
	}

	void Driver::error(const class location& l,
		const std::string& m)
	{
		std::cerr << l << ": " << m << std::endl;
	}

	void Driver::error(const std::string& m)
	{
		std::cerr << m << std::endl;
	}

}