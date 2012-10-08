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

#include "formats.h"
#include <algorithm>
#include <QtDebug>
#include <QProcess>
#include "transcodingparams.h"

namespace LeechCraft
{
namespace LMP
{
	QString Format::GetFileExtension () const
	{
		return GetFormatID ();
	}

	QStringList Format::ToFFmpeg (const TranscodingParams& params) const
	{
		QStringList result;
		result << "-acodec" << GetCodecName ();
		StandardQualityAppend (result, params);
		return result;
	}

	void Format::StandardQualityAppend (QStringList& result, const TranscodingParams& params) const
	{
		const auto& num = GetBitrateLabels (params.BitrateType_).value (params.Quality_);
		switch (params.BitrateType_)
		{
		case Format::BitrateType::CBR:
			result << "-ab"
					<< (QString::number (num) + "k");
			break;
		case Format::BitrateType::VBR:
			result << "-aq"
					<< QString::number (num);
			break;
		}
	}

#if QT_VERSION >= 0x040800
	class OggFormat : public Format
	{
	public:
		QString GetFormatID () const
		{
			return "ogg";
		}

		QString GetFormatName () const
		{
			return "OGG Vorbis";
		}

		QString GetCodecName () const
		{
			return "libvorbis";
		}

		QList<BitrateType> GetSupportedBitrates() const
		{
			return { BitrateType::VBR, BitrateType::CBR };
		}

		QList<int> GetBitrateLabels (BitrateType type) const
		{
			switch (type)
			{
			case BitrateType::CBR:
				return { 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 500 };
			case BitrateType::VBR:
				return { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown bitrate type";
			return QList<int> ();
		}
	};

	class AACFormatBase : public Format
	{
	public:
		QString GetFileExtension () const
		{
			return "m4a";
		}

		QList<BitrateType> GetSupportedBitrates () const
		{
			return { BitrateType::VBR, BitrateType::CBR };
		}

		QList<int> GetBitrateLabels (BitrateType type) const
		{
			switch (type)
			{
			case BitrateType::CBR:
				return { 24, 56, 76, 92, 128, 144, 176, 180, 192, 200, 224 };
			case BitrateType::VBR:
			{
				QList<int> result;
				for (int i = 0; i <= 10; ++i)
					result << i * 25 + 5;
				return result;
			}
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown bitrate type";
			return QList<int> ();
		}

		QStringList ToFFmpeg (const TranscodingParams& params) const
		{
			QStringList result;
			AppendCodec (result);
			StandardQualityAppend (result, params);
			return result;
		}
	protected:
		virtual void AppendCodec (QStringList& result) const = 0;
	};

	class AACFormat : public AACFormatBase
	{
	public:
		QString GetFormatID () const
		{
			return "aac-free";
		}

		QString GetFormatName () const
		{
			return QObject::tr ("AAC (free)");
		}

		QString GetCodecName () const
		{
			return "aac";
		}
	protected:
		void AppendCodec (QStringList& result) const
		{
			result << "-acodec" << "aac" << "-strict" << "-2";
		}
	};

	class FAACFormat : public AACFormatBase
	{
	public:
		QString GetFormatID () const
		{
			return "aac-nonfree";
		}

		QString GetFormatName () const
		{
			return QObject::tr ("AAC (non-free libfaac implementation)");
		}

		QString GetCodecName () const
		{
			return "libfaac";
		}
	protected:
		void AppendCodec (QStringList& result) const
		{
			result << "-acodec" << "libfaac";
		}
	};

	class MP3Format : public Format
	{
	public:
		QString GetFormatID () const
		{
			return "mp3";
		}

		QString GetFormatName () const
		{
			return "MP3";
		}

		QString GetCodecName () const
		{
			return "libmp3lame";
		}

		QList<BitrateType> GetSupportedBitrates () const
		{
			return { BitrateType::CBR, BitrateType::VBR };
		}

		QList<int> GetBitrateLabels (BitrateType type) const
		{
			switch (type)
			{
			case BitrateType::CBR:
				return { 64, 96, 128, 144, 160, 192, 224, 256, 320 };
			case BitrateType::VBR:
				return { -9, -8, -7, -6, -5, -4, -3, -2, -1 };
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown bitrate type";
			return QList<int> ();
		}
	};

	class WMAFormat : public Format
	{
	public:
		QString GetFormatID () const
		{
			return "wma";
		}

		QString GetFormatName () const
		{
			return "Windows Media Audio";
		}

		QString GetCodecName () const
		{
			return "wmav2";
		}

		QList<BitrateType> GetSupportedBitrates () const
		{
			return { BitrateType::CBR };
		}

		QList<int> GetBitrateLabels (BitrateType type) const
		{
			if (type == BitrateType::CBR)
				return { 65, 75, 88, 106, 133, 180, 271, 545 };

			qWarning () << Q_FUNC_INFO
					<< "unknown bitrate type";
			return QList<int> ();
		}
	};
#endif

	QString Formats::S_FFmpegCodecs_;

	Formats::Formats ()
	{
#if QT_VERSION >= 0x040800
		if (S_FFmpegCodecs_.isEmpty ())
		{
			QProcess ffmpegProcess;
			ffmpegProcess.start ("ffmpeg", QStringList ("-codecs"));
			ffmpegProcess.waitForFinished (1000);
			S_FFmpegCodecs_ = ffmpegProcess.readAllStandardOutput ();
		}

		Formats_ << Format_ptr (new OggFormat);
		Formats_ << Format_ptr (new AACFormat);
		Formats_ << Format_ptr (new FAACFormat);
		Formats_ << Format_ptr (new MP3Format);
		Formats_ << Format_ptr (new WMAFormat);

		std::copy_if (Formats_.begin (), Formats_.end (), std::back_inserter (EnabledFormats_),
				[] (const Format_ptr format)
				{
					return S_FFmpegCodecs_.contains (QRegExp (".EA... " + format->GetCodecName ()));
				});
#endif
	}

	QList<Format_ptr> Formats::GetFormats () const
	{
		return EnabledFormats_;
	}

	Format_ptr Formats::GetFormat (const QString& id) const
	{
		const auto pos = std::find_if (EnabledFormats_.begin (), EnabledFormats_.end (),
				[&id] (const Format_ptr format) { return format->GetFormatID () == id; });
		return pos == EnabledFormats_.end () ?
				Format_ptr () :
				*pos;
	}
}
}
