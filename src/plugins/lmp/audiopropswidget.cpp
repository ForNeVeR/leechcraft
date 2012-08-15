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

#include "audiopropswidget.h"
#include <functional>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <QDialog>
#include <QDialogButtonBox>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QClipboard>
#include <QMutexLocker>
#include <QFileInfo>
#include <QtDebug>
#include <taglib/taglib_config.h>
#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

#define HAS_APE TAGLIB_MAJOR_VERSION > 1 || TAGLIB_MINOR_VERSION > 6
#if HAS_APE
#include <taglib/apeproperties.h>
#endif

#ifdef TAGLIB_WITH_ASF
#include <taglib/asfproperties.h>
#endif

#include <taglib/flacproperties.h>

#ifdef TAGLIB_WITH_MP4
#include <taglib/mp4properties.h>
#endif

#include <taglib/mpcproperties.h>
#include <taglib/mpegproperties.h>
#include <taglib/speexproperties.h>
#include <taglib/vorbisproperties.h>
#include <taglib/aiffproperties.h>
#include <taglib/wavproperties.h>
#include <taglib/trueaudioproperties.h>
#include <taglib/wavpackproperties.h>
#include "localfileresolver.h"
#include "core.h"

namespace LeechCraft
{
namespace LMP
{
	AudioPropsWidget::AudioPropsWidget (QWidget *parent)
	: QWidget (parent)
	, PropsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.PropsView_->setModel (PropsModel_);

