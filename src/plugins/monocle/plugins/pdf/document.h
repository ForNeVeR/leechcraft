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

#include <memory>
#include <QObject>
#include <interfaces/monocle/idocument.h>

namespace Poppler
{
	class Document;
}

namespace LeechCraft
{
namespace Monocle
{
namespace PDF
{
	typedef std::shared_ptr<Poppler::Document> PDocument_ptr;

	class Document : public QObject
				   , public IDocument
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Monocle::IDocument)

		PDocument_ptr PDocument_;
	public:
		Document (const QString&, QObject* = 0);

		bool IsValid () const;
		DocumentInfo GetDocumentInfo () const;
		int GetNumPages () const;
		QSize GetPageSize (int) const;
		QImage RenderPage (int, double, double);
		QList<ILink_ptr> GetPageLinks (int);
	};
}
}
}
