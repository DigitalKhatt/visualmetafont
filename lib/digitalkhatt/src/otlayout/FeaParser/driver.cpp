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
#include <utility>

#include "driver.h"
#include "scanner.h"
#include <filesystem>



namespace feayy {

	Driver::Driver(class FeaContext& _fea)
		: trace_scanning(false),
		trace_parsing(false),
		lexer(nullptr),
		context(_fea)
	{
	}

	bool Driver::parse_stream(std::istream& in, const std::string& sname)
	{
		auto previousStream = std::exchange(streamname, sname);

		Scanner scanner(&in);
		scanner.set_debug(trace_scanning);

		auto prevLexer = this->lexer;
		this->lexer = &scanner;

		Parser parser(*this);
		parser.set_debug_level(trace_parsing);

		const auto conditionDepth = conditions.size();
		auto ret = parser.parse();

		conditions.resize(conditionDepth);
		this->lexer = prevLexer;
		streamname = std::move(previousStream);


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

	bool Driver::inActiveBranch() const
	{
		return conditions.empty() || conditions.back().active;
	}

	bool Driver::beginCondition(std::string_view name)
	{
		conditions.push_back({inActiveBranch()});
		return selectBranch(name);
	}

	bool Driver::selectBranch(std::optional<std::string_view> name)
	{
		auto& condition = conditions.back();
		condition.active = condition.parentActive && !condition.matched &&
			(!name || context.evaluateCondition(*name));
		condition.matched |= condition.active;
		return condition.active;
	}

	void Driver::endCondition()
	{
		conditions.pop_back();
	}

	int Driver::registerJustificationGlyphSet(GlyphSet* glyphSet)
	{
		if (!inActiveBranch()) {
			delete glyphSet;
			return -1;
		}
		context.justificationGlyphSets.emplace_back(glyphSet);
		return static_cast<int>(context.justificationGlyphSets.size()) - 1;
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
