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

struct Vertex {
  Node from;
  Node to;

  Vertex(const Node& from, const Node& to);
  Vertex(const Message& message);
};

using Graph = std::set<Vertex>;

bool operator<(const Node& a, const Node& b);
bool operator<(const Vertex& a, const Vertex& b);
std::vector<Node> FindTopologicalOrder(const Graph& graph);

}  // namespace graph
