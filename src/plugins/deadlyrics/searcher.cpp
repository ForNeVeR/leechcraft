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

#include "searcher.h"
#include <stdexcept>
#include <QDataStream>

namespace LeechCraft
{
namespace DeadLyrics
{
	bool operator== (const Lyrics& l1, const Lyrics& l2)
	{
		return (l1.Author_ == l2.Author_ ||
				l1.Title_ == l2.Title_) &&
			l1.Text_ == l2.Text_;
	}

	QDataStream& operator<< (QDataStream& out, const Lyrics& lyrics)
	{
		quint8 version = 1;
		out << version
			<< lyrics.Author_
			<< lyrics.Album_
			<< lyrics.Title_
			<< lyrics.Text_
			<< lyrics.URL_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, Lyrics& lyrics)
	{
		quint8 version = 0;
		in >> version;
		if (version == 1)
		{
			in >> lyrics.Author_
				>> lyrics.Album_
				>> lyrics.Title_
				>> lyrics.Text_
				>> lyrics.URL_;
		}
		else
			throw std::runtime_error (qPrintable (QString ("Unknown %1 %2")
					.arg (version)
					.arg (Q_FUNC_INFO)));

		return in;
	}

	Searcher::~Searcher ()
	{
	}
}
}
