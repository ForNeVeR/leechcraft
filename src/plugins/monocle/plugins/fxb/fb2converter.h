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

#include <functional>
#include <QString>
#include <QObject>
#include <QHash>
#include <QStack>
#include <interfaces/monocle/idocument.h>
#include <interfaces/monocle/ihavetoc.h>

class QTextCharFormat;
class QTextCursor;
class QDomElement;
class QDomDocument;
class QTextDocument;

namespace LeechCraft
{
namespace Monocle
{
namespace FXB
{
	class Document;

	class FB2Converter : public QObject
	{
		Document *ParentDoc_;

		const QDomDocument& FB2_;

		QTextDocument *Result_;
		DocumentInfo DocInfo_;
		TOCEntryLevel_t TOC_;

		QStack<TOCEntry*> CurrentTOCStack_;

		QTextCursor *Cursor_;

		int SectionLevel_;

		typedef std::function<void (QDomElement)> Handler_f;
		QHash<QString, Handler_f> Handlers_;

		QString Error_;
	public:
		FB2Converter (Document*, const QDomDocument&);
		~FB2Converter ();

		QString GetError () const;
		QTextDocument* GetResult () const;
		DocumentInfo GetDocumentInfo () const;
		TOCEntryLevel_t GetTOC () const;
	private:
		void HandleDescription (const QDomElement&);
		void HandleBody (const QDomElement&);

		void HandleSection (const QDomElement&);
		void HandleTitle (const QDomElement&, int = 0);
		void HandleEpigraph (const QDomElement&);
		void HandleImage (const QDomElement&);

		void HandlePara (const QDomElement&);
		void HandleParaWONL (const QDomElement&);
		void HandlePoem (const QDomElement&);
		void HandleStanza (const QDomElement&);
		void HandleEmptyLine (const QDomElement&);

		void Handle (const QDomElement&);

		void HandleMangleCharFormat (const QDomElement&,
				std::function<void (QTextCharFormat&)>, Handler_f);

		void FillPreamble ();

		void AddImage (const QDomElement&);
	};
}
}
}
