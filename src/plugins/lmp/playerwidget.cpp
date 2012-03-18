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

#include "playerwidget.h"
#include <QToolBar>
#include <QStylePainter>
#include <QBitmap>
#include <QTimer>
#include <QDialog>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include "phonon.h"
#include "videosettings.h"
#include "xmlsettingsmanager.h"
#include "core.h"

using namespace Phonon;

namespace LeechCraft
{
namespace LMP
{
	PlayerWidget::PlayerWidget (QWidget *parent)
	: QWidget (parent)
	, MediaObject_ (new MediaObject)
	{
		setObjectName ("LeechCraft::Plugins::LMP::PlayerWidget");
		MediaObject_->setTickInterval (700);
		connect (MediaObject_.get (),
				SIGNAL (tick (qint64)),
				this,
				SLOT (updateState ()));
		connect (MediaObject_.get (),
				SIGNAL (tick (qint64)),
				this,
				SLOT (updateOSD ()));
		connect (MediaObject_.get (),
				SIGNAL (stateChanged (Phonon::State,
						Phonon::State)),
				this,
				SLOT (updateState ()));
		connect (MediaObject_.get (),
				SIGNAL (hasVideoChanged (bool)),
				this,
				SLOT (handleHasVideoChanged (bool)));
		Ui_.setupUi (this);
		Ui_.VideoWidget_->installEventFilter (this);
		Ui_.ControlsLayout_->insertWidget (0, SetupToolbar ());

		Ui_.SeekSlider_->setMediaObject (MediaObject_.get ());

		AudioOutput_.reset (new AudioOutput (MusicCategory, this));

		connect (MediaObject_.get (),
				SIGNAL (finished ()),
				Ui_.VideoWidget_,
				SLOT (exitFullScreen ()));

		SetupContextMenu ();

		QBitmap bm (32, 32);
		bm.clear ();
		TransparentCursor_ = QCursor (bm, bm);
	}

	void PlayerWidget::Play ()
	{
		play ();
	}

	void PlayerWidget::Pause ()
	{
		pause ();
	}

	void PlayerWidget::Stop ()
	{
		MediaObject_->stop ();
	}

	void PlayerWidget::Clear ()
	{
		MediaObject_->clear ();
	}

	void PlayerWidget::Enqueue (const QUrl& url)
	{
		Enqueue (MediaSource (url));
	}

	void PlayerWidget::Enqueue (QIODevice *data)
	{
		Enqueue (MediaSource (data));
	}

	QWidget* PlayerWidget::Widget ()
	{
		return this;
	}

	void PlayerWidget::Enqueue (const MediaSource& source)
	{
		bool shouldStop = GetState () == Phonon::StoppedState;
		MediaObject_->enqueue (source);
		if (shouldStop)
			MediaObject_->stop ();
		Ui_.VideoWidget_->setVisible (MediaObject_->hasVideo ());
	}

	void PlayerWidget::Forward (SkipAmount a)
	{
		if (MediaObject_.get ())
			MediaObject_->seek (MediaObject_->currentTime () + a * 1000);

		updateState ();
	}

	void PlayerWidget::Rewind (SkipAmount a)
	{
		if (MediaObject_.get ())
			MediaObject_->seek (MediaObject_->currentTime () - a * 1000);

		updateState ();
	}

	State PlayerWidget::GetState () const
	{
		return MediaObject_->state ();
	}

	MediaObject* PlayerWidget::GetMediaObject () const
	{
		return MediaObject_.get ();
	}

	void PlayerWidget::play ()
	{
		if (MediaObject_.get ())
		{
			if (!VideoPath_.isValid ())
				VideoPath_.reconnect (MediaObject_.get (), Ui_.VideoWidget_);
			if (!AudioPath_.isValid ())
			{
				AudioPath_.reconnect (MediaObject_.get (), AudioOutput_.get ());
				Ui_.VolumeSlider_->setAudioOutput (AudioOutput_.get ());
			}

			MediaObject_->play ();
		}
	}

	void PlayerWidget::pause ()
	{
		if (MediaObject_.get ())
			MediaObject_->pause ();
	}

	void PlayerWidget::toggleFullScreen ()
	{
		if (!Ui_.VideoWidget_->isFullScreen ())
			QTimer::singleShot (1500,
					this,
					SLOT (hideCursor ()));
		Ui_.VideoWidget_->setFullScreen (!Ui_.VideoWidget_->isFullScreen ());
	}

	void PlayerWidget::togglePause ()
	{
		if (MediaObject_->state () == PausedState)
			Play ();
		else
			Pause ();
	}

