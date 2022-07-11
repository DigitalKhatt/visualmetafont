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

#ifndef VM_FSMDRIVER_H
#define VM_FSMDRIVER_H

#include  <functional>
#include <unordered_map>
#include <string>

class OtLayout;
class FSMSubtable;
struct hb_buffer_t;
namespace OT {
  struct hb_ot_apply_context_t;
}


struct FSMDriver  {
public:

	struct Pos {
		int state;
		int pos;
	};

	using ActionType = std::function<void(hb_buffer_t& buffer, const std::vector<Pos>& match_positions, int pos, const std::unordered_map<std::string,int>&  aliases)>;
	using ConditionType = std::function<void(hb_buffer_t& buffer, std::vector<Pos> match_positions, int pos, std::unordered_map<std::string, int>  aliases)>;

	FSMDriver(OtLayout& layout);

	void executeFSM(FSMSubtable& table, OT::hb_ot_apply_context_t* context);

private:
	OtLayout& layout;
	std::unordered_map<std::string, ActionType> actions;
	std::unordered_map<std::string, ActionType> conditions;
};


#endif
