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

#ifndef PLUGINS_AZOTH_PLUGINS_CHATHISTORY_STORAGE_H
#define PLUGINS_AZOTH_PLUGINS_CHATHISTORY_STORAGE_H
#include <memory>
#include <QSqlQuery>
#include <QHash>
#include <QVariant>
#include <QDateTime>

class QSqlDatabase;

namespace LeechCraft
{
namespace Azoth
{
class IMessage;

namespace ChatHistory
{
	class Storage : public QObject
	{
		Q_OBJECT

		std::shared_ptr<QSqlDatabase> DB_;
		QSqlQuery UserSelector_;
		QSqlQuery AccountSelector_;
		QSqlQuery UserIDSelector_;
		QSqlQuery AccountIDSelector_;
		QSqlQuery UserInserter_;
		QSqlQuery AccountInserter_;
		QSqlQuery MessageDumper_;
		QSqlQuery UsersForAccountGetter_;
		QSqlQuery Date2Pos_;
		QSqlQuery GetMonthDates_;
		QSqlQuery LogsSearcher_;
		QSqlQuery LogsSearcherWOContact_;
		QSqlQuery LogsSearcherWOContactAccount_;
		QSqlQuery HistoryGetter_;
		QSqlQuery HistoryClearer_;
		QSqlQuery EntryCacheSetter_;
		QSqlQuery EntryCacheGetter_;
		QHash<QString, qint32> Users_;
		QHash<QString, qint32> Accounts_;

		struct RawSearchResult
		{
			qint32 EntryID_;
			qint32 AccountID_;
			QDateTime Date_;

			RawSearchResult ();
			RawSearchResult (qint32 entryId, qint32 accountId, const QDateTime& date);

			bool IsEmpty () const;
		};
	public:
		Storage (QObject* = 0);
	private:
		void InitializeTables ();

		QHash<QString, qint32> GetUsers ();
		qint32 GetUserID (const QString&);
		void AddUser (const QString& id);

		QHash<QString, qint32> GetAccounts ();
		qint32 GetAccountID (const QString&);
		void AddAccount (const QString& id);
		RawSearchResult Search (const QString& accountId, const QString& entryId,
				const QString& text, int shift);
		RawSearchResult Search (const QString& accountId, const QString& text, int shift);
		RawSearchResult Search (const QString& text, int shift);
		void SearchDate (qint32, qint32, const QDateTime&);
	public slots:
		void addMessage (const QVariantMap&);
		void getOurAccounts ();
		void getUsersForAccount (const QString&);
		void getChatLogs (const QString& accountId,
				const QString& entryId, int backpages, int amount);
		void search (const QString& accountId, const QString& entryId,
				const QString& text, int shift);
		void searchDate (const QString& accountId, const QString& entryId, const QDateTime& dt);
		void getDaysForSheet (const QString& accountId, const QString& entryId, int year, int month);
		void clearHistory (const QString& accountId, const QString& entryId);
	signals:
		void gotOurAccounts (const QStringList&);
		void gotUsersForAccount (const QStringList&, const QString&, const QStringList&);
		void gotChatLogs (const QString&, const QString&,
				int, int, const QVariant&);
		void gotSearchPosition (const QString&, const QString&, int);
		void gotDaysForSheet (const QString& accountId, const QString& entryId,
				int year, int month, const QList<int>& days);
	};
}
}
}

#endif
