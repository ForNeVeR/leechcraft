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

#include <stdexcept>
#include <type_traits>
#include <memory>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/fold.hpp>
#include <boost/fusion/include/filter_if.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/include/zip.hpp>
#include <boost/variant/variant.hpp>
#include <QStringList>
#include <QDateTime>
#include <QPair>
#include <QSqlQuery>
#include <QVariant>
#include <QtDebug>
#include "oraltypes.h"
#include "prelude.h"

typedef std::shared_ptr<QSqlQuery> QSqlQuery_ptr;

namespace LeechCraft
{
namespace Poleemery
{
namespace oral
{
	class QueryException : public std::runtime_error
	{
		const QSqlQuery_ptr Query_;
	public:
		QueryException (const std::string& str, const QSqlQuery_ptr& q)
		: std::runtime_error (str)
		, Query_ (q)
		{
		}

		virtual ~QueryException () throw ()
		{
		}

		const QSqlQuery_ptr& GetQueryPtr () const
		{
			return Query_;
		}

		const QSqlQuery& GetQuery () const
		{
			return *Query_;
		}
	};

	namespace detail
	{
		template<typename Seq, int Idx>
		struct GetFieldName
		{
			static QString value () { return boost::fusion::extension::struct_member_name<Seq, Idx>::call (); }
		};

		template<typename S, typename N>
		struct GetFieldsNames_
		{
			QStringList operator() () const
			{
				return QStringList { GetFieldName<S, N::value>::value () } + GetFieldsNames_<S, typename boost::mpl::next<N>::type> {} ();
			}
		};

		template<typename S>
		struct GetFieldsNames_<S, typename boost::fusion::result_of::size<S>::type>
		{
			QStringList operator() () const
			{
				return {};
			}
		};

		template<typename S>
		struct GetFieldsNames : GetFieldsNames_<S, boost::mpl::int_<0>>
		{
		};

		template<typename Seq, int Idx>
		struct GetBoundName
		{
			static QString value () { return ':' + Seq::ClassName () + "_" + GetFieldName<Seq, Idx>::value (); }
		};
	}

	template<typename T>
	struct Type2Name;

	template<>
	struct Type2Name<int>
	{
		QString operator() () const { return "INTEGER"; }
	};

	template<>
	struct Type2Name<double>
	{
		QString operator() () const { return "REAL"; }
	};

	template<>
	struct Type2Name<bool>
	{
		QString operator() () const { return "INTEGER"; }
	};

	template<>
	struct Type2Name<QString>
	{
		QString operator() () const { return "TEXT"; }
	};

	template<>
	struct Type2Name<QDateTime>
	{
		QString operator() () const { return "DATETIME"; }
	};

	template<typename T>
	struct Type2Name<Unique<T>>
	{
		QString operator() () const { return Type2Name<T> () () + " UNIQUE"; }
	};

	template<typename T>
	struct Type2Name<PKey<T>>
	{
		QString operator() () const { return Type2Name<T> () () + " PRIMARY KEY"; }
	};

	template<>
	struct Type2Name<PKey<int>>
	{
		QString operator() () const { return Type2Name<int> () () + " PRIMARY KEY AUTOINCREMENT"; }
	};

	template<typename Seq, int Idx>
	struct Type2Name<References<Seq, Idx>>
	{
		QString operator() () const
		{
			return Type2Name<typename References<Seq, Idx>::value_type> () () +
					" REFERENCES " + Seq::ClassName () + " (" + detail::GetFieldName<Seq, Idx>::value () + ") ON DELETE CASCADE";
		}
	};

	namespace detail
	{
		struct Types
		{
			template<typename T>
			QStringList operator() (const QStringList& init, const T&) const
			{
				return init + QStringList { Type2Name<T> () () };
			}
		};
	}

	template<typename T>
	struct ToVariant
	{
		QVariant operator() (const T& t) const
		{
			return t;
		}
	};

	template<typename T>
	struct ToVariant<Unique<T>>
	{
		QVariant operator() (const Unique<T>& t) const
		{
			return static_cast<typename Unique<T>::value_type> (t);
		}
	};

	template<typename T>
	struct ToVariant<PKey<T>>
	{
		QVariant operator() (const PKey<T>& t) const
		{
			return static_cast<typename PKey<T>::value_type> (t);
		}
	};

