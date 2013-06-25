/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Slava Barinov <rayslava@gmail.com>
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


#include "twitterpage.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QClipboard>
#include <QCoreApplication>
#include <qjson/parser.h>
#include <interfaces/core/icoreproxy.h>
#include <util/util.h>
#include "core.h"
#include "xmlsettingsmanager.h"

Q_DECLARE_METATYPE (QObject**);

namespace LeechCraft
{
namespace Woodpecker
{
	TwitterPage::TwitterPage (const TabClassInfo& tc, Plugin *plugin, 
							  const FeedMode mode,
							  const KQOAuthParameters& params)
	: TC_ (tc)
	, Toolbar_ (new QToolBar (this))
	, EntityManager_ (Core::Instance ().GetCoreProxy ()->GetEntityManager ())
	, PageDefaultParam_ (params)
	, PageMode_ (mode)
	, ParentPlugin_ (plugin)
	{
		Ui_.setupUi (this);
		Delegate_ = new TwitDelegate (Ui_.TwitList_, ParentPlugin_);
		Ui_.TwitList_->setItemDelegate (Delegate_);

		//	Toolbar_->addAction (ui->actionRefresh);
		Interface_ = new TwitterInterface (this);
		connect (Interface_,
				SIGNAL (tweetsReady (QList<Tweet_ptr>)),
				this,
				SLOT (updateScreenTwits (QList<Tweet_ptr>)));
		TwitterTimer_ = new QTimer (this);
		TwitterTimer_->setInterval (XmlSettingsManager::Instance ()->property ("timer").toInt () * 1000); // Update twits every 1.5 minutes by default
		connect (TwitterTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (requestUpdate ()));
		tryToLogin ();

		connect (Ui_.TwitEdit_,
				SIGNAL (returnPressed ()),
				Ui_.TwitButton_,
				SLOT (click ( )));

		connect (Ui_.TwitList_->verticalScrollBar (),
				SIGNAL (valueChanged (int)),
				this,
				SLOT (scrolledDown (int)));

		connect (Ui_.TwitButton_,
				SIGNAL (clicked ()),
				this,
				SLOT (twit ()));
		Settings_ = new QSettings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Woodpecker");

		ActionRetwit_ = new QAction (tr ("Retwit"), Ui_.TwitList_);
		ActionRetwit_->setShortcut (Qt::Key_R + Qt::ALT);
		ActionRetwit_->setProperty ("ActionIcon", "edit-redo");
		connect (ActionRetwit_,
				SIGNAL (triggered ()),
				this,
				SLOT (retwit ()));

		ActionReply_ = new QAction (tr ("Reply"), Ui_.TwitList_);
		ActionReply_->setShortcut (Qt::Key_A + Qt::ALT);
		ActionReply_->setProperty ("ActionIcon", "mail-reply-sender");
		connect (ActionReply_,
				SIGNAL (triggered ()),
				this,
				SLOT (reply ()));

		ActionSPAM_ = new QAction (tr ("Report SPAM"), Ui_.TwitList_);
		ActionSPAM_->setProperty ("ActionIcon", "dialog-close");
		connect (ActionSPAM_,
				SIGNAL (triggered ()),
				this,
				SLOT (reportSpam ()));

		ActionOpenWeb_ = new QAction (tr ("Open twit in web interface"), Ui_.TwitList_);
		ActionOpenWeb_->setProperty ("ActionIcon", "webarchiver");
		connect (ActionOpenWeb_,
				SIGNAL (triggered ()),
				this,
				SLOT (webOpen ()));

		ActionCopyText_ = new QAction (tr ("Copy text to clipboard"), Ui_.TwitList_);
		ActionCopyText_->setProperty ("ActionIcon", "edit-copy");
		connect (ActionCopyText_,
				SIGNAL (triggered ()),
				this,
				SLOT (copyTwitText ()));

		ActionDelete_ = new QAction (tr ("Delete twit"), Ui_.TwitList_);
		ActionDelete_->setProperty ("ActionIcon", "edit-delete");
		connect (ActionDelete_,
				SIGNAL (triggered ()),
				this,
				SLOT (deleteTwit ()));

		connect (Ui_.TwitList_,
				SIGNAL (itemDoubleClicked (QListWidgetItem*)),
				this,
				SLOT (reply ()));

		Ui_.TwitList_->addActions ({ ActionRetwit_, ActionReply_ });

		if ((!Settings_->value ("token").isNull ()) && (!Settings_->value ("tokenSecret").isNull ()))
		{
			qDebug () << "Have an authorized" << Settings_->value ("token") << ":" << Settings_->value ("tokenSecret");
			Interface_->Login (Settings_->value ("token").toString (), Settings_->value ("tokenSecret").toString ());
			requestUpdate ();
			TwitterTimer_->start ();
		}

		UpdateReady_ = false;
		UiUpdateTimer_ = new QTimer (this);
		UiUpdateTimer_->setSingleShot (false);
		UiUpdateTimer_->setInterval (1000);		// Should not update more frequently than once a second
		connect (UiUpdateTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (updateTweetList ()));
		UiUpdateTimer_->start ();
	}

