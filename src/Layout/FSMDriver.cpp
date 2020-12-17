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

#include "FSMDriver.h"
#include "Subtable.h"
#include <hb-ot-layout-common.hh>
#include <iostream>
#include <OtLayout.h>
#include "hb-ot-layout-gsub-table.hh"


FSMDriver::FSMDriver(OtLayout& layout) : layout{ layout } {

	auto action1 = [&layout= layout](hb_buffer_t& buffer, std::vector<Pos> match_positions, int pos, std::unordered_map<std::string, int>  aliases) {
		
		auto bufferIndex = match_positions[pos].pos - 1;
		buffer.info[bufferIndex].lefttatweel = 7;
		cout << "action1 from lammbda at position " << pos << " and index " << bufferIndex << " at glyph " << layout.glyphNamePerCode.value(buffer.info[bufferIndex].codepoint).toStdString() << '\n';
	};

	auto action2 = [&layout = layout](hb_buffer_t& buffer, std::vector<Pos> match_positions, int pos, std::unordered_map<std::string, int>  aliases) {

		auto bufferIndex = match_positions[pos].pos - 1;		
		cout << "action2 from lammbda at position " << pos << " and index " << bufferIndex << " at glyph " << layout.glyphNamePerCode.value(buffer.info[bufferIndex].codepoint).toStdString() << '\n';
	};

	actions.insert({ "action1",action1 });
	actions.insert({ "action2",action2 });
}

void FSMDriver::executeFSM(FSMSubtable& table, OT::hb_ot_apply_context_t* context) {

  auto buffer = context->buffer;

	auto& dfa = table.dfa;

	if (dfa.backupStates.size() == 0) return;

	//buffer->idx = 0;
	bool applied = false;
	//buffer->clear_output();

	

	std::vector<Pos> accumulatedStates;
	std::unordered_map<std::string, int> aliases;
	int lastfinal;
	bool restart = true;
	int currentIndex = -1;
	DFASTate* currentState = nullptr;
	int currentPos = 0;

	while (currentPos < buffer->len && buffer->successful) {

		if (restart) {

			lastfinal = -1;

			if (currentPos < dfa.minBackup) {
				currentPos++;
				continue;
			}

			int max = dfa.maxBackup;

			if (currentPos < dfa.maxBackup) {
				max = currentPos;
			}

			int backup = max - dfa.minBackup;

			auto currentIndex = dfa.backupStates[backup];
			currentState = &dfa.states[currentIndex];

			currentPos-= max;

			buffer->move_to(currentPos);

			accumulatedStates.push_back({ currentIndex,currentPos });
		}

		restart = false;

		int nextIndex = -1;

		auto codepoint = buffer->info[currentPos].codepoint;

		const auto& it = currentState->transtitions.find(codepoint);
		if (it != currentState->transtitions.end()) {
			nextIndex = (*it).second;
		}
		else {
			const auto& any = currentState->transtitions.find(0xFFFF);
			if (any != currentState->transtitions.end()) {
				nextIndex = (*any).second;
			}
		}

		if (nextIndex != -1) {
			currentState = &dfa.states[nextIndex];
			accumulatedStates.push_back({ nextIndex, currentPos + 1 });
			if (currentState->final != 0) {
				lastfinal = currentState->final;
			}
			currentPos++;
			if (currentPos < buffer->len) continue;
		}
		//current character not matched or end on buffer
		if (lastfinal != -1) {
			//take the last sequence matched			
			int nextposaction = -1;
			int nextposrest = -1;
			int posIndex = 0;
			for (auto pos : accumulatedStates) {
				auto& state = dfa.states[pos.state];
				if (state.resetPosition) {
					nextposrest = pos.pos;
				}
				const auto& actions = state.actions.find(lastfinal);
				if (actions != state.actions.end()) {
					for (auto& action : (*actions).second) {						
						const auto& it = this->actions.find(action.name);
						if (it != this->actions.end()) {
							(*it).second(*buffer, accumulatedStates, posIndex, aliases);
						}
					}
					
					nextposaction = pos.pos;
					
				}					
				posIndex++;
				if (state.final == lastfinal)
					break;
			}
			

			currentPos = nextposrest != -1 ? nextposrest : nextposaction != -1 ? nextposaction : currentPos + 1;

			std::cout << "Current Pos = " << currentPos << '\n';
		}
		else {
			//no match
			currentPos++;
		}

		accumulatedStates.clear();
		restart = true;

	}

  buffer->next_glyphs(buffer->len - buffer->idx);

	//buffer->swap_buffers();


}
