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

#pragma once

#include <QObject>
#include <poppler-link.h>
#include <interfaces/monocle/ilink.h>

namespace LeechCraft
{
namespace Monocle
{
namespace PDF
{
	class Document;

	class Link : public QObject
			   , public ILink
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Monocle::ILink)

	protected:
		Document *Doc_;
		std::shared_ptr<Poppler::Link> Link_;
	public:
		explicit Link (Document*, Poppler::Link*);

		LinkType GetLinkType () const;
		QRectF GetArea () const;

		void Execute ();
	private:
		void ExecutePageLink ();
	};

	class TOCLink : public QObject
				  , public ILink
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Monocle::ILink)
	protected:
		Document *Doc_;
		Poppler::LinkDestination *Dest_;
	public:
		TOCLink (Document*, Poppler::LinkDestination*);

		LinkType GetLinkType () const;
		QRectF GetArea () const;

		void Execute ();
	};
}
}
}