	void PlayerWidget::incrementVolume ()
	{
		qreal nv = AudioOutput_->volume ();
		nv += 0.1;
		if (nv > 1)
			nv = 1;
		AudioOutput_->setVolume (nv);
	}

	void PlayerWidget::decrementVolume ()
	{
		qreal nv = AudioOutput_->volume ();
		nv -= 0.1;
		if (nv < 0)
			nv = 0;
		AudioOutput_->setVolume (nv);
	}

	QToolBar* PlayerWidget::SetupToolbar ()
	{
		QToolBar *bar = new QToolBar (this);

		Play_ = new QAction (tr ("Play"),
				this);
		Play_->setObjectName ("Play_");
		Play_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("lmp_play"));
		connect (Play_,
				SIGNAL (triggered ()),
				this,
				SLOT (play ()));

		Pause_ = new QAction (tr ("Pause"),
				this);
		Pause_->setObjectName ("Pause_");
		Pause_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("lmp_pause"));
		connect (Pause_,
				SIGNAL (triggered ()),
				this,
				SLOT (pause ()));

		ViewerSettings_ = new QAction (tr ("Viewer settings"),
				this);
		ViewerSettings_->setObjectName ("ViewerSettings_");
		ViewerSettings_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("lmp_viewersettings"));
		connect (ViewerSettings_,
				SIGNAL (triggered ()),
				this,
				SLOT (changeViewerSettings ()));

		ApplyVideoSettings (XmlSettingsManager::Instance ()->
					Property ("Brightness", 0).value<qreal> (),
				XmlSettingsManager::Instance ()->Property ("Contrast", 0).value<qreal> (),
				XmlSettingsManager::Instance ()->Property ("Hue", 0).value<qreal> (),
				XmlSettingsManager::Instance ()->Property ("Saturation", 0).value <qreal> ());

		bar->addAction (Play_);
		bar->addAction (Pause_);
		bar->addSeparator ();
		bar->addAction (ViewerSettings_);

		return bar;
	}

	void PlayerWidget::SetupContextMenu ()
	{
		FullScreen_ = new QAction (tr ("Toggle full screen"),
				this);
		FullScreen_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("lmp_togglefullscreen"));
		QList<QKeySequence> fullscreens;
		fullscreens << tr ("F");
		FullScreen_->setShortcuts (fullscreens);
		connect (FullScreen_,
				SIGNAL (triggered ()),
				this,
				SLOT (toggleFullScreen ()));

		TogglePause_ = new QAction (tr ("Pause/resume"),
				this);
		TogglePause_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("lmp_togglepause"));
		QList<QKeySequence> togglePauses;
		togglePauses << Qt::Key_Space
			<< tr ("P")
			<< tr ("R");
		TogglePause_->setShortcuts (togglePauses);
		connect (TogglePause_,
				SIGNAL (triggered ()),
				this,
				SLOT (togglePause ()));

		VolumeUp_ = new QAction (tr ("Volume up"),
				this);
		VolumeUp_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("lmp_volumeup"));
		QList<QKeySequence> volumeUps;
		volumeUps << QString ("=")
			<< QString ("+")
			<< QString ("*");
		VolumeUp_->setShortcuts (volumeUps);
		connect (VolumeUp_,
				SIGNAL (triggered ()),
				this,
				SLOT (incrementVolume ()));

		VolumeDown_ = new QAction (tr ("Volume down"),
				this);
		VolumeDown_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("lmp_volumedown"));
		QList<QKeySequence> volumeDowns;
		volumeDowns << QString ("-")
			<< QString ("/");;
		VolumeDown_->setShortcuts (volumeDowns);
		connect (VolumeDown_,
				SIGNAL (triggered ()),
				this,
				SLOT (decrementVolume ()));

		OSD_ = new QAction (tr ("On-screen display"),
				this);
		OSD_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("lmp_osd"));
		QList<QKeySequence> osds;
		osds << QString ("o");
		OSD_->setShortcuts (osds);
		OSD_->setCheckable (true);
		connect (OSD_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (updateOSD ()));

		Ui_.VideoWidget_->addAction (FullScreen_);
		Ui_.VideoWidget_->addAction (TogglePause_);
		Ui_.VideoWidget_->addAction (VolumeUp_);
		Ui_.VideoWidget_->addAction (VolumeDown_);
		Ui_.VideoWidget_->addAction (OSD_);
	}

	void PlayerWidget::ApplyVideoSettings (qreal b, qreal c, qreal h, qreal s)
	{
		Ui_.VideoWidget_->setBrightness (b);
		Ui_.VideoWidget_->setContrast (c);
		Ui_.VideoWidget_->setHue (h);
		Ui_.VideoWidget_->setSaturation (s);
	}

	bool PlayerWidget::eventFilter (QObject *o, QEvent *e)
	{
		if (OSD_->isChecked () &&
				e->type () == QEvent::Paint &&
				o == Ui_.VideoWidget_)
		{
			/*
			QPainter p (Ui_.VideoWidget_);
			p.setPen (QColor (Qt::white));
			p.drawText (200, 200, "Test");
			*/
		}
		else if (e->type () == QEvent::MouseMove &&
				o == Ui_.VideoWidget_)
		{
			if (Ui_.VideoWidget_->isFullScreen ())
			{
				Ui_.VideoWidget_->setCursor (QCursor ());
				QTimer::singleShot (1500,
						this,
						SLOT (hideCursor ()));
			}
		}
		return QObject::eventFilter (o, e);
	}

	void PlayerWidget::handleHasVideoChanged (bool has)
	{
		Ui_.VideoWidget_->setVisible (has);
		updateOSD ();
	}

	void PlayerWidget::updateOSD ()
	{
		if (!MediaObject_->hasVideo () &&
				!OSD_->isChecked ())
			return;

		Ui_.VideoWidget_->update ();
	}

	void PlayerWidget::updateState ()
	{
		QString result;
		switch (MediaObject_->state ())
		{
			case LoadingState:
				result = tr ("Initializing");
				break;
			case StoppedState:
				result = tr ("Stopped");
				break;
			case PlayingState:
				result = tr ("Playing");
				break;
			case BufferingState:
				result = tr ("Buffering");
				break;
			case PausedState:
				result = tr ("Paused");
				break;
			case ErrorState:
				result = tr ("Error");
				break;
		}
		if (MediaObject_->state () == ErrorState)
			result += QString (" (%1)").arg (MediaObject_->errorString ());
		result += tr (" [");
		result += QString::number (static_cast<double> (MediaObject_->
					currentTime ())/1000., 'f', 1);
		int totalTime = MediaObject_->totalTime ();
		if (totalTime)
		{
			result += tr ("/");
			result += QString::number (static_cast<double> (totalTime)/1000.,
					'f', 1);
		}
		result += tr ("]");

		result += tr (" from ");
		MediaSource source = MediaObject_->currentSource ();
		switch (source.type ())
		{
			case MediaSource::Invalid:
#if PHONON_VERSION >= PHONON_VERSION_CHECK (4, 3, 0)
			case MediaSource::Empty:
#endif
				result += tr ("nowhere");
				break;
			case MediaSource::LocalFile:
				result += source.fileName ();
				break;
			case MediaSource::Url:
				result += source.url ().toString ();
				break;
			case MediaSource::Disc:
				result += source.deviceName ();
				switch (source.discType ())
				{
					case Cd:
						result += tr (" (CD)");
						break;
					case Dvd:
						result += tr (" (DVD)");
						break;
					case Vcd:
						result += tr (" (VCD)");
						break;
					default:
						result += tr (" (Unknown disc type)");
						break;
				}
				break;
			case MediaSource::Stream:
				result += tr ("stream");
				break;
		}

		if (MediaObject_->state () == ErrorState)
			emit gotEntity (Util::MakeNotification ("LMP", result, PCritical_));
		else
			emit stateUpdated (result);
	}

	void PlayerWidget::changeViewerSettings ()
	{
		std::auto_ptr<VideoSettings> settings (new VideoSettings (Ui_.VideoWidget_->brightness (),
				Ui_.VideoWidget_->contrast (),
				Ui_.VideoWidget_->hue (),
				Ui_.VideoWidget_->saturation (),
				this));
		if (settings->exec () == QDialog::Rejected)
			return;

		qreal b = settings->Brightness (),
				c = settings->Contrast (),
				h = settings->Hue (),
				s = settings->Saturation ();

		ApplyVideoSettings (b, c, h, s);

		XmlSettingsManager::Instance ()->setProperty ("Brightness", b);
		XmlSettingsManager::Instance ()->setProperty ("Contrast", c);
		XmlSettingsManager::Instance ()->setProperty ("Hue", h);
		XmlSettingsManager::Instance ()->setProperty ("Saturation", s);
	}

	void PlayerWidget::handleStateUpdated (const QString& state)
	{
		setToolTip (state);
	}

	void PlayerWidget::hideCursor ()
	{
		Ui_.VideoWidget_->setCursor (TransparentCursor_);
	}
}
}