	TwitterPage::~TwitterPage ()
	{
		Settings_->deleteLater ();
		TwitterTimer_->stop ();
		UiUpdateTimer_->stop ();
	}

	TabClassInfo TwitterPage::GetTabClassInfo () const
	{
		return TC_;
	}

	QToolBar* TwitterPage::GetToolBar () const
	{
		return Toolbar_;
	}

	QObject* TwitterPage::ParentMultiTabs ()
	{
		return ParentPlugin_;
	}

	QList<QAction*> TwitterPage::GetTabBarContextMenuActions () const
	{
		return QList<QAction*> ();
	}

	QMap<QString, QList<QAction*>> TwitterPage::GetWindowMenus () const
	{
		return WindowMenus_;
	}

	void TwitterPage::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	void TwitterPage::tryToLogin ()
	{
		Interface_->GetAccess ();
		connect (Interface_,
				SIGNAL (authorized (QString, QString)),
				this,
				SLOT (recvdAuth (QString, QString)));
	}

	void TwitterPage::updateScreenTwits (QList<Tweet_ptr> twits)
	{
		if (twits.isEmpty ())	// if we have no tweets to parse
			return;

		Tweet_ptr firstNewTwit = twits.first ();

		if (ScreenTwits_.length () && (twits.last ()->GetId () == ScreenTwits_.first ()->GetId ())) // if we should prepend
			for (auto i = twits.end () - 2; i >= twits.begin (); i--)
				ScreenTwits_.insert (0, *i);
		else
		{
			int i;
			// Now we'd find firstNewTwit in twitList
			for (i = 0; i < ScreenTwits_.length (); i++)
				if ((ScreenTwits_.at (i)->GetId ()) == firstNewTwit->GetId ())
					break;

			int insertionShift = ScreenTwits_.length () - i;    // We've already got insertionShift twits to our list

			for (i = 0; i < insertionShift; i++)
				twits.removeFirst ();

			if (XmlSettingsManager::Instance ()->property ("notify").toBool ())
			{
				if (twits.length () == 1)			// We can notify the only twit
				{
					const auto& notification = Util::MakeNotification (twits.first ()->GetAuthor ()->GetUsername (),
							twits.first ()->GetText (),
							PInfo_);
					EntityManager_->HandleEntity (notification);
				}
				else if (!twits.isEmpty ()) {
					const auto& notification = Util::MakeNotification (tr ("Woodpecker"),
							tr ( "%1 new twit (s)" ).arg (twits.length ()),
							PInfo_);
					EntityManager_->HandleEntity (notification);
				}
			}
			ScreenTwits_.append (twits);
		}

		UpdateReady_ = true;
	}

