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

#ifndef UTIL_TAGSLINEEDIT_H
#define UTIL_TAGSLINEEDIT_H
#include <memory>
#include <QLineEdit>
#include <util/utilconfig.h>
#include "categoryselector.h"

namespace LeechCraft
{
	namespace Util
	{
		class TagsCompleter;

		/** @brief A line edit class suitable for use with TagsCompleter.
		 *
		 * One would need this extra class because of custom behavior of both
		 * tags completer and line edit semantics.
		 *
		 * @sa TagsCompleter
		 */
		class TagsLineEdit : public QLineEdit
		{
			Q_OBJECT

			friend class TagsCompleter;

			std::unique_ptr<CategorySelector> CategorySelector_;
			TagsCompleter *Completer_;

			QString Separator_;
		public:
			/** @brief Constructs the line edit widget.
			 *
			 * Creates the line edit widget.
			 *
			 * @param[in] parent Parent widget.
			 */
			UTIL_API TagsLineEdit (QWidget *parent);

			/** @brief Adds the selector widget to the line edit.
			 *
			 * Because this function uses the completion model, it should be
			 * used after a TagsCompleter has been set on this line edit.
			 *
			 * @sa TagsCompleter
			 */
			UTIL_API void AddSelector ();

			/** @brief Returns the separator for the tags.
			 *
			 * The default separator is "; ".
			 *
			 * @sa SetSeparator()
			 */
			UTIL_API QString GetSeparator () const;

			/** @brief Sets the separator for the tags.
			 *
			 * This function doesn't update the text in the line edit.
			 *
			 * @sa GetSeparator()
			 */
			UTIL_API void SetSeparator (const QString&);
		public slots:
			/** @brief Completes the string.
			 *
			 * Completes the current text in line edit with completion passed
			 * throught string parameter.
			 *
			 * @param[in] string String with completion.
			 */
			UTIL_API void insertTag (const QString& string);

			/** @brief Sets thew new list of the available tags.
			 *
			 * The list of tags will be passed to the selector if it was
			 * added via AddSelector().
			 *
			 * @param[in] allTags The list of new available tags.
			 */
			UTIL_API void handleTagsUpdated (const QStringList& allTags);

			/** @brief Sets the currently selected tags.
			 *
			 * Sets the line edit text to tags joined by separator. If
			 * tags selector is installed via AddSelector(), the selector
			 * is updated as well.
			 *
			 * @param[in] tags The list of selected tags.
			 */
			UTIL_API void setTags (const QStringList& tags);
		private slots:
			void handleSelectionChanged (const QStringList&);
		protected:
			virtual void keyPressEvent (QKeyEvent*);
			virtual void focusInEvent (QFocusEvent*);
			virtual void contextMenuEvent (QContextMenuEvent*);
			void SetCompleter (TagsCompleter*);
		private:
			QString textUnderCursor () const;
		signals:
			UTIL_API void tagsChosen ();
		};
	};
};

#endif