	template<typename Seq, int Idx>
	struct ToVariant<References<Seq, Idx>>
	{
		QVariant operator() (const References<Seq, Idx>& t) const
		{
			return static_cast<typename References<Seq, Idx>::value_type> (t);
		}
	};

	namespace detail
	{
		struct Inserter
		{
			const bool BindPrimaryKey_;
			QSqlQuery_ptr Q_;

			template<typename T>
			QStringList operator() (QStringList bounds, const T& t) const
			{
				if (BindPrimaryKey_ || !IsPKey<T>::value)
					Q_->bindValue (bounds.takeFirst (), ToVariant<T> {} (t));
				return bounds;
			}
		};
	}

	template<typename T>
	struct FromVariant
	{
		T operator() (const QVariant& var) const
		{
			return var.value<T> ();
		}
	};

	template<typename T>
	struct FromVariant<Unique<T>>
	{
		T operator() (const QVariant& var) const
		{
			return var.value<T> ();
		}
	};

	template<typename T>
	struct FromVariant<PKey<T>>
	{
		T operator() (const QVariant& var) const
		{
			return var.value<T> ();
		}
	};

	template<typename Seq, int Idx>
	struct FromVariant<References<Seq, Idx>>
	{
		typedef typename References<Seq, Idx>::value_type value_type;

		value_type operator() (const QVariant& var) const
		{
			return var.value<value_type> ();
		}
	};

	namespace detail
	{
		struct Selector
		{
			QSqlQuery_ptr Q_;

			template<typename T>
			int operator() (int index, T& t) const
			{
				t = FromVariant<T> {} (Q_->value (index));
				return index + 1;
			}
		};

		struct CachedFieldsData
		{
			QString Table_;
			QSqlDatabase DB_;

			QList<QString> Fields_;
			QList<QString> BoundFields_;
		};

		template<typename T>
		std::function<void (T)> MakeInserter (CachedFieldsData data, QSqlQuery_ptr insertQuery, bool bindPrimaryKey)
		{
			return [data, insertQuery, bindPrimaryKey] (const T& t)
			{
				boost::fusion::fold<T, QStringList, Inserter> (t, data.BoundFields_, Inserter { bindPrimaryKey, insertQuery });
				if (!insertQuery->exec ())
					throw QueryException ("insert query execution failed", insertQuery);
			};
		}

		template<typename T>
		struct Lazy
		{
			typedef T type;
		};

		template<typename Seq, typename MemberIdx = boost::mpl::int_<0>>
		struct FindPKey
		{
			static_assert ((boost::fusion::result_of::size<Seq>::value) != (MemberIdx::value),
					"Primary key not found");

			typedef typename boost::fusion::result_of::at<Seq, MemberIdx>::type item_type;
			typedef typename std::conditional<
						IsPKey<typename std::decay<item_type>::type>::value,
						Lazy<MemberIdx>,
						Lazy<FindPKey<Seq, typename boost::mpl::next<MemberIdx>>>
					>::type::type result_type;
		};

		template<typename T>
		QPair<QSqlQuery_ptr, std::function<void (T&)>> AdaptInsert (CachedFieldsData data)
		{
			const auto index = FindPKey<T>::result_type::value;

			data.Fields_.removeAt (index);
			data.BoundFields_.removeAt (index);

			const auto& insert = "INSERT INTO " + data.Table_ +
					" (" + QStringList { data.Fields_ }.join (", ") + ") VALUES (" +
					QStringList { data.BoundFields_ }.join (", ") + ");";
			QSqlQuery_ptr insertQuery (new QSqlQuery (data.DB_));
			insertQuery->prepare (insert);

			auto inserter = MakeInserter<T> (data, insertQuery, false);
			auto insertUpdater = [inserter, insertQuery] (T& t)
			{
				inserter (t);
				constexpr auto index = FindPKey<T>::result_type::value;
				boost::fusion::at_c<index> (t) = FromVariant<typename std::decay<typename boost::fusion::result_of::at_c<T, index>::type>::type> {} (insertQuery->lastInsertId ());
			};
			return
			{
				insertQuery,
				insertUpdater
			};
		}