	void TwitterPage::updateTweetList ()
	{
		if (!UpdateReady_)
			return;

		Ui_.TwitList_->setEnabled (false);
		Ui_.TwitList_->clear ();

		for (const auto& twit : ScreenTwits_)
		{
			QListWidgetItem *tmpitem = new QListWidgetItem ();

			tmpitem->setData (Qt::DisplayRole, "Title");
			tmpitem->setData (Qt::UserRole, QVariant::fromValue(twit));

			if (twit->GetAuthor ()->Avatar.isNull ())
				tmpitem->setData (Qt::DecorationRole, QIcon ("lcicons:/plugins/woodpecker/resources/images/woodpecker.svg"));
			else
				tmpitem->setData (Qt::DecorationRole, twit->GetAuthor ()->Avatar);
			Ui_.TwitList_->insertItem (0, tmpitem);
			Ui_.TwitList_->updateGeometry ();
		}

		Ui_.TwitList_->update ();
		Ui_.TwitList_->installEventFilter (this);
		Ui_.TwitList_->setEnabled (true);
		UpdateReady_ = false;
	}

	void TwitterPage::recvdAuth (const QString& token, const QString& tokenSecret)
	{
		Settings_->setValue ("token", token);
		Settings_->setValue ("tokenSecret", tokenSecret);
		requestUpdate ();
		TwitterTimer_->start ();
	}

	void TwitterPage::twit ()
	{
		Interface_->SendTweet (Ui_.TwitEdit_->text ());
		Ui_.TwitEdit_->clear ();
	}

	void TwitterPage::retwit ()
	{
		const auto& idx = Ui_.TwitList_->currentItem ();
		const auto twitid = (idx->data (Qt::UserRole).value<Tweet_ptr> ())->GetId ();
		Interface_->Retweet (twitid);
	}

	void TwitterPage::sendReply ()
	{
		const auto& idx = Ui_.TwitList_->currentItem ();
		const auto twitid = (idx->data (Qt::UserRole).value<Tweet_ptr> ())->GetId ();
		Interface_->Reply (twitid, Ui_.TwitEdit_->text ());
		Ui_.TwitEdit_->clear ();
		disconnect (Ui_.TwitButton_,
					SIGNAL (clicked ()),
					0,
					0);
		connect (Ui_.TwitButton_,
				SIGNAL (clicked ()),
				this,
				SLOT (twit ()));
	}


	void TwitterPage::reply (QListWidgetItem *index)
	{
		QListWidgetItem *idx = index;
		if (!index)
			idx = Ui_.TwitList_->currentItem ();

		const auto twitid = (idx->data (Qt::UserRole).value<Tweet_ptr> ())->GetId ();
		auto replyto = std::find_if (ScreenTwits_.begin (), ScreenTwits_.end (),
				[twitid] (decltype (ScreenTwits_.front ()) tweet)
					{ return tweet->GetId () == twitid; });
		if (replyto == ScreenTwits_.end ())
		{
			qWarning () << Q_FUNC_INFO << "Failed to find twit";
			return;
		}

		Tweet_ptr found_twit = *replyto;
		Ui_.TwitEdit_->setText (QString ("@") +
								((*replyto)->GetAuthor ()->GetUsername ()) +
								" ");
		disconnect (Ui_.TwitButton_,
					SIGNAL (clicked ()),
					0,
					0);
		connect (Ui_.TwitButton_,
				SIGNAL (clicked ()),
				this,
				SLOT (sendReply ()));
		Ui_.TwitEdit_->setFocus ();
	}

	void TwitterPage::scrolledDown (int sliderPos)
	{
		if (sliderPos == Ui_.TwitList_->verticalScrollBar ()->maximum ())
		{
			Ui_.TwitList_->verticalScrollBar ()->setSliderPosition (Ui_.TwitList_->verticalScrollBar ()->maximum () - 1);
			Ui_.TwitList_->setEnabled (false);
			if (!ScreenTwits_.empty ())
			{
				KQOAuthParameters param (PageDefaultParam_);
				param.insert ("max_id", QString::number ((*ScreenTwits_.begin ())->GetId ()));
				param.insert ("count", QString::number (XmlSettingsManager::Instance ()->property ("additional_twits").toUInt ()));
				
				Interface_->request (param, PageMode_);
			}
		}
	}

