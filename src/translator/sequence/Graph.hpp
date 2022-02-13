// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <map>
#include <vector>
#include <set>

namespace graph {

struct Message {
  int id;    // message.
  int from;  // actor.
  int to;    // actor.
};

struct Node {
  int actor;
  int message;
};

struct Edge {
  Node from;
  Node to;

  Edge(const Node& from, const Node& to);
  Edge(const Message& message);
};

using Graph = std::set<Edge>;

bool operator<(const Node& a, const Node& b);
bool operator<(const Edge& a, const Edge& b);
std::vector<Node> FindTopologicalOrder(const Graph& graph);

}  // namespace graph
