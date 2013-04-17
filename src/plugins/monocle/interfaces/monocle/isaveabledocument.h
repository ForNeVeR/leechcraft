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

#pragma once

#include <QtPlugin>

namespace LeechCraft
{
namespace Monocle
{
	/** @brief Interface for documents that can be saved.
	 *
	 * This interface should be implemented by documents for formats that
	 * allow saving the document after its editable elements (like forms
	 * or annotations) were modified.
	 *
	 * Not all documents of the same format support saving: for example,
	 * encrypted PDF documents cannot be saved, while regular ones can.
	 * Thus the CanSave() method checks whether this particular document
	 * can be saved.
	 *
	 * @sa ISupportForms, ISupportAnnotations
	 */
	class ISaveableDocument
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~ISaveableDocument () {}

		/** @brief Describes the result of check for the possibility of
		 * saving.
		 */
		struct SaveQueryResult
		{
			/** @brief Whether the document can be saved.
			 */
			bool CanSave_;

			/** @brief Human-readable reason string.
			 *
			 * If the document cannot be saved, this field contains the
			 * human-readable string containing the reason why it can't
			 * be saved.
			 */
			QString Reason_;
		};

		/** @brief Checks whether this document can be saved.
		 *
		 * This method should check if the document can be saved and
		 * return a proper SaveQueryResult.
		 *
		 * @return Whether this document can be saved, and reason string
		 * if it can't.
		 */
		virtual SaveQueryResult CanSave () const = 0;

		/** @brief Saves the document at the given path.
		 *
		 * The \em path can be equal to the original document path,
		 * plugins should take this into account.
		 *
		 * @param[in] path The full path to the target file including the
		 * file name.
		 * @return Whether the document is saved successfully.
		 */
		virtual bool Save (const QString& path) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Monocle::ISaveableDocument,
		"org.LeechCraft.Monocle.ISaveableDocument/1.0");