		template<typename T>
		QPair<QSqlQuery_ptr, std::function<void (T)>> AdaptUpdate (const CachedFieldsData& data)
		{
			const auto index = FindPKey<T>::result_type::value;

			auto removedFields = data.Fields_;
			auto removedBoundFields = data.BoundFields_;

			const auto& fieldName = removedFields.takeAt (index);
			const auto& boundName = removedBoundFields.takeAt (index);

			const auto& statements = ZipWith (removedFields, removedBoundFields,
					[] (const QString& s1, const QString& s2) -> QString
						{ return s1 + " = " + s2; });

			const auto& update = "UPDATE " + data.Table_ +
					" SET " + QStringList { statements }.join (", ") +
					" WHERE " + fieldName + " = " + boundName + ";";

			QSqlQuery_ptr updateQuery (new QSqlQuery (data.DB_));
			updateQuery->prepare (update);

			return { updateQuery, MakeInserter<T> (data, updateQuery, true) };
		}

		template<typename T>
		QPair<QSqlQuery_ptr, std::function<void (T)>> AdaptDelete (CachedFieldsData data)
		{
			const auto index = FindPKey<T>::result_type::value;

			const auto& boundName = data.BoundFields_.at (index);
			const auto& del = "DELETE FROM " + data.Table_ +
					" WHERE " + data.Fields_.at (index) + " = " + boundName + ";";

			QSqlQuery_ptr deleteQuery (new QSqlQuery (data.DB_));
			deleteQuery->prepare (del);

			auto deleter = [deleteQuery, boundName] (const T& t)
			{
				constexpr auto index = FindPKey<T>::result_type::value;
				deleteQuery->bindValue (boundName,
						ToVariant<typename std::decay<typename boost::fusion::result_of::at_c<T, index>::type>::type> {} (boost::fusion::at_c<index> (t)));
				if (!deleteQuery->exec ())
					throw QueryException ("delete query execution failed", deleteQuery);
			};

			return { deleteQuery, deleter };
		}

		template<typename T>
		QList<T> PerformSelect (QSqlQuery_ptr q)
		{
			if (!q->exec ())
				throw QueryException ("fetch query execution failed", q);

			QList<T> result;
			while (q->next ())
			{
				T t;
				boost::fusion::fold<T, int, Selector> (t, 0, Selector { q });
				result << t;
			}
			q->finish ();
			return result;
		}

		template<typename T>
		QPair<QSqlQuery_ptr, std::function<QList<T> ()>> AdaptSelectAll (const CachedFieldsData& data)
		{
			const auto& selectAll = "SELECT " + QStringList { data.Fields_ }.join (", ") + " FROM " + data.Table_ + ";";
			QSqlQuery_ptr selectQuery (new QSqlQuery (data.DB_));
			selectQuery->prepare (selectAll);
			auto selector = [selectQuery] () { return PerformSelect<T> (selectQuery); };
			return { selectQuery, selector };
		}

		template<int Field, int... Fields>
		struct SelectFields
		{
			QList<QPair<QString, QString>> operator() (const CachedFieldsData& data) const
			{
				return QPair<QString, QString> { data.Fields_.at (Field), data.BoundFields_.at (Field) } + SelectFields<Fields...> {} (data);
			}
		};

		template<int Field>
		struct SelectFields<Field>
		{
			QList<QPair<QString, QString>> operator() (const CachedFieldsData& data) const
			{
				return { { data.Fields_.at (Field), data.BoundFields_.at (Field) } };
			}
		};

		template<int HeadT, int... TailT>
		struct FieldsUnpacker
		{
			static const int Head = HeadT;
			typedef FieldsUnpacker<TailT...> Tail_t;
		};

		template<int HeadT>
		struct FieldsUnpacker<HeadT>
		{
			static const int Head = HeadT;
			typedef std::false_type Tail_t;
		};

		template<typename FieldsUnpacker, typename HeadArg, typename... TailArgs>
		struct ValueBinder
		{
			QSqlQuery_ptr Query_;
			QList<QString> BoundFields_;

			void operator() (const HeadArg& arg, const TailArgs&... tail) const
			{
				Query_->bindValue (BoundFields_.at (FieldsUnpacker::Head), arg);

				ValueBinder<typename FieldsUnpacker::Tail_t, TailArgs...> { Query_, BoundFields_ } (tail...);
			}
		};

		template<typename FieldsUnpacker, typename HeadArg>
		struct ValueBinder<FieldsUnpacker, HeadArg>
		{
			QSqlQuery_ptr Query_;
			QList<QString> BoundFields_;

