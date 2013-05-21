/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "simplestorage.h"
#include <QSettings>
#include <QIcon>
#include <QCoreApplication>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SecMan
		{
			namespace StoragePlugins
			{
				namespace SimpleStorage
				{
					void Plugin::Init (ICoreProxy_ptr)
					{
						Storage_ .reset (new QSettings (QSettings::IniFormat,
									QSettings::UserScope,
									QCoreApplication::organizationName (),
									QCoreApplication::applicationName () + "_SecMan_SimpleStorage"));
					}

					void Plugin::SecondInit ()
					{
					}

					QByteArray Plugin::GetUniqueID () const
					{
						return "org.LeechCraft.SecMan.StoragePlugins.SimpleStorage";
					}

					void Plugin::Release ()
					{
					}

					QString Plugin::GetName () const
					{
						return "SimpleStorage";
					}

					QString Plugin::GetInfo () const
					{
						return tr ("Simple unencrypted storage plugin for SecMan");
					}

					QIcon Plugin::GetIcon () const
					{
						static QIcon icon ("lcicons:/secman/simplestorage/resources/images/simplestorage.svg");
						return icon;
					}

					QStringList Plugin::Provides () const
					{
						return QStringList ();
					}

					QStringList Plugin::Needs () const
					{
						return QStringList ();
					}

					QStringList Plugin::Uses () const
					{
						return QStringList ();
					}

					void Plugin::SetProvider (QObject*, const QString&)
					{
					}

					QSet<QByteArray> Plugin::GetPluginClasses () const
					{
						return QSet<QByteArray> () << "org.LeechCraft.SecMan.StoragePlugins/1.0";
					}

					IStoragePlugin::StorageTypes Plugin::GetStorageTypes () const
					{
						return STInsecure;
					}

					QList<QByteArray> Plugin::ListKeys (IStoragePlugin::StorageType st)
					{
						if (st != STInsecure)
							return QList<QByteArray> ();

						QStringList keys = Storage_->allKeys ();
						qDebug () << Q_FUNC_INFO << keys;
						QList<QByteArray> result;
						Q_FOREACH (const QString& key, keys)
							result << key.toUtf8 ();
						return result;
					}

					void Plugin::Save (const QByteArray& key, const QVariantList& values,
							IStoragePlugin::StorageType st, bool overwrite)
					{
						if (st != STInsecure)
							return;

						QVariantList oldValues;
						if (!overwrite)
							oldValues = Load (key, st);
						Storage_->setValue (key, oldValues + values);
					}

					QVariantList Plugin::Load (const QByteArray& key, IStoragePlugin::StorageType st)
					{
						if (st != STInsecure)
							return QVariantList ();

						return Storage_->value (key).toList ();
					}

					void Plugin::Save (const QList<QPair<QByteArray, QVariantList>>& keyValues,
							IStoragePlugin::StorageType st, bool overwrite)
					{
						QPair<QByteArray, QVariantList> keyValue;
						Q_FOREACH (keyValue, keyValues)
							Save (keyValue.first, keyValue.second, st, overwrite);
					}

					QList<QVariantList> Plugin::Load (const QList<QByteArray>& keys, IStoragePlugin::StorageType st)
					{
						QList<QVariantList> result;
						Q_FOREACH (const QByteArray& key, keys)
							result << Load (key, st);
						return result;
					}
				}
			}
		}
	}
}

LC_EXPORT_PLUGIN (leechcraft_secman_simplestorage, LeechCraft::Plugins::SecMan::StoragePlugins::SimpleStorage::Plugin);
