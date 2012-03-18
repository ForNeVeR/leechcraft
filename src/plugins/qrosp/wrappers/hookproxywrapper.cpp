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

#include "hookproxywrapper.h"
#include <interfaces/core/ihookproxy.h>

namespace LeechCraft
{
namespace Qrosp
{
	HookProxyWrapper::HookProxyWrapper (IHookProxy_ptr proxy)
	: Proxy_ (proxy)
	{
	}

	void HookProxyWrapper::CancelDefault ()
	{
		Proxy_->CancelDefault ();
	}

	const QVariant& HookProxyWrapper::GetReturnValue () const
	{
		return Proxy_->GetReturnValue ();
	}

	void HookProxyWrapper::SetReturnValue (const QVariant& val)
	{
		Proxy_->SetReturnValue (val);
	}

	void HookProxyWrapper::SetValue (const QByteArray& name, const QVariant& value)
	{
		Proxy_->SetValue (name, value);
	}
}
}
