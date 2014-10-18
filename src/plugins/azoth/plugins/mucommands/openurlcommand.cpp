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

#include "openurlcommand.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <QStringList>
#include <QUrl>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/iproxyobject.h>

namespace LeechCraft
{
namespace Azoth
{
namespace MuCommands
{
	namespace
	{
		QStringList GetAllUrls (IProxyObject *azothProxy, ICLEntry *entry)
		{
			QStringList urls;
			for (const auto msg : entry->GetAllMessages ())
			{
				switch (msg->GetMessageType ())
				{
				case IMessage::Type::ChatMessage:
				case IMessage::Type::MUCMessage:
					break;
				default:
					continue;
				}

				urls += azothProxy->GetFormatterProxy ().FindLinks (msg->GetBody ());
			}

			urls.removeDuplicates ();

			return urls;
		}
	}

	StringCommandResult ListUrls (IProxyObject *azothProxy, ICLEntry *entry, const QString&)
	{
		const auto& urls = GetAllUrls (azothProxy, entry);
		const auto& body = urls.isEmpty () ?
				QObject::tr ("Sorry, no links found, chat more!") :
				QObject::tr ("Found links:") + "<ol><li>" + urls.join ("</li><li>") + "</li></ol>";
		return { true, body };
	}

	namespace
	{
		using UrlIndex_t = int;

		struct UrlRange
		{
			boost::optional<int> Start_;
			boost::optional<int> End_;
		};

		struct UrlRegExp
		{
			std::string Pat_;
		};

		using OpenUrlParams_t = boost::variant<UrlIndex_t, UrlRange, UrlRegExp>;
	}
}
}
}

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Azoth::MuCommands::UrlRange,
		(boost::optional<int>, Start_)
		(boost::optional<int>, End_));

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Azoth::MuCommands::UrlRegExp,
		(std::string, Pat_));

namespace LeechCraft
{
namespace Azoth
{
namespace MuCommands
{
	namespace
	{
		namespace ascii = boost::spirit::ascii;
		namespace qi = boost::spirit::qi;
		namespace phoenix = boost::phoenix;

		template<typename Iter>
		struct Parser : qi::grammar<Iter, OpenUrlParams_t ()>
		{
			qi::rule<Iter, OpenUrlParams_t ()> Start_;
			qi::rule<Iter, UrlIndex_t ()> Index_;
			qi::rule<Iter, UrlRange ()> Range_;
			qi::rule<Iter, UrlRegExp ()> RegExp_;

			Parser ()
			: Parser::base_type { Start_ }
			{
				Index_ = qi::int_;
				Range_ = -(qi::int_) >> qi::lit (':') >> -(qi::int_);
				RegExp_ = qi::lit ("rx ") >> +qi::char_;

				Start_ = Range_ | Index_ | RegExp_;
			}
		};

		template<typename Iter>
		OpenUrlParams_t ParseCommand (Iter begin, Iter end)
		{
			OpenUrlParams_t res;
			qi::parse (begin, end, Parser<Iter> {}, res);
			return res;
		}

		OpenUrlParams_t ParseCommand (const QString& cmd)
		{
			const auto& unicode = cmd.section (' ', 1).toUtf8 ();
			return ParseCommand (unicode.begin (), unicode.end ());
		}

		struct ParseResultVisitor : public boost::static_visitor<CommandResult_t>
		{
			const QStringList Urls_;
			IEntityManager * const IEM_;
			const TaskParameters Params_;

			ParseResultVisitor (const QStringList& urls, IEntityManager *iem, TaskParameters params)
			: Urls_ { urls }
			, IEM_ { iem }
			, Params_ { params }
			{
			}

			CommandResult_t operator() (UrlIndex_t idx) const
			{
				return (*this) ({ idx, idx });
			}

			CommandResult_t operator() (const UrlRange& range) const
			{
				if (Urls_.isEmpty ())
					return true;

				const auto begin = boost::get_optional_value_or (range.Start_, 1) - 1;
				const auto end = boost::get_optional_value_or (range.End_, Urls_.size ()) - 1;

				if (begin >= end)
					return StringCommandResult
					{
						true,
						QObject::tr ("Begin index should be greater than end index.")
					};

				if (end >= Urls_.size ())
					return StringCommandResult
					{
						true,
						QObject::tr ("End index is out of bounds of the URLs list.")
					};

				for (auto i = begin; i <= end; ++i)
				{
					const auto& url = Urls_.value (i);
					if (url.isEmpty ())
						continue;

					const auto& entity = Util::MakeEntity (QUrl::fromEncoded (url.toUtf8 ()),
							{},
							Params_ | FromUserInitiated);
					IEM_->HandleEntity (entity);
				}

				return true;
			}

			CommandResult_t operator() (const UrlRegExp& rx) const
			{
				const auto& matching = Urls_.filter (QRegExp { QString::fromStdString (rx.Pat_) });

				for (const auto& url : matching)
				{
					if (url.isEmpty ())
						continue;

					const auto& entity = Util::MakeEntity (QUrl::fromEncoded (url.toUtf8 ()),
							{},
							Params_ | FromUserInitiated);
					IEM_->HandleEntity (entity);
				}

				return true;
			}
		};
	}

	CommandResult_t OpenUrl (const ICoreProxy_ptr& coreProxy, IProxyObject *azothProxy,
			ICLEntry *entry, const QString& text, TaskParameters params)
	{
		auto parseResult = ParseCommand (text);
		return boost::apply_visitor (ParseResultVisitor
				{
					GetAllUrls (azothProxy, entry),
					coreProxy->GetEntityManager (),
					params
				},
				parseResult);
	}
}
}
}