			void operator() (const HeadArg& arg) const
			{
				Query_->bindValue (BoundFields_.at (FieldsUnpacker::Head), arg);
			}
		};

		enum class ExprType
		{
			LeafPlaceholder,
			LeafData,

			Greater,
			Less,
			Equal,
			Geq,
			Leq,
			Neq,

			And,
			Or
		};

		QString TypeToSql (ExprType type)
		{
			switch (type)
			{
			case ExprType::Greater:
				return ">";
			case ExprType::Less:
				return "<";
			case ExprType::Equal:
				return "=";
			case ExprType::Geq:
				return ">=";
			case ExprType::Leq:
				return "<=";
			case ExprType::Neq:
				return "!=";
			case ExprType::And:
				return "AND";
			case ExprType::Or:
				return "OR";

			case ExprType::LeafPlaceholder:
			case ExprType::LeafData:
				return "invalid type";
			}

			qWarning () << Q_FUNC_INFO
					<< "unhandled type"
					<< static_cast<int> (type);
			return {};
		}

		template<ExprType Type>
		struct IsLeaf : std::false_type {};

		template<>
		struct IsLeaf<ExprType::LeafPlaceholder> : std::true_type {};

		template<>
		struct IsLeaf<ExprType::LeafData> : std::true_type {};

		template<ExprType Type1, ExprType Type2>
		struct IsCompatible : std::false_type {};

		template<ExprType Type>
		struct IsCompatible<Type, ExprType::And> : std::true_type {};

		template<ExprType Type>
		struct IsCompatible<Type, ExprType::Or> : std::true_type {};

		template<ExprType Type>
		struct IsCompatible<Type, ExprType::LeafPlaceholder> : std::true_type {};

		template<ExprType Type>
		struct IsCompatible<Type, ExprType::LeafData> : std::true_type {};

		template<typename T>
		constexpr T Ctfy (T t)
		{
			return t;
		}

		template<ExprType T1, ExprType T2>
		constexpr bool CheckCompatible ()
		{
			return IsCompatible<T1, T2>::value || IsCompatible<T2, T1>::value;
		}

		template<typename T>
		struct ToSqlState
		{
			int LastID_;
			QVariantMap BoundMembers_;
		};

		template<ExprType Type, typename L = void, typename R = void>
		class ExprTree
		{
			static const ExprType Type_ = Type;

			L Left_;
			R Right_;
		public:
			ExprTree (const L& l, const R& r)
			: Left_ (l)
			, Right_ (r)
			{
			}

			template<typename T>
			QString ToSql (ToSqlState<T>& state) const
			{
				return Left_.ToSql (state) + " " + TypeToSql (Type_) + " " + Right_.ToSql (state);
			}
		};

		template<>
		class ExprTree<ExprType::LeafPlaceholder, void, void>
		{
			static const ExprType Type_ = ExprType::LeafPlaceholder;

			int Index_;
		public:
			ExprTree (int idx)
			: Index_ (idx)
			{
			}

			template<typename T>
			QString ToSql (ToSqlState<T>&) const
			{
				return detail::GetFieldsNames<T> () ().at (Index_);
			}
		};

		template<typename T>
		class ExprTree<ExprType::LeafData, T, void>
		{
			static const ExprType Type_ = ExprType::LeafData;

			T Data_;
		public:
			ExprTree (const T& t)
			: Data_ (t)
			{
			}

			template<typename ObjT>
			QString ToSql (ToSqlState<ObjT>& state) const
			{
				const auto& name = ":bound_" + QString::number (++state.LastID_);
				state.BoundMembers_ [name] = ToVariant<T> {} (Data_);
				return name;
			}
		};

		template<typename T>
		struct IsExprTree : std::false_type {};

		template<ExprType Type, typename L, typename R>
		struct IsExprTree<ExprTree<Type, L, R>> : std::true_type {};

		template<ExprType LType, typename LL, typename LR, ExprType RType, typename RL, typename RR>
		ExprTree<ExprType::Less, ExprTree<LType, LL, LR>, ExprTree<RType, RL, RR>> operator< (const ExprTree<LType, LL, LR>& left, const ExprTree<RType, RL, RR>& right)
		{
			static_assert (CheckCompatible<LType, RType> (), "comparing incompatible subexpressions");
			return { left, right };
		}

