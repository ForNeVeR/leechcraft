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

#include "basehookinterconnector.h"
#include <QMetaMethod>
#include <QtDebug>

namespace LeechCraft
{
	namespace Util
	{
		BaseHookInterconnector::BaseHookInterconnector (QObject *parent)
		: QObject (parent)
		{
		}

		BaseHookInterconnector::~BaseHookInterconnector ()
		{
		}

		namespace
		{
#define LC_N(a) (QMetaObject::normalizedSignature(a))
#define LC_TOSLOT(a) ('1' + QByteArray(a))
#define LC_TOSIGNAL(a) ('2' + QByteArray(a))
			void ConnectHookSignals (QObject *sender, QObject *receiver,
					bool destSlot)
			{
				const QMetaObject *mo = sender->metaObject ();
				for (int i = 0, size = mo->methodCount ();
						i < size; ++i)
				{
					QMetaMethod method = mo->method (i);
					if (method.methodType () != QMetaMethod::Signal)
						continue;
					if (method.parameterTypes ().size () == 0)
						continue;
					if (method.parameterTypes ().at (0) != "LeechCraft::IHookProxy_ptr")
						continue;

					if (receiver->metaObject ()->
							indexOfMethod (LC_N (method.signature ())) == -1)
					{
						if (!destSlot)
						{
							qWarning () << Q_FUNC_INFO
									<< "not found meta method for"
									<< method.signature ()
									<< "in receiver object"
									<< receiver;
						}
						continue;
					}

					if (!QObject::connect (sender,
							LC_TOSIGNAL (method.signature ()),
							receiver,
							destSlot ? LC_TOSLOT (method.signature ()) : LC_TOSIGNAL (method.signature ()),
							Qt::UniqueConnection))
					{
						qWarning () << Q_FUNC_INFO
								<< "connect for"
								<< sender
								<< "->"
								<< receiver
								<< ":"
								<< method.signature ()
								<< "failed";
					}
				}
			}
#undef LC_N
		};

		void BaseHookInterconnector::AddPlugin (QObject *plugin)
		{
			Plugins_.push_back (plugin);

			ConnectHookSignals (this, plugin, true);
		}

		void BaseHookInterconnector::RegisterHookable (QObject *object)
		{
			ConnectHookSignals (object, this, false);
		}
	};
};
