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

#include "artistsinfodisplay.h"
#include <algorithm>
#include <QStandardItemModel>
#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QDeclarativeImageProvider>
#include <QDeclarativeEngine>
#include <util/util.h>
#include "core.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class SysIconProvider : public QDeclarativeImageProvider
		{
			ICoreProxy_ptr Proxy_;
		public:
			SysIconProvider (ICoreProxy_ptr proxy)
			: QDeclarativeImageProvider (Pixmap)
			, Proxy_ (proxy)
			{
			}

			QPixmap requestPixmap (const QString& id, QSize *size, const QSize& requestedSize)
			{
				const auto& icon = Proxy_->GetIcon (id);

				const auto& getSize = requestedSize.width () > 2 && requestedSize.height () > 2 ?
						requestedSize :
						QSize (48, 48);
				if (size)
					*size = icon.actualSize (getSize);
				return icon.pixmap (getSize);
			}
		};

		class SimilarModel : public QStandardItemModel
		{
		public:
			enum Role
			{
				ArtistName = Qt::UserRole + 1,
				Similarity,
				ArtistImageURL,
				ArtistBigImageURL,
				ArtistPageURL,
				ArtistTags,
				ShortDesc,
				FullDesc,
				IsInCollection
			};

			SimilarModel (QObject *parent = 0)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> names;
				names [ArtistName] = "artistName";
				names [Similarity] = "similarity";
				names [ArtistImageURL] = "artistImageURL";
				names [ArtistBigImageURL] = "artistBigImageURL";
				names [ArtistPageURL] = "artistPageURL";
				names [ArtistTags] = "artistTags";
				names [ShortDesc] = "shortDesc";
				names [FullDesc] = "fullDesc";
				names [IsInCollection] = "artistInCollection";
				setRoleNames (names);
			}
		};
	}

	ArtistsInfoDisplay::ArtistsInfoDisplay (QWidget *parent)
	: QDeclarativeView (parent)
	, Model_ (new SimilarModel (this))
	{
		engine ()->addImageProvider ("sysIcons", new SysIconProvider (Core::Instance ().GetProxy ()));
		rootContext ()->setContextProperty ("similarModel", Model_);
		setSource (QUrl ("qrc:/lmp/resources/qml/SimilarView.qml"));

		connect (rootObject (),
				SIGNAL (bookmarkArtistRequested (QString, QString, QString)),
				this,
				SLOT (handleBookmark (QString, QString, QString)));
	}

	void ArtistsInfoDisplay::SetSimilarArtists (Media::SimilarityInfos_t infos)
	{
		Model_->clear ();

		std::sort (infos.begin (), infos.end (),
				[] (const Media::SimilarityInfo& left, const Media::SimilarityInfo& right)
					{ return left.Similarity_ > right.Similarity_; });

		const auto col = Core::Instance ().GetLocalCollection ();
		Q_FOREACH (const Media::SimilarityInfo& info, infos)
		{
			auto item = new QStandardItem ();

			const auto& artist = info.Artist_;
			item->setData (artist.Name_, SimilarModel::Role::ArtistName);
			item->setData (artist.Image_, SimilarModel::Role::ArtistImageURL);
			item->setData (artist.LargeImage_, SimilarModel::Role::ArtistBigImageURL);
			item->setData (artist.ShortDesc_, SimilarModel::Role::ShortDesc);
			item->setData (artist.FullDesc_, SimilarModel::Role::FullDesc);
			item->setData (artist.Page_, SimilarModel::Role::ArtistPageURL);

			QString simStr;
			if (info.Similarity_ > 0)
				simStr = tr ("Similarity: %1%")
					.arg (info.Similarity_);
			else if (!info.SimilarTo_.isEmpty ())
				simStr = tr ("Similar to: %1")
					.arg (info.SimilarTo_.join ("; "));
			if (!simStr.isEmpty ())
				item->setData (simStr, SimilarModel::Role::Similarity);

			QStringList tags;
			const int diff = artist.Tags_.size () - 5;
			auto begin = artist.Tags_.begin ();
			if (diff > 0)
				std::advance (begin, diff);
			std::transform (begin, artist.Tags_.end (), std::back_inserter (tags),
					[] (decltype (artist.Tags_.front ()) tag) { return tag.Name_; });
			std::reverse (tags.begin (), tags.end ());
			item->setData (tr ("Tags: %1").arg (tags.join ("; ")), SimilarModel::Role::ArtistTags);

			item->setData (col->FindArtist (artist.Name_) >= 0, SimilarModel::Role::IsInCollection);

			Model_->appendRow (item);
		}
	}

	void ArtistsInfoDisplay::handleBookmark (const QString& name, const QString& page, const QString& tags)
	{
		auto e = Util::MakeEntity (tr ("Check out \"%1\"").arg (name),
				QString (),
				FromUserInitiated | OnlyHandle,
				"x-leechcraft/todo-item");
		e.Additional_ ["TodoBody"] = tags + "<br />" + QString ("<a href='%1'>%1</a>").arg (page);
		e.Additional_ ["Tags"] = QStringList ("music");
		Core::Instance ().SendEntity (e);
	}
}
}