		auto copy = new QAction (tr ("Copy"), this);
		copy->setIcon (Core::Instance ().GetProxy ()->GetIcon ("edit-copy"));
		connect (copy,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCopy ()));
		Ui_.PropsView_->addAction (copy);
	}

	AudioPropsWidget* AudioPropsWidget::MakeDialog ()
	{
		auto dia = new QDialog ();
		dia->setWindowTitle (tr ("Track properties"));
		dia->resize (800, 600);
		dia->setLayout (new QVBoxLayout);

		auto props = new AudioPropsWidget;

		auto box = new QDialogButtonBox (QDialogButtonBox::Close);
		connect (box,
				SIGNAL (rejected ()),
				dia,
				SLOT (close ()));

		dia->layout ()->addWidget (props);
		dia->layout ()->addWidget (box);

		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();

		return props;
	}

	void AudioPropsWidget::SetProps (const QString& path)
	{
		try
		{
			const auto& info = Core::Instance ().GetLocalFileResolver ()->ResolveInfo (path);
			SetProps (info);
		}
		catch (const ResolveError& e)
		{
			qWarning () << Q_FUNC_INFO
					<< path
					<< e.what ();

			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Error showing properties for %1: %2.")
						.arg (QFileInfo (path).fileName ())
						.arg (e.what ()));
		}
	}

	namespace
	{
		QMap<QString, QString> GetGenericProps (TagLib::AudioProperties *props)
		{
			QMap<QString, QString> result;
			result [AudioPropsWidget::tr ("Bitrate")] = QString::number (props->bitrate ()) + " kbps";
			result [AudioPropsWidget::tr ("Channels")] = QString::number (props->channels ());
			result [AudioPropsWidget::tr ("Sample rate")] = QString::number (props->sampleRate ()) + " Hz";
			return result;
		}

		template<typename Appender>
		struct PropsGetter
		{
			Appender F_;
			std::function<void (QString, int)> IF_;
			TagLib::AudioProperties *Props_;

			PropsGetter (Appender app, TagLib::AudioProperties *props)
			: F_ (app)
			, Props_ (props)
			{
				IF_ = [app] (const QString& name, int val) { app (name, QString::number (val)); };
			}

			template<typename T>
			void operator() (T *t)
			{
				if ((t = dynamic_cast<T*> (Props_)))
					F_ (AudioPropsWidget::tr ("File type"), Parse (t));
			}
#if HAS_APE
			QString Parse (TagLib::APE::Properties *props)
			{
				IF_ ("APE version", props->version ());
				IF_ ("Bits per sample", props->bitsPerSample ());
				return "APE";
			}
#endif

#ifdef TAGLIB_WITH_ASF
			QString Parse (TagLib::ASF::Properties*)
			{
				return "ASF";
			}
#endif

			QString Parse (TagLib::FLAC::Properties *props)
			{
				IF_ ("Sample width", props->sampleWidth ());
				return "FLAC";
			}

#ifdef TAGLIB_WITH_MP4
			QString Parse (TagLib::MP4::Properties *props)
			{
				IF_ ("Bits per sample", props->bitsPerSample ());
				return "MP4";
			}
#endif

			QString Parse (TagLib::MPC::Properties *props)
			{
				IF_ ("MPC version", props->mpcVersion ());
				return "MPC";
			}

			QString Parse (TagLib::MPEG::Properties *props)
			{
				switch (props->version ())
				{
				case TagLib::MPEG::Header::Version1:
					IF_ ("MPEG version", 1);
					break;
				case TagLib::MPEG::Header::Version2:
					IF_ ("MPEG version", 2);
					break;
				case TagLib::MPEG::Header::Version2_5:
					F_ ("MPEG version", "2.5");
					break;
				}

				IF_ ("MPEG layer", props->layer ());
				F_ ("Protected", props->protectionEnabled () ? "true" : "false");
				F_ ("Copyrighted", props->isCopyrighted () ? "true" : "false");
				F_ ("Original", props->isOriginal () ? "true" : "false");

				switch (props->channelMode ())
				{
				case TagLib::MPEG::Header::Stereo:
					F_ ("Channel mode", "Stereo");
					break;
				case TagLib::MPEG::Header::JointStereo:
					F_ ("Channel mode", "Joint Stereo");
					break;
				case TagLib::MPEG::Header::DualChannel:
					F_ ("Channel mode", "Dual Mono");
					break;
				case TagLib::MPEG::Header::SingleChannel:
					F_ ("Channel mode", "Mono");
					break;
				}

				return "MPEG";
			}

			QString Parse (TagLib::Ogg::Speex::Properties *props)
			{
				IF_ ("Speex version", props->speexVersion ());
				return "Speex";
			}

			QString Parse (TagLib::Vorbis::Properties *props)
			{
				IF_ ("Vorbis version", props->vorbisVersion ());
				IF_ ("Minimum bitrate", props->bitrateMinimum ());
				IF_ ("Maximum bitrate", props->bitrateMaximum ());
				IF_ ("Nominal bitrate", props->bitrateNominal ());
				return "OGG Vorbis";
			}

			QString Parse (TagLib::RIFF::AIFF::Properties *props)
			{
#if TAGLIB_MAJOR_VERSION > 1 || TAGLIB_MINOR_VERSION > 6
				IF_ ("Sample width", props->sampleWidth ());
#endif
				return "AIFF";
			}

			QString Parse (TagLib::RIFF::WAV::Properties *props)
			{
#if TAGLIB_MAJOR_VERSION > 1 || TAGLIB_MINOR_VERSION > 6
				IF_ ("Sample width", props->sampleWidth ());
#endif
				return "WAV";
			}

			QString Parse (TagLib::TrueAudio::Properties *props)
			{
				IF_ ("Bits per sample", props->bitsPerSample ());
				IF_ ("TTA version", props->ttaVersion ());
				return "TTA (TrueAudio)";
			}

			QString Parse (TagLib::WavPack::Properties *props)
			{
				IF_ ("Bits per sample", props->bitsPerSample ());
				IF_ ("WavPack version", props->version ());
				return "WavPack";
			}
		};

		template<typename T>
		PropsGetter<T> MakeGetter (T f, TagLib::AudioProperties *props)
		{
			return PropsGetter<T> (f, props);
		}
	}

	void AudioPropsWidget::SetProps (const MediaInfo& info)
	{
		PropsModel_->clear ();

		auto append = [this] (const QString& name, const QString& val)
		{
			auto nameItem = new QStandardItem (name);
			nameItem->setEditable (false);
			auto valItem = new QStandardItem (val);
			valItem->setEditable (false);
			QList<QStandardItem*> items;
			items << nameItem << valItem;
			PropsModel_->appendRow (items);
		};

		append (tr ("Artist"), info.Artist_);
		append (tr ("Title"), info.Title_);
		append (tr ("Album"), info.Album_);
		append (tr ("Track number"), QString::number (info.TrackNumber_));
		append (tr ("Year"), QString::number (info.Year_));
		append (tr ("Genres"), info.Genres_.join ("; "));
		append (tr ("Length"), QString::number (info.Length_));
		append (tr ("Local path"), info.LocalPath_.isEmpty () ? tr ("unknown") : info.LocalPath_);

		if (info.LocalPath_.isEmpty ())
			return;

		QMutexLocker tlLocker (&Core::Instance ().GetLocalFileResolver ()->GetMutex ());

		auto r = Core::Instance ().GetLocalFileResolver ()->GetFileRef (info.LocalPath_);
		auto tag = r.tag ();
		if (!tag)
			return;

		append (tr ("Comment"), QString::fromUtf8 (tag->comment ().toCString (true)));

		auto props = r.audioProperties ();
		if (!props)
			return;

		auto addMap = [append] (const QMap<QString, QString>& map)
		{
			Q_FOREACH (const auto& key, map.keys ())
				append (key, map [key]);
		};

		addMap (GetGenericProps (props));

		boost::mpl::vector<
#if HAS_APE
				TagLib::APE::Properties*,
#endif
#ifdef TAGLIB_WITH_ASF
				TagLib::ASF::Properties*,
#endif
				TagLib::FLAC::Properties*,
#ifdef TAGLIB_WITH_MP4
				TagLib::MP4::Properties*,
#endif
				TagLib::MPC::Properties*,
				TagLib::MPEG::Properties*,
				TagLib::Ogg::Speex::Properties*,
				TagLib::Vorbis::Properties*,
				TagLib::RIFF::AIFF::Properties*,
				TagLib::RIFF::WAV::Properties*,
				TagLib::TrueAudio::Properties*,
				TagLib::WavPack::Properties*
				> propsTypes;
		boost::mpl::for_each (MakeGetter (append, props), &propsTypes);
	}

	void AudioPropsWidget::handleCopy ()
	{
		const auto& idx = Ui_.PropsView_->currentIndex ();
		if (!idx.isValid ())
			return;

		QString text = idx.sibling (idx.row (), 1).data ().toString ();
		if (!idx.column ())
		{
			text.prepend (": ");
			text.prepend (idx.data ().toString ());
		}

		qApp->clipboard ()->setText (text);
	}
}
}