		template<ExprType LType, typename LL, typename LR, typename R>
		ExprTree<ExprType::Less, ExprTree<LType, LL, LR>, ExprTree<ExprType::LeafData, R>> operator< (const ExprTree<LType, LL, LR>& left, const R& right)
		{
			return left < ExprTree<ExprType::LeafData, R> { right };
		}

		template<ExprType RType, typename RL, typename RR, typename L>
		ExprTree<ExprType::Less, ExprTree<ExprType::LeafData, L>, ExprTree<RType, RL, RR>> operator< (const L& left, const ExprTree<RType, RL, RR>& right)
		{
			return ExprTree<ExprType::LeafData, L> { left } < right;
		}

		template<ExprType LType, typename LL, typename LR, ExprType RType, typename RL, typename RR>
		ExprTree<ExprType::Equal, ExprTree<LType, LL, LR>, ExprTree<RType, RL, RR>> operator== (const ExprTree<LType, LL, LR>& left, const ExprTree<RType, RL, RR>& right)
		{
			static_assert (CheckCompatible<LType, RType> (), "comparing incompatible subexpressions");
			return { left, right };
		}

		template<ExprType LType, typename LL, typename LR, typename R>
		ExprTree<ExprType::Equal, ExprTree<LType, LL, LR>, ExprTree<ExprType::LeafData, R>> operator== (const ExprTree<LType, LL, LR>& left, const R& right)
		{
			return left == ExprTree<ExprType::LeafData, R> { right };
		}

		template<ExprType RType, typename RL, typename RR, typename L>
		ExprTree<ExprType::Equal, ExprTree<ExprType::LeafData, L>, ExprTree<RType, RL, RR>> operator== (const L& left, const ExprTree<RType, RL, RR>& right)
		{
			return ExprTree<ExprType::LeafData, L> { left } == right;
		}

		template<ExprType LType, typename LL, typename LR, ExprType RType, typename RL, typename RR>
		ExprTree<ExprType::And, ExprTree<LType, LL, LR>, ExprTree<RType, RL, RR>> operator&& (const ExprTree<LType, LL, LR>& left, const ExprTree<RType, RL, RR>& right)
		{
			return { left, right };
		}

		template<ExprType LType, typename LL, typename LR, typename R>
		ExprTree<ExprType::And, ExprTree<LType, LL, LR>, ExprTree<ExprType::LeafData, R>> operator&& (const ExprTree<LType, LL, LR>& left, const R& right)
		{
			return left && ExprTree<ExprType::LeafData, R> { right };
		}

		template<ExprType RType, typename RL, typename RR, typename L>
		ExprTree<ExprType::And, ExprTree<ExprType::LeafData, L>, ExprTree<RType, RL, RR>> operator&& (const L& left, const ExprTree<RType, RL, RR>& right)
		{
			return ExprTree<ExprType::LeafData, L> { left } && right;
		}

		template<typename T>
		class ByFieldsWrapper
		{
			CachedFieldsData Cached_;
		public:
			ByFieldsWrapper ()
			{
			}

			ByFieldsWrapper (const CachedFieldsData& data)
			: Cached_ (data)
			{
			}

			template<int... Fields>
			class ByFieldsSelector
			{
				const CachedFieldsData Cached_;
				QSqlQuery_ptr Query_;
			public:
				ByFieldsSelector (const ByFieldsWrapper<T>& w)
				: Cached_ (w.Cached_)
				, Query_ (new QSqlQuery (w.Cached_.DB_))
				{
					QStringList whereClauses;
					for (const auto& pair : SelectFields<Fields...> {} (Cached_))
						whereClauses << pair.first + " = " + pair.second;

					auto selectAll = "SELECT " + QStringList { Cached_.Fields_ }.join (", ") +
							" FROM " + Cached_.Table_ +
							" WHERE " + whereClauses.join (" AND ") + ";";
					Query_->prepare (selectAll);
				}

				template<typename... Args>
				QList<T> operator() (Args... args) const
				{
					ValueBinder<FieldsUnpacker<Fields...>, Args...> { Query_, Cached_.BoundFields_ } (args...);
					return PerformSelect<T> (Query_);
				}
			};

			template<int... Fields>
			ByFieldsSelector<Fields...> Prepare ()
			{
				return { *this };
			}

			template<ExprType Type, typename L, typename R>
			QList<T> operator() (const ExprTree<Type, L, R>& tree) const
			{
				ToSqlState<T> state { 0, {} };

				auto selectAll = "SELECT " + QStringList { Cached_.Fields_ }.join (", ") +
						" FROM " + Cached_.Table_ +
						" WHERE " + tree.ToSql (state) + ";";
				qDebug () << selectAll << state.BoundMembers_;

				QSqlQuery_ptr query (new QSqlQuery (Cached_.DB_));
				query->prepare (selectAll);
				for (auto i = state.BoundMembers_.begin (), end = state.BoundMembers_.end (); i != end; ++i)
					query->bindValue (i.key (), *i);
				return PerformSelect<T> (query);
			}
		};

		template<typename T>
		ByFieldsWrapper<T> AdaptSelectFields (const CachedFieldsData& data)
		{
			return ByFieldsWrapper<T> (data);
		}

		template<typename OrigSeq, typename OrigIdx, typename RefSeq, typename MemberIdx>
		struct FieldInfo
		{
		};

		template<typename To, typename OrigSeq, typename OrigIdx, typename T>
		struct FieldAppender
		{
			typedef To value_type;
		};

		template<typename To, typename OrigSeq, typename OrigIdx, typename RefSeq, int RefIdx>
		struct FieldAppender<To, OrigSeq, OrigIdx, References<RefSeq, RefIdx>>
		{
			typedef typename boost::fusion::result_of::as_vector<
					typename boost::fusion::result_of::push_front<
						To,
						FieldInfo<OrigSeq, OrigIdx, RefSeq, boost::mpl::int_<RefIdx>>
					>::type
				>::type value_type;
		};

		template<typename Seq, typename MemberIdx>
		struct CollectRefs_
		{
			typedef typename FieldAppender<
					typename CollectRefs_<Seq, typename boost::mpl::next<MemberIdx>::type>::type_list,
					Seq,
					MemberIdx,
					typename std::decay<typename boost::fusion::result_of::at<Seq, MemberIdx>::type>::type
				>::value_type type_list;
		};

		template<typename Seq>
		struct CollectRefs_<Seq, typename boost::fusion::result_of::size<Seq>::type>
		{
			typedef boost::fusion::vector<> type_list;
		};

		template<typename Seq>
		struct CollectRefs : CollectRefs_<Seq, boost::mpl::int_<0>>
		{
		};

		struct Ref2Select
		{
			template<typename OrigSeq, typename OrigIdx, typename RefSeq, typename RefIdx>
			QStringList operator() (const QStringList& init, const FieldInfo<OrigSeq, OrigIdx, RefSeq, RefIdx>&) const
			{
				const auto& thisQualified = OrigSeq::ClassName () + "." + GetFieldName<OrigSeq, OrigIdx::value>::value ();
				return init + QStringList { thisQualified + " = " + GetBoundName<RefSeq, RefIdx::value>::value () };
			}
		};

		template<typename T>
		struct ExtrObj;

		template<typename OrigSeq, typename OrigIdx, typename RefSeq, typename MemberIdx>
		struct ExtrObj<FieldInfo<OrigSeq, OrigIdx, RefSeq, MemberIdx>>
		{
			typedef RefSeq type;
		};

		struct SingleBind
		{
			QSqlQuery_ptr Q_;

			template<typename ObjType, typename OrigSeq, typename OrigIdx, typename RefSeq, typename RefIdx>
			void operator() (const boost::fusion::vector2<ObjType, const FieldInfo<OrigSeq, OrigIdx, RefSeq, RefIdx>&>& pair) const
			{
				Q_->bindValue (GetBoundName<RefSeq, RefIdx::value>::value (),
						ToVariant<typename std::decay<typename boost::fusion::result_of::at<RefSeq, RefIdx>::type>::type> () (boost::fusion::at<RefIdx> (boost::fusion::at_c<0> (pair))));
			}
		};

		template<typename T, typename RefSeq>
		struct MakeBinder
		{
			typedef typename boost::mpl::transform<RefSeq, ExtrObj<boost::mpl::_1>> transform_view;
			typedef typename transform_view::type objects_view;
			typedef typename boost::fusion::result_of::as_vector<objects_view>::type objects_vector;

			QSqlQuery_ptr Q_;

