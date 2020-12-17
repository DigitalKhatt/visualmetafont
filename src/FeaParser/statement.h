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

#ifndef H_FEAAST_STATEMENT
#define H_FEAAST_STATEMENT

namespace feayy {

	class Visitor;


	class Statement {
	public:
		virtual void accept(Visitor&) = 0;
		virtual ~Statement() {}
	};

	class LookupStatement : public Statement {
	public:
		void accept(Visitor&) override;
	};
}
#endif // H_FEAAST_STATEMENT