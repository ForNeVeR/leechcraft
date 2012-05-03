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

#include "nowplayingwidget.h"
#include "mediainfo.h"
#include "core.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	NowPlayingWidget::NowPlayingWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
	}

	ArtistsInfoDisplay* NowPlayingWidget::GetArtistsDisplay () const
	{
		return Ui_.SimilarView_;
	}

	void NowPlayingWidget::SetAlbumArt (const QPixmap& px)
	{
		Ui_.Art_->setPixmap (px);
	}

	void NowPlayingWidget::SetTrackInfo (const MediaInfo& info)
	{
		const bool isNull = info.Title_.isEmpty () && info.Artist_.isEmpty ();
		Ui_.TrackInfoLayout_->setEnabled (!isNull);
		Ui_.SimilarView_->setVisible (!isNull);

		const QString& unknown = isNull ?
				QString () :
				tr ("unknown");
		auto str = [&unknown] (const QString& str)
		{
			return str.isNull () ?
					unknown :
					("<strong>") + str + ("</strong>");
		};
		Ui_.ArtistName_->setText (str (info.Artist_));
		Ui_.AlbumName_->setText (str (info.Album_));
		Ui_.TrackName_->setText (str (info.Title_));

		const auto& genres = info.Genres_.join (" / ");
		Ui_.Genres_->setText ("<em>" + genres + "</em>");

		SetStatistics (info.LocalPath_);
	}

	namespace
	{
		QString FormatDateTime (const QDateTime& datetime)
		{
			const QDateTime& current = QDateTime::currentDateTime ();
			const int days = datetime.daysTo (current);
			if (days > 30)
				return datetime.toString ("MMMM yyyy");
			else if (days >= 7)
				return NowPlayingWidget::tr ("%n day(s) ago", 0, days);
			else if (days >= 1)
				return datetime.toString ("dddd");
			else
				return datetime.time ().toString ();
		}
	}

	void NowPlayingWidget::SetStatistics (const QString& path)
	{
		auto stats = Core::Instance ().GetLocalCollection ()->GetTrackStats (path);
		const bool valid = stats.Added_.isValid ();
		Ui_.LastPlay_->setVisible (valid);
		Ui_.LabelLastPlay_->setVisible (valid);
		Ui_.StatsCount_->setVisible (valid);
		Ui_.LabelPlaybacks_->setVisible (valid);
		if (!valid)
			return;

		Ui_.LastPlay_->setText (FormatDateTime (stats.LastPlay_));
		Ui_.StatsCount_->setText (tr ("%n play(s) since %1", 0, stats.Playcount_)
					.arg (FormatDateTime (stats.Added_)));
	}
}
}