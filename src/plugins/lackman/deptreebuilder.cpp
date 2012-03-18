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

#include "deptreebuilder.h"
#include <boost/graph/visitors.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/topological_sort.hpp>
#include "core.h"

namespace LeechCraft
{
namespace LackMan
{
	DepTreeBuilder::VertexInfo::VertexInfo ()
	: IsFulfilled_ (false)
	, Type_ (TAny)
	{
	}

	DepTreeBuilder::VertexInfo::VertexInfo (DepTreeBuilder::VertexInfo::Type type)
	: IsFulfilled_ (false)
	, Type_ (type)
	{
	}

	DepTreeBuilder::VertexInfo::VertexInfo (int packageId)
	: PackageId_ (packageId)
	, IsFulfilled_ (false)
	, Type_ (TAll)
	{
	}

	DepTreeBuilder::VertexInfo::VertexInfo (const QString& depName)
	: Dependency_ (depName)
	, IsFulfilled_ (false)
	, Type_ (TAny)
	{
	}

	struct CycleDetector : public boost::default_dfs_visitor
	{
		QList<DepTreeBuilder::Edge_t>& BackEdges_;

		CycleDetector (QList<DepTreeBuilder::Edge_t>& be)
		: BackEdges_ (be)
		{
		}

		template<typename Edge, typename Graph>
		void back_edge (Edge edge, Graph&)
		{
			BackEdges_ << edge;
		}
	};

	struct FulfillableChecker : public boost::default_dfs_visitor
	{
		const QList<DepTreeBuilder::Vertex_t>& BackVertices_;
		const QList<DepTreeBuilder::Edge_t>& BackEdges_;
		const DepTreeBuilder::Edge2Vertices_t E2V_;
		DepTreeBuilder::Graph_t& G_;

		FulfillableChecker (const QList<DepTreeBuilder::Vertex_t>& bv,
				const QList<DepTreeBuilder::Edge_t>& be,
				const DepTreeBuilder::Edge2Vertices_t& e2v,
				DepTreeBuilder::Graph_t& g)
		: BackVertices_ (bv)
		, BackEdges_ (be)
		, E2V_ (e2v)
		, G_ (g)
		{
		}

		template<typename Vertex, typename Graph>
		void finish_vertex (Vertex u, Graph&)
		{
			bool hasBackEdge = BackVertices_.contains (u);
			if (hasBackEdge &&
					G_ [u].Type_ == DepTreeBuilder::VertexInfo::TAll)
			{
				G_ [u].IsFulfilled_ = false;
				return;
			}

			auto range = boost::out_edges (u, G_);
			switch (G_ [u].Type_)
			{
			case DepTreeBuilder::VertexInfo::TAll:
				G_ [u].IsFulfilled_ = true;
				for (auto i = range.first; 	i < range.second; ++i)
					if (!G_ [GetV (i)].IsFulfilled_)
					{
						G_ [u].IsFulfilled_ = false;
						break;
					}
				break;
			case DepTreeBuilder::VertexInfo::TAny:
				G_ [u].IsFulfilled_ = false;
				for (auto i = range.first; 	i < range.second; ++i)
				{
					if (BackEdges_.contains (*i))
						continue;

					if (G_ [GetV (i)].IsFulfilled_)
					{
						G_ [u].IsFulfilled_ = true;
						break;
					}
				}
				break;
			}
		}

		DepTreeBuilder::Vertex_t GetV (const DepTreeBuilder::OutEdgeIterator_t it)
		{
			return E2V_ [*it].second;
		}
	};

	struct VertexPredicate
	{
		DepTreeBuilder::Graph_t *G_;
		DepTreeBuilder::Edge2Vertices_t *E2V_;

		VertexPredicate ()
		: G_ (0)
		, E2V_ (0)
		{
		}

		VertexPredicate (DepTreeBuilder::Edge2Vertices_t& e2v,
				DepTreeBuilder::Graph_t& g)
		: G_ (&g)
		, E2V_ (&e2v)
		{
		}

		template<typename Vertex>
		bool operator() (const Vertex& v) const
		{
			/* If dependency is not fulfilled, we should not
				* see it in filtered output in any case.
				*/
			if (!(*G_) [v].IsFulfilled_)
				return false;

			/* If this dependency is of type TAny, then the
				* parent dependency is of type TAll, and we should
				* always see it if it's fulfilled (what we've
				* checked in previous condition).
				*/
			if ((*G_) [v].Type_ == DepTreeBuilder::VertexInfo::TAny)
				return true;

			/* This dependency is fulfilled, but is of type
				* TAll. Bad for us: we should step one level up and
				* check if there is any dependency (which would be
				* of type TAny) that lists this dependency as first
				* fulfillable.
				*
				* This way we leave only one fulfillable dependency.
				*
				* Just as a sidenote, there is little reason in
				* tying to being "first fulfillable": it'd be much
				* more sensible to check, for example, if we pull
				* the least possible amount of additional packages,
				* but that's too difficult.
				*/

			auto range = boost::in_edges (v, *G_);
			for (auto i = range.first;
					i < range.second; ++i)
			{
				Vertex u = (*E2V_) [*i].first;
				auto sameLevel = boost::out_edges (u, *G_);

				for (auto candIt = sameLevel.first; candIt < sameLevel.second; ++candIt)
				{
					Vertex candidate = (*E2V_) [*candIt].second;
					if ((*G_) [candidate].IsFulfilled_)
					{
						// If we're here, we're checking the
						// first fulfillable candidate.

						// The next if would succeed only if
						// first fulfillable is the Vertex we
						// are checking.
						if (candidate == v)
							return true;
						else
							break;
					}
				}
			}

			return false;
		}
	};

