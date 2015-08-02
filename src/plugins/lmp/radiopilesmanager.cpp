/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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

#include "radiopilesmanager.h"
#include <QStandardItemModel>
#include <QInputDialog>
#include <QtDebug>
#include <util/sll/slotclosure.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/iaudiopile.h>
#include <interfaces/media/iradiostationprovider.h>
#include "previewcharacteristicinfo.h"
#include "mediainfo.h"
#include "util.h"

namespace LeechCraft
{
namespace LMP
{
	RadioPilesManager::RadioPilesManager (const IPluginsManager *pm, QObject *parent)
	: QObject { parent }
	, PilesModel_ { new QStandardItemModel { this } }
	{
		FillModel (pm);
	}

	QAbstractItemModel* RadioPilesManager::GetModel () const
	{
		return PilesModel_;
	}

	void RadioPilesManager::FillModel (const IPluginsManager *pm)
	{
		for (auto pileObj : pm->GetAllCastableRoots<Media::IAudioPile*> ())
		{
			auto pile = qobject_cast<Media::IAudioPile*> (pileObj);

			auto item = new QStandardItem (tr ("Search in %1")
					.arg (pile->GetServiceName ()));
			item->setIcon (pile->GetServiceIcon ());
			item->setEditable (false);

			const auto function = [item, pile, this] { HandlePile (item, pile); };
			item->setData (QVariant::fromValue<Media::ActionFunctor_f> (function),
					Media::RadioItemRole::ActionFunctor);

			PilesModel_->appendRow (item);
		}
	}

	namespace
	{
		void AddResults (const QList<Media::IPendingAudioSearch::Result>& results, QStandardItem *item)
		{
			const auto& queryText = item->text ();
			item->setText (RadioPilesManager::tr ("%1 (%n result(s))", 0, results.size ())
						.arg (queryText));

			QSet<QUrl> urls;
			QSet<PreviewCharacteristicInfo> infos;
			for (const auto& res : results)
			{
				if (urls.contains (res.Source_))
					continue;
				urls.insert (res.Source_);

				const PreviewCharacteristicInfo checkInfo { res.Info_ };
				if (infos.contains (checkInfo))
					continue;
				infos << checkInfo;

				auto info = res.Info_;
				info.Other_ ["URL"] = res.Source_;

				const auto& name = PerformSubstitutionsPlaylist (MediaInfo::FromAudioInfo (info));
				const auto resItem = new QStandardItem { name };
				resItem->setEditable (false);
				resItem->setData ({}, Media::RadioItemRole::ItemType);
				resItem->setData (Media::RadioType::SingleTrack, Media::RadioItemRole::ItemType);
				resItem->setData (QVariant::fromValue<QList<Media::AudioInfo>> ({ info }),
						Media::RadioItemRole::TracksInfos);
				item->appendRow (resItem);
			}
		}
	}

	void RadioPilesManager::HandlePile (QStandardItem *item, Media::IAudioPile *pile)
	{
		const auto& query = QInputDialog::getText (nullptr,
				tr ("Audio search"),
				tr ("Enter the string to search for:"));
		if (query.isEmpty ())
			return;

		Media::AudioSearchRequest req;
		req.FreeForm_ = query;

		const auto searchItem = new QStandardItem { query };
		searchItem->setData (Media::RadioType::TracksList, Media::RadioItemRole::ItemType);
		searchItem->setEditable (false);
		item->appendRow (searchItem);

		const auto pending = pile->Search (req);
		const auto pendingObj = pending->GetQObject ();

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[]
			{
				qWarning () << Q_FUNC_INFO
						<< "error";
			},
			pendingObj,
			SIGNAL (error ()),
			pendingObj
		};

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[pending, searchItem] { AddResults (pending->GetResults (), searchItem); },
			pendingObj,
			SIGNAL (ready ()),
			pendingObj
		};
	}
}
}