			QList<T> operator() (const objects_vector& objs)
			{
				boost::fusion::for_each (boost::fusion::zip (objs, RefSeq {}), SingleBind { Q_ });
				return PerformSelect<T> (Q_);
			}
		};

		template<typename T, typename ObjInfo>
		typename std::enable_if<CollectRefs<T>::type_list::size::value == 1>::type AdaptSelectRef (const CachedFieldsData& data, ObjInfo& info)
		{
			typedef typename CollectRefs<T>::type_list references_list;
			const auto& statements = boost::fusion::fold (references_list {}, QStringList {}, Ref2Select {});

			const auto& selectAll = "SELECT " + QStringList { data.Fields_ }.join (", ") +
					" FROM " + data.Table_ +
					(statements.isEmpty () ? "" : " WHERE ") + statements.join (" AND ") +
					";";
			QSqlQuery_ptr selectQuery (new QSqlQuery (data.DB_));
			selectQuery->prepare (selectAll);

			info.SelectByFKeys_ = selectQuery;
			info.SelectByFKeysActor_ = MakeBinder<T, references_list> { selectQuery };
		}

		template<typename T, typename Ret>
		struct WrapAsFunc
		{
			typedef std::function<QList<Ret> (T)> type;
		};

		template<typename T>
		struct MakeSingleBinder
		{
			const CachedFieldsData Data_;

			template<typename Vec, typename OrigObj, typename OrigIdx, typename RefObj, typename RefIdx>
			auto operator() (Vec vec, const FieldInfo<OrigObj, OrigIdx, RefObj, RefIdx>&) -> decltype (boost::fusion::push_back (vec, typename WrapAsFunc<RefObj, T>::type {}))
			{
				const auto& boundName = GetBoundName<OrigObj, OrigIdx::value>::value ();
				const auto& query = "SELECT " + QStringList { Data_.Fields_ }.join (", ") +
						" FROM " + Data_.Table_ +
						" WHERE " + GetFieldName<OrigObj, OrigIdx::value>::value () + " = " + boundName +
						";";
				QSqlQuery_ptr selectQuery (new QSqlQuery (Data_.DB_));
				selectQuery->prepare (query);

				typename WrapAsFunc<RefObj, T>::type inserter = [selectQuery, boundName] (const RefObj& obj) -> QList<T>
				{
					selectQuery->bindValue (boundName,
							ToVariant<typename std::decay<typename boost::fusion::result_of::at<RefObj, RefIdx>::type>::type> {} (boost::fusion::at<RefIdx> (obj)));
					return PerformSelect<T> (selectQuery);
				};

				return boost::fusion::push_back (vec, inserter);
			}
		};

		template<typename T, typename ObjInfo>
		typename std::enable_if<CollectRefs<T>::type_list::size::value >= 2>::type AdaptSelectRef (const CachedFieldsData& data, ObjInfo& info)
		{
			typedef typename CollectRefs<T>::type_list references_list;
			const auto& statements = boost::fusion::fold (references_list {}, QStringList {}, Ref2Select {});

			const auto& selectAll = "SELECT " + QStringList { data.Fields_ }.join (", ") +
					" FROM " + data.Table_ +
					(statements.isEmpty () ? "" : " WHERE ") + statements.join (" AND ") +
					";";
			QSqlQuery_ptr selectQuery (new QSqlQuery (data.DB_));
			selectQuery->prepare (selectAll);

			info.SelectByFKeys_ = selectQuery;
			info.SelectByFKeysActor_ = MakeBinder<T, references_list> { selectQuery };

			auto singleSelectors = boost::fusion::fold (references_list {}, boost::fusion::vector<> {}, MakeSingleBinder<T> { data });
			info.SingleFKeySelectors_ = boost::fusion::as_vector (singleSelectors);
		}

		template<typename T, typename ObjInfo>
		typename std::enable_if<CollectRefs<T>::type_list::size::value <= 0>::type AdaptSelectRef (const CachedFieldsData&, ObjInfo&)
		{
		}

		template<typename T>
		QString AdaptCreateTable (const CachedFieldsData& data)
		{
			const QList<QString> types = boost::fusion::fold (T {}, QStringList {}, Types {});

			auto statements = ZipWith (types, data.Fields_,
					[] (const QString& type, const QString& field) -> QString { return field + " " + type; });
			return "CREATE TABLE " + data.Table_ +  " (" + QStringList { statements }.join (", ") + ");";
		}

