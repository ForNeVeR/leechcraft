/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "core.h"
#include "todomanager.h"
#include "deltagenerator.h"

namespace LeechCraft
{
namespace Otlozhu
{
	Core::Core ()
	: TodoManager_ (new TodoManager ("Default", this))
	, DeltaGen_ (new DeltaGenerator (this))
	{
		connect (TodoManager_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	void Core::SendEntity (const Entity& e)
	{
		emit gotEntity (e);
	}

	TodoManager* Core::GetTodoManager () const
	{
		return TodoManager_;
	}

	DeltaGenerator* Core::GetDeltaGenerator () const
	{
		return DeltaGen_;
	}
}
}
