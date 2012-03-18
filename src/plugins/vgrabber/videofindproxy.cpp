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

#include "videofindproxy.h"
#include <QAction>
#include <QTextCodec>
#include <util/util.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			VideoFindProxy::VideoFindProxy (const Request& r, CategoriesSelector *cs)
			: FindProxy (r, cs, FPTVideo)
			, Type_ (PTInvalid)
			{
				SetError (tr ("Searching for %1...")
						.arg (r.String_));
			}

			QVariant VideoFindProxy::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();

				switch (role)
				{
					case Qt::DisplayRole:
						if (Error_)
						{
							switch (index.column ())
							{
								case 0:
									return *Error_;
								case 1:
									return tr ("Video vkontakte.ru");
								default:
									return QString ();
							}
						}
						else
						{
							const VideoResult& res = VideoResults_ [index.row ()];
							switch (index.column ())
							{
								case 0:
									return res.Title_;
								case 1:
									return tr ("Video");
								case 2:
									return res.URL_.toString ();
								default:
									return QString ();
							}
						}
					case LeechCraft::RoleControls:
						{
							UpdateURLActionsData (index.row ());
							return QVariant::fromValue<QToolBar*> (Toolbar_);
						}
					case LeechCraft::RoleContextMenu:
						{
							UpdateURLActionsData (index.row ());
							return QVariant::fromValue<QMenu*> (ContextMenu_);
						}
					default:
						return QVariant ();
				}
			}

			void VideoFindProxy::UpdateURLActionsData (int row) const
			{
				QUrl url;
				if (VideoResults_.size () > row)
					url = VideoResults_ [row].URL_;
				if (!url.isEmpty ())
				{
					ActionDownload_->setData (url);
					ActionHandle_->setData (url);
					ActionCopyToClipboard_->setData (url);
				}
				ActionDownload_->setEnabled (!url.isEmpty ());
				ActionHandle_->setEnabled (!url.isEmpty ());
				ActionCopyToClipboard_->setEnabled (!url.isEmpty ());
			}

			int VideoFindProxy::rowCount (const QModelIndex& parent) const
			{
				if (parent.isValid ())
					return 0;

				if (Error_)
					return 1;
				else
					return VideoResults_.size ();
			}

			QUrl VideoFindProxy::GetURL () const
			{
				QByteArray urlStr = "http://vk.com/gsearch.php?q=FIND&section=video";
				urlStr.replace ("FIND",
						QTextCodec::codecForName ("Windows-1251")->fromUnicode (R_.String_).toPercentEncoding ());
				QUrl result = QUrl::fromEncoded (urlStr);
				return result;
			}

			void VideoFindProxy::Handle (const QString& contents)
			{
				if (Type_ == PTInvalid)
					HandleSearchResults (contents);
				else
					HandleVideoPage (contents);
			}

			void VideoFindProxy::HandleSearchResults (const QString& contents)
			{
				QRegExp upt ("<div class=\"aname\" style='width:255px; overflow: hidden'><a href=\"video(.*)\\?noiphone\">(.*)</a></div>",
						Qt::CaseSensitive,
						QRegExp::RegExp2);
				upt.setMinimal (true);
				int pos = 0;
				while ((pos = upt.indexIn (contents, pos)) >= 0)
				{
					QStringList captured = upt.capturedTexts ();
					QUrl url = QUrl (QString ("http://vk.com/video%1")
							.arg (captured.at (1)));
					QString title = captured.at (2);
					title.remove ("<span class=\"match\">").remove ("</span>");

					VideoResult vr =
					{
						url,
						title
					};

					VideoResults_ << vr;
					pos += upt.matchedLength ();
				}

				if (VideoResults_.size ())
				{
					SetError (QString ());

					beginInsertRows (QModelIndex (), 0, VideoResults_.size () - 1);
					endInsertRows ();
				}
				else
					SetError (tr ("Nothing found for %1")
							.arg (R_.String_));
			}

			namespace
			{
				QString GetStringFromRX (const QString& pattern, const QString& contents)
				{
					QString result;
					QRegExp rx (pattern,
							Qt::CaseSensitive,
							QRegExp::RegExp2);
					rx.setMinimal (true);
					if (rx.indexIn (contents) != -1)
						result = rx.capturedTexts ().at (1);
					else
						qWarning () << Q_FUNC_INFO
							<< "nothing captured for pattern"
							<< rx.pattern ();
					return result;
				}
			};

			void VideoFindProxy::HandleVideoPage (const QString& contents)
			{
				// http://cs12739.vkontakte.ru/u16199765/video/1684dec3e6.240.mp4
				// -----------$host-----------/--$user--/video/---$vtag--.240.mp4
				QString host = GetStringFromRX (".*\"host\":\"([0-9a-z/\\:\\.\\\\]*)\".*", contents);
				QString user = GetStringFromRX (".*\"uid\":\"([0-9]*)\".*", contents);
				QString vtag = GetStringFromRX (".*\"vtag\":\"([0-9a-f\\-]*)\".*", contents);
				// Not needed anymore, but let's have it here just in case.
				// QString vkid = GetStringFromRX (".*\"vkid\":\"([0-9a-f]*)\".*", contents);

				host.replace ("\\/", "/");
				if (host.endsWith ('/'))
					host.chop (1);

				if (host.isEmpty () ||
						vtag.isEmpty () ||
						user.isEmpty ())
				{
					qWarning () << Q_FUNC_INFO
						<< "one of required attrs is empty"
						<< host
						<< vtag
						<< user
						<< "for"
						<< contents;
					return;
				}

				QString source = QString ("%1/u%2/video/%3.240.mp4")
						.arg (host)
						.arg (user)
						.arg (vtag);

				qDebug () << Q_FUNC_INFO
						<< source;

				LeechCraft::TaskParameter hd = OnlyHandle;
				switch (Type_)
				{
					case PTInvalid:
						qWarning () << Q_FUNC_INFO
							<< "invalid Type_, assuming both Download and Handle";
						break;
					case PTHandle:
						hd = OnlyHandle;
						break;
					case PTDownload:
						hd = OnlyDownload;
						break;
				}

				EmitWith (hd, QUrl (source));
			}

			void VideoFindProxy::handleDownload ()
			{
				Type_ = PTDownload;
				HandleAction ();
			}

			void VideoFindProxy::handleHandle ()
			{
				Type_ = PTHandle;
				HandleAction ();
			}

			void VideoFindProxy::HandleAction ()
			{
				QUrl url = qobject_cast<QAction*> (sender ())->data ().value<QUrl> ();

				QString fname = Util::GetTemporaryName ();
				Entity e =
					Util::MakeEntity (url,
						fname,
						LeechCraft::Internal |
							LeechCraft::DoNotNotifyUser |
							LeechCraft::DoNotSaveInHistory |
							LeechCraft::NotPersistent |
							LeechCraft::DoNotAnnounceEntity);

				int id = -1;
				QObject *pr = 0;
				emit delegateEntity (e, &id, &pr);
				if (id == -1)
				{
					emit error (tr ("Job for request<br />%1<br />wasn't delegated.")
							.arg (url.toString ()));
					return;
				}

				Jobs_ [id] = fname;
				HandleProvider (pr);
			}
		};
	};
};