		template<typename T, typename Enable = void>
		struct ObjectInfoFKeysHelper
		{
		};

		template<typename T>
		struct ObjectInfoFKeysHelper<T, typename std::enable_if<CollectRefs<T>::type_list::size::value == 1, void>::type>
		{
			QSqlQuery_ptr SelectByFKeys_;
			std::function<QList<T> (typename MakeBinder<T, typename CollectRefs<T>::type_list>::objects_vector)> SelectByFKeysActor_;
		};

		template<typename T>
		struct ObjectInfoFKeysHelper<T, typename std::enable_if<CollectRefs<T>::type_list::size::value >= 2, void>::type>
		{
			typedef typename MakeBinder<T, typename CollectRefs<T>::type_list>::objects_vector objects_vector;
			QSqlQuery_ptr SelectByFKeys_;
			std::function<QList<T> (objects_vector)> SelectByFKeysActor_;

			typedef typename boost::mpl::transform<objects_vector, WrapAsFunc<boost::mpl::_1, T>>::type transform_view;
			typename boost::fusion::result_of::as_vector<transform_view>::type SingleFKeySelectors_;
		};
	}

	template<typename T>
	struct ObjectInfo : detail::ObjectInfoFKeysHelper<T>
	{
		QSqlQuery_ptr QuerySelectAll_;
		std::function<QList<T> ()> DoSelectAll_;

		QSqlQuery_ptr QueryInsertOne_;
		std::function<void (T&)> DoInsert_;

		QSqlQuery_ptr QueryUpdate_;
		std::function<void (T)> DoUpdate_;

		QSqlQuery_ptr QueryDelete_;
		std::function<void (T)> DoDelete_;

		detail::ByFieldsWrapper<T> DoSelectByFields_;

		QString CreateTable_;

		ObjectInfo ()
		{
		}

		ObjectInfo (decltype (QuerySelectAll_) sel, decltype (DoSelectAll_) doSel,
				decltype (QueryInsertOne_) insert, decltype (DoInsert_) doIns,
				decltype (QueryUpdate_) update, decltype (DoUpdate_) doUpdate,
				decltype (QueryDelete_) del, decltype (DoDelete_) doDelete,
				decltype (DoSelectByFields_) byFields,
				decltype (CreateTable_) createTable)
		: QuerySelectAll_ (sel)
		, DoSelectAll_ (doSel)
		, QueryInsertOne_ (insert)
		, DoInsert_ (doIns)
		, QueryUpdate_ (update)
		, DoUpdate_ (doUpdate)
		, QueryDelete_ (del)
		, DoDelete_ (doDelete)
		, DoSelectByFields_ (byFields)
		, CreateTable_ (createTable)
		{
		}
	};

	namespace ph
	{
		static const detail::ExprTree<detail::ExprType::LeafPlaceholder> _0 { 0 };
		static const detail::ExprTree<detail::ExprType::LeafPlaceholder> _1 { 1 };
		static const detail::ExprTree<detail::ExprType::LeafPlaceholder> _2 { 2 };
	}

	template<typename T>
	ObjectInfo<T> Adapt (const QSqlDatabase& db)
	{
		const QList<QString> fields = detail::GetFieldsNames<T> {} ();
		const QList<QString> boundFields = Map (fields, [] (const QString& str) -> QString { return ':' + str; });

		const auto& table = T::ClassName ();

		const detail::CachedFieldsData cachedData { table, db, fields, boundFields };
		const auto& selectPair = detail::AdaptSelectAll<T> (cachedData);
		const auto& insertPair = detail::AdaptInsert<T> (cachedData);
		const auto& updatePair = detail::AdaptUpdate<T> (cachedData);
		const auto& deletePair = detail::AdaptDelete<T> (cachedData);
		const auto& createTable = detail::AdaptCreateTable<T> (cachedData);

		const auto& byVal = detail::AdaptSelectFields<T> (cachedData);

		ObjectInfo<T> info
		{
			selectPair.first, selectPair.second,
			insertPair.first, insertPair.second,
			updatePair.first, updatePair.second,
			deletePair.first, deletePair.second,
			byVal,
			createTable
		};

		detail::AdaptSelectRef<T> (cachedData, info);

		return info;
	}
}
}
}
