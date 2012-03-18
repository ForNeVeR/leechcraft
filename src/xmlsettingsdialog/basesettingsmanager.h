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

#ifndef XMLSETTINGSDIALOG_BASESETTINGSMANAGER_H
#define XMLSETTINGSDIALOG_BASESETTINGSMANAGER_H
#include <QMap>
#include <QPair>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QDynamicPropertyChangeEvent>
#include <QPointer>
#include "xsdconfig.h"

#define PROP2CHAR(a) (a.toUtf8 ().constData ())

namespace LeechCraft
{
namespace Util
{
	/** @brief Base class for settings manager.
	 *
	 * Facilitates creation of settings managers due to providing some
	 * frequently used features.
	 */
	class XMLSETTINGSMANAGER_API BaseSettingsManager : public QObject
	{
		Q_OBJECT

		typedef QPair<QPointer<QObject>, QByteArray> ObjectElement_t;
		typedef QMap<QByteArray, ObjectElement_t> Properties2Object_t;
		Properties2Object_t ApplyProps_;
		Properties2Object_t SelectProps_;
		bool Initializing_;
		QSettings *Settings_;
	protected:
		bool ReadAllKeys_;
	public:
		BaseSettingsManager (bool readAllKeys = false, QObject* = 0);

		/** @brief Initalizes the settings manager.
		 *
		 * Loads all settings from the QSettings created by BeginSettings and
		 * creates dynamic properties for them.
		 *
		 * @sa Release
		 */
		void Init ();

		/** @brief Prepares the settings manager for deletion.
		 *
		 * Flushes all settigns to the QSettings created by BeginSettings
		 * to prepare settings manager object for the deletion.
		 *
		 * @sa Init
		 */
		void Release ();

		enum EventFlag
		{
			Apply = 0x01,
			Select = 0x02
		};
		Q_DECLARE_FLAGS (EventFlags, EventFlag);

		/** @brief Subscribes object to property changes.
		 *
		 * After a property has changed, a specified function of the
		 * object is called to notify it about the change.
		 *
		 * @param[in] propName The name of property object wants to
		 * subscribe to.
		 * @param[in] object The object instance that will get
		 * notifications.
		 * @param[in] funcName Name of the function that will be called.
		 * Note that it should be known to the Qt's metaobject system, so
		 * it should be a (public) slot.
		 */
		void RegisterObject (const QByteArray& propName,
				QObject* object, const QByteArray& funcName, EventFlags flags = EventFlag::Apply);

		/** @brief Subscribes object to property changes.
		 *
		 * This is an overloaded function provided for convenience.
		 *
		 * @param[in] propNames The names of properties object wants to
		 * subscribe to.
		 * @param[in] object The object instance that will get
		 * notifications.
		 * @param[in] funcName Name of the function that will be called.
		 * Note that it should be known to the Qt's metaobject system, so
		 * it should be a (public) slot.
		 */
		void RegisterObject (const QList<QByteArray>& propNames,
				QObject* object, const QByteArray& funcName, EventFlags flags = EventFlag::Apply);

		/** @brief Gets a property with default value.
		 *
		 * This is a wrapper around standard QObject::property() function.
		 * It checks whether specified property exists, and if so, it
		 * returns its value, otherwise it creates this property, sets its
		 * value to def and returns def.
		 *
		 * @param[in] propName Name of the property that should be checked
		 * and returned.
		 * @param[in] def Default value of the property.
		 * @return Resulting value of the property.
		 */
		QVariant Property (const QString& propName, const QVariant& def);

		/** @brief Sets the value directly, without metaproperties system.
		 *
		 * This function just plainly calls setValue() on the
		 * corresponding QSettings object, without all this
		 * properties machinery.
		 *
		 * @param[in] path The key path.
		 * @param[in] val The value to set.
		 *
		 * @sa GetRawValue()
		 */
		void SetRawValue (const QString& path, const QVariant& val);

		/** @brief Gets the value that is set directly.
		 *
		 * This function plainly returns the value that is set
		 * previously with SetRawValue().
		 *
		 * @param[in] path The key path.
		 * @param[in] def The default value to return.
		 * @return The stored value.
		 *
		 * @sa SetRawValue()
		 */
		QVariant GetRawValue (const QString& path, const QVariant& def = QVariant ()) const;

		void OptionSelected (const QByteArray&, const QVariant&);
	protected:
		virtual bool event (QEvent*);
		/*! @brief Allocates and returns a QSettings object suitable for
		 * use.
		 *
		 * @return The created QSettings object.
		 * @sa EndSettings
		 */
		virtual QSettings* BeginSettings () const = 0;

		/*! @brief Correctly closes the QSettings object.
		 *
		 * Closes the QSettings object previously created by
		 * BeginSettings. It should NOT delete it, BaseSettignsManager's
		 * code would do that.
		 *
		 * @param[in] settings The QSettings object.
		 * @sa BeginSettings
		 */
		virtual void EndSettings (QSettings *settings) const = 0;
	};
}
}

#endif