	void TwitterPage::on_TwitList__customContextMenuRequested (const QPoint& pos)
	{
		const auto& idx = Ui_.TwitList_->indexAt (pos);
		if (!idx.isValid ())
			return;
		
		const auto username = idx.data (Qt::UserRole).value<Tweet_ptr> ()->GetAuthor ()->GetUsername ();
		
		auto menu = new QMenu (Ui_.TwitList_);
		auto actionOpenTimeline = new QAction (tr ("Open @%1 timeline").arg (username), menu);
		actionOpenTimeline->setProperty ("ActionIcon", "document-open-folder");
		connect (actionOpenTimeline,
				SIGNAL (triggered ()),
				this,
				SLOT (openUserTimeline ()));

		menu->addActions ({ ActionRetwit_, ActionReply_, menu->addSeparator (),
			ActionSPAM_, menu->addSeparator (), ActionDelete_, menu->addSeparator (), ActionCopyText_, 
						  ActionOpenWeb_, actionOpenTimeline});
		menu->setAttribute (Qt::WA_DeleteOnClose);

		menu->exec (Ui_.TwitList_->viewport ()->mapToGlobal (pos));
	}

	void TwitterPage::reportSpam ()
	{
		const auto& idx = Ui_.TwitList_->currentItem ();
		const auto& twitid = (idx->data (Qt::UserRole).value<Tweet_ptr> ())->GetId ();

		auto spamTwit = std::find_if (ScreenTwits_.begin (), ScreenTwits_.end (),
				[twitid] (decltype (ScreenTwits_.front ()) tweet)
					{ return tweet->GetId () == twitid; });
		Interface_->ReportSPAM ((*spamTwit)->GetAuthor ()->GetUsername ());
	}

	void TwitterPage::webOpen ()
	{
		const auto& idx = Ui_.TwitList_->currentItem ();
		const auto& twitid = idx->data (Qt::UserRole).value<Tweet_ptr> ()->GetId ();
		auto currentTwit = std::find_if (ScreenTwits_.begin (), ScreenTwits_.end (),
				[twitid] (decltype (ScreenTwits_.front ()) tweet)
					{ return tweet->GetId () == twitid; });

		const auto& url = Util::MakeEntity (QUrl (QString ("https://twitter.com/%1/status/%2").arg ((*currentTwit)->GetAuthor ()->GetUsername ()).arg (twitid)),
				QString (), OnlyHandle | FromUserInitiated, QString ());
		EntityManager_->HandleEntity (url);
	}

	void TwitterPage::setUpdateReady ()
	{
		UpdateReady_ = true;
	}
	
	QByteArray TwitterPage::GetTabRecoverData () const
	{
		QByteArray result;
		QDataStream stream (&result, QIODevice::WriteOnly);

		stream << TC_.TabClass_ << PageDefaultParam_;

		return result;
	}
	
	QIcon TwitterPage::GetTabRecoverIcon () const
	{
		return GetTabClassInfo ().Icon_;
	}
	
	QString TwitterPage::GetTabRecoverName () const
	{
		return GetTabClassInfo ().VisibleName_;
	}
	
	void TwitterPage::requestUpdate ()
	{
		Interface_->request (PageDefaultParam_, PageMode_);
	}
	
	void TwitterPage::openUserTimeline ()
	{
		const auto& idx = Ui_.TwitList_->currentItem ();
		const auto& username = idx->data (Qt::UserRole).value<Tweet_ptr> ()->GetAuthor ()->GetUsername ();
		KQOAuthParameters param;
		param.insert ("screen_name", username.toUtf8 ().constData ());
		ParentPlugin_->AddTab (QString ("User/%1").arg (username),
								tr ("User tab"), tr ("Own timeline"), 
								FeedMode::UserTimeline,
								param);
	}
	
	void TwitterPage::copyTwitText ()
	{
		const auto& idx = Ui_.TwitList_->currentItem ();
		const auto& text = idx->data (Qt::UserRole).value<Tweet_ptr> ()->GetText ();
		
		QApplication::clipboard ()->setText (text, QClipboard::Clipboard);
	}
	
	void TwitterPage::deleteTwit ()
	{
		const auto& idx = Ui_.TwitList_->currentItem ();
		const auto& twitid = (idx->data (Qt::UserRole).value<Tweet_ptr> ())->GetId ();
		
		Interface_->Delete (twitid);
	}
	
}
}

