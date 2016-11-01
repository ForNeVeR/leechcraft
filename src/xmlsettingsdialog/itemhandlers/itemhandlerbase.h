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

#pragma once

#include <QMap>
#include <QVariant>
#include <QDomElement>
#include "../xmlsettingsdialog.h"

namespace LeechCraft
{
	/** Base class for the handlers of the <item />-QDomElements.
	 */
	class ItemHandlerBase : public QObject
	{
		Q_OBJECT
	public:
		typedef QMap<QString, QVariant> Prop2NewValue_t;

		ItemHandlerBase (Util::XmlSettingsDialog*);

		/** @brief Whether this item handler can handle this particular
		 * element.
		 *
		 * @param[in] element The element to try to handle.
		 * @return Whether this element can be handled.
		 */
		virtual bool CanHandle (const QDomElement& element) const = 0;

		/** @brief Creates the representation widget for the given
		 * element and parent widget \em pwidget.
		 *
		 * The created widget's property "ItemHandler" should point to
		 * the class that created it in order to retrieve it later.
		 *
		 * \em pwidget is guaranteed to have a layout, and the layout is
		 * QGridLayout.
		 *
		 * The created widget should invoke the updatePreferences() slot
		 * when the user changes the value represented by the widget.
		 *
		 * @param[in] element The element to make representation for.
		 * @param[in] pwidget The parent widget of the representation
		 * widget.
		 */
		virtual void Handle (const QDomElement& element, QWidget *pwidget) = 0;

		/** @brief Return the value of the given element and given
		 * predefined value.
		 *
		 * This function should inspect the given \em element and return
		 * a QVariant containing its value. If \em value is given and it
		 * is valid and sensible for this widget, it should be considered
		 * instead.
		 *
		 * @param[in] element The element to retrieve value for.
		 * @param[in] value The default value that should be considered
		 * just in case.
		 * @return The resulting value.
		 */
		virtual QVariant GetValue (const QDomElement& element,
				QVariant value) const = 0;

		/** @brief Sets the value for the widget created earlier.
		 *
		 * This method should update the \em widget so that it represents
		 * the given \em value.
		 *
		 * @param[in] widget The widget created earlier by the call to
		 * Handle().
		 * @param[in] value The value that should be set for this
		 * widget.
		 */
		virtual void SetValue (QWidget *widget, const QVariant& value) const = 0;

		/** @brief Update the value of the given element.
		 *
		 * This function should update the current default \em value of
		 * the given \em element to a new value.
		 *
		 * @param[in,out] element The element which should be updated.
		 * @param[in] value The new value for this element.
		 */
		virtual void UpdateValue (QDomElement& element, const QVariant& value) const = 0;

		/** @brief Returns the list of the changed properties for
		 * widgets that are managed by this item handler.
		 *
		 * @return The list of the changed properties and their values.
		 */
		virtual Prop2NewValue_t GetChangedProperties () const;

		/** @brief Clears the list of the changed properties for
		 * widgets that are managed by this item handler.
		 */
		virtual void ClearChangedProperties ();
	protected:
		/** @brief Returns the value of the object.
		 *
		 * This function should return a correct value, assuming that
		 * \em object is a pointer to a widget created earlier by the
		 * Handle() function of this item handler, so it's safe to
		 * <code>qobject_cast<WidgetType*> (object)</code>.
		 *
		 * @param[in] object Pointer to a widget previously created by
		 * this item handler.
		 * @return The value that the widget pointed by object holds.
		 *
		 * @sa Handle()
		 */
		virtual QVariant GetObjectValue (QObject *object) const = 0;

		Prop2NewValue_t ChangedProperties_;
		Util::XmlSettingsDialog *XSD_;
	protected slots:
		/** @brief This slot should be invoked when created widget
		 * changes value.
		 *
		 * This slot should be invoked by the widgets created in the
		 * Handle() method whenever they change their value.
		 */
		virtual void updatePreferences ();
	};

	typedef std::shared_ptr<ItemHandlerBase> ItemHandlerBase_ptr;
}
