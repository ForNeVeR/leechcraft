/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QFileInfo>
#include <QStringList>
#include <interfaces/lmp/mediainfo.h>

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	class FilesModel : public QAbstractItemModel
	{
		Q_OBJECT

		const QStringList Headers_;

		struct File
		{
			QString Path_;

			QString Name_;

			MediaInfo Info_;
			MediaInfo OrigInfo_;
			bool IsChanged_;

			File (const QFileInfo&);
		};
		QList<File> Files_;

		enum Columns
		{
			Title,
			Album,
			Artist,
			Filename,

			MaxColumn
		};
	public:
		enum Roles
		{
			MediaInfoRole = Qt::UserRole + 1,
			OrigMediaInfo
		};

		FilesModel (QObject*);

		QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
		QModelIndex parent (const QModelIndex&) const;
		int rowCount (const QModelIndex&) const;
		int columnCount (const QModelIndex&) const;
		QVariant headerData (int, Qt::Orientation, int) const;
		QVariant data (const QModelIndex&, int) const;

		void AddFiles (const QList<QFileInfo>&);

		void SetInfos (const QList<MediaInfo>&);
		void UpdateInfo (const QModelIndex&, const MediaInfo&);

		void Clear ();

		QModelIndex FindIndex (const QString& path) const;

		QList<QPair<MediaInfo, MediaInfo>> GetModified () const;
	private:
		QList<File>::iterator FindFile (const QString&);
		QList<File>::const_iterator FindFile (const QString&) const;
	};
}
}
}
