// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "translator/sequence/Graph.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

namespace graph {

Edge::Edge(const Node& from, const Node& to) : from(from), to(to) {}
Edge::Edge(const Message& message)
    : from({message.from, message.id}), to({message.to, message.id}) {}

bool operator<(const Node& a, const Node& b) {
  if (a.actor < b.actor)
    return true;
  if (b.actor < a.actor)
    return false;
  return a.message < b.message;
}

bool operator<(const Edge& a, const Edge& b) {
  if (a.from < b.from)
    return true;
  if (b.from < a.from)
    return false;
  return a.to < b.to;
}

using Graph = std::set<Edge>;

std::vector<Node> FindTopologicalOrder(const Graph& graph) {
  std::map<Node, int> weight;
  bool work_to_do = true;
  int iteration = 0;
  while (work_to_do) {
    work_to_do = false;
    for (const auto& edge : graph) {
      if (weight[edge.to] <= weight[edge.from]) {
        weight[edge.to] = weight[edge.from] + 1;
        work_to_do = true;
      }
    }

    iteration++;
    if (iteration >= 1000) {
      std::cout << "There are cycles" << std::endl;
      break;
    }
  }

  std::vector<Node> nodes;
  for (auto& it : weight) {
    nodes.push_back(it.first);
  }
  std::sort(nodes.begin(), nodes.end(),
            [&weight](const Node& a, const Node& b) {
              return weight[a] < weight[b];
            });
  return nodes;
}

}  // namespace graph