	DepTreeBuilder::DepTreeBuilder (int packageId)
	{
		// First, build the graph.
		Vertex_t root = boost::add_vertex (Graph_);
		Graph_ [root] = VertexInfo (packageId);
		Package2Vertex_ [packageId] = root;
		InnerLoop (packageId);

		// Second, find all the backedges.
		QList<Edge_t> backEdges;
		CycleDetector cd (backEdges);
		boost::depth_first_search (Graph_, boost::visitor (cd));

		// Prepare the list of those vertices that have back
		// edges coming from them.
		QList<Vertex_t> backVertices;
		Q_FOREACH (const Edge_t& edge, backEdges)
			backVertices << Edge2Vertices_ [edge].first;

		// Third, mark fulfillable/unfulfillable deps.
		FulfillableChecker checker (backVertices,
				backEdges,
				Edge2Vertices_,
				Graph_);
		boost::depth_first_search (Graph_, boost::visitor (checker));

		// Create filtered graph with only those that are
		// fulfilled.
		typedef boost::filtered_graph<Graph_t, boost::keep_all, VertexPredicate> fg_t;
		fg_t fg = fg_t (Graph_,
				boost::keep_all (),
				VertexPredicate (Edge2Vertices_, Graph_));

		// Finally run topological sort over filtered graph.
		QList<Vertex_t> vertices;
		boost::topological_sort (fg,
				std::front_inserter (vertices));
		Q_FOREACH (const Vertex_t& vertex, vertices)
			if (fg [vertex].Type_ == VertexInfo::TAll)
				PackagesToInstall_ << fg [vertex].PackageId_;
	}

	DepTreeBuilder::~DepTreeBuilder ()
	{
	}

	bool DepTreeBuilder::IsFulfilled () const
	{
		return Graph_ [*boost::vertices (Graph_).first].IsFulfilled_;
	}

	QStringList DepTreeBuilder::GetUnfulfilled () const
	{
		if (IsFulfilled ())
			return QStringList ();

		QStringList result;
		auto range = boost::out_edges (*boost::vertices (Graph_).first, Graph_);
		for (auto i = range.first; i < range.second; ++i)
		{
			Vertex_t vertex = Edge2Vertices_ [*i].second;
			if (!Graph_ [vertex].IsFulfilled_)
				result << Graph_ [vertex].Dependency_;
		}

		return result;
	}

	const QList<int>& DepTreeBuilder::GetPackagesToInstall () const
	{
		return PackagesToInstall_;
	}

	void DepTreeBuilder::InnerLoop (int packageId)
	{
		const auto& dependencies = Core::Instance ().GetDependencies (packageId);

		Q_FOREACH (const Dependency& dep, dependencies)
		{
			if (Core::Instance ().IsFulfilled (dep))
				continue;

			Vertex_t depVertex;
			if (!Dependency2Vertex_.contains (dep))
			{
				depVertex = boost::add_vertex (Graph_);
				Graph_ [depVertex] = VertexInfo (dep.Name_);

				Dependency2Vertex_ [dep] = depVertex;
			}
			else
				depVertex = Dependency2Vertex_ [dep];

			Vertex_t packageVertex = Package2Vertex_ [packageId];
			Edge_t edge = boost::add_edge (packageVertex, depVertex, Graph_).first;
			Edge2Vertices_ [edge] = qMakePair (packageVertex, depVertex);

			const auto& suitable = Core::Instance ().GetDependencyFulfillers (dep);

			Q_FOREACH (const ListPackageInfo& lpi, suitable)
			{
				Vertex_t ffVertex;
				if (!Package2Vertex_.contains (lpi.PackageID_))
				{
					ffVertex = boost::add_vertex (Graph_);
					Graph_ [ffVertex] = VertexInfo (lpi.PackageID_);

					Package2Vertex_ [lpi.PackageID_] = ffVertex;

					InnerLoop (lpi.PackageID_);
				}
				else
					ffVertex = Package2Vertex_ [lpi.PackageID_];

				Edge_t edge = boost::add_edge (depVertex, ffVertex, Graph_).first;
				Edge2Vertices_ [edge] = qMakePair (depVertex, ffVertex);
			}
		}
	}
}
}
