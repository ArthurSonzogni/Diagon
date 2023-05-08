// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <cassert>
#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
#include "screen/Screen.h"
#include "translator/Translator.h"
#include "translator/antlr_error_listener.h"
#include "translator/graph_planar/GraphPlanarLexer.h"
#include "translator/graph_planar/GraphPlanarParser.h"

// With emscripten, you may need to execute in the src directory.
// # ln -s /usr/include/boost/ boost
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/chrobak_payne_drawing.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/make_biconnected_planar.hpp>
#include <boost/graph/make_connected.hpp>
#include <boost/graph/make_maximal_planar.hpp>
#include <boost/graph/planar_canonical_ordering.hpp>
#include <boost/graph/planar_detail/add_edge_visitors.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>

using Graph = boost::adjacency_list<boost::vecS,
                                    boost::vecS,
                                    boost::undirectedS,
                                    boost::property<boost::vertex_index_t, int>,
                                    boost::property<boost::edge_index_t, int>>;
using EdgeT = boost::graph_traits<Graph>::edge_descriptor;
using EdgePermutationStorage = std::vector<std::vector<EdgeT>>;
using EdgePermutation = boost::iterator_property_map<
    EdgePermutationStorage::iterator,
    boost::property_map<Graph, boost::vertex_index_t>::type>;
struct Coordinates {
  size_t x;
  size_t y;
};
using StraightLineDrawing = boost::iterator_property_map<
    std::vector<Coordinates>::iterator,
    boost::property_map<Graph, boost::vertex_index_t>::type>;

struct Box {
  int left;
  int right;
  int top;
  int bottom;
  static Box Union(Box A, Box B);
  static Box Translate(Box A, int x, int y);
};

class GraphPlanar;

struct DrawnEdge {
  int x;
  int vertex_up;
  int vertex_down;
  int y_up;
  int y_down;

  void Draw(Screen& screen, GraphPlanar& graph);
};

struct DrawnVertex {
  int left;
  int right;
  int y;
  std::wstring text;
  std::vector<DrawnEdge> edges;

  void Draw(Screen& screen);
};

enum class Arrow {
  RIGHT,
  LEFT_RIGHT,
  NONE,
  LEFT,
};

struct Edge {
  int from;
  int to;
  Arrow arrow;
};

enum class ArrowStyle {
  NONE,
  LINE,
  ARROW,
};

struct Node {
  int id;
  std::vector<Node> childs;
};

class GraphPlanar : public Translator {
 public:
  virtual ~GraphPlanar() = default;
  std::map<int, std::map<int, ArrowStyle>> arrow_style;

 private:
  const char* Name() final { return "Graph - planar"; }
  const char* Identifier() final { return "GraphPlanar"; }
  const char* Description() final {
    return "Build a graph from node and edges";
  }
  std::vector<Translator::OptionDescription> Options() final;
  std::vector<Translator::Example> Examples() final;
  std::string Translate(const std::string& input,
                        const std::string& options_string) final;
  std::string Highlight(const std::string& input) final;

  void Read(const std::string& input);
  void ReadGraph(GraphPlanarParser::GraphContext* graph);
  void ReadEdges(GraphPlanarParser::EdgesContext* edges);
  int ReadNode(GraphPlanarParser::NodeContext* node);
  Arrow ReadArrow(GraphPlanarParser::ArrowContext* arrow);

  void Write();
  void ComputeArrowStyle();

  bool ascii_only_;
  std::string output_;

  std::map<std::wstring, int> name_to_id;
  std::vector<std::wstring> id_to_name;
  int next_id = 0;

  std::vector<Edge> vertex_;
};

std::vector<Translator::OptionDescription> GraphPlanar::Options() {
  return {
      {
          "ascii_only",
          {
              "false",
              "true",
          },
          "false",
          "Use the full unicode charset or only ASCII.",
          Widget::Checkbox,
      },
  };
}

std::vector<Translator::Example> GraphPlanar::Examples() {
  return {
      {
          "if then else loop",
          "if -> \"then A\" -> end\n"
          "if -> \"then B\" -> end\n"
          "end -> loop -> if",
      },
      {
          "test",
          "A -- B\n"
          "A -- C\n"
          "A -- D -- G\n"
          "B -- Z\n"
          "C -- Z",
      },
  };
}

std::string GraphPlanar::Translate(const std::string& input,
                                   const std::string& options_string) {
  *this = GraphPlanar();
  auto options = SerializeOption(options_string);
  ascii_only_ = (options["ascii_only"] == "true");

  Read(input);
  Write();
  return output_;
}

void GraphPlanar::Read(const std::string& input) {
  antlr4::ANTLRInputStream input_stream(input);

  // Lexer.
  GraphPlanarLexer lexer(&input_stream);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  // Parser:
  AntlrErrorListener error_listener;
  GraphPlanarParser parser(&tokens);
  parser.addErrorListener(&error_listener);
  GraphPlanarParser::GraphContext* context = nullptr;
  try {
    context = parser.graph();
  } catch (...) {
    return;
  }

  ReadGraph(context);
}

void GraphPlanar::ReadGraph(GraphPlanarParser::GraphContext* graph) {
  for (GraphPlanarParser::EdgesContext* edges : graph->edges()) {
    ReadEdges(edges);
  }
}

void GraphPlanar::ReadEdges(GraphPlanarParser::EdgesContext* edges) {
  std::vector<int> nodes;
  std::vector<Arrow> arrows;
  for (GraphPlanarParser::NodeContext* node : edges->node()) {
    nodes.push_back(ReadNode(node));
  }
  for (GraphPlanarParser::ArrowContext* arrow : edges->arrow()) {
    arrows.push_back(ReadArrow(arrow));
  }
  for (int i = 0; i < arrows.size(); ++i) {
    vertex_.push_back(Edge{nodes[i], nodes[i + 1], arrows[i]});
  }
}

int GraphPlanar::ReadNode(GraphPlanarParser::NodeContext* node) {
  std::wstring label;
  if (auto id = node->ID()) {
    label = to_wstring(id->getSymbol()->getText());
  } else if (auto text = node->STRING()) {
    label = to_wstring(text->getSymbol()->getText());
    // Remove the two '"'.
    label = label.substr(1, label.size() - 2);
  } else {
    // NOTREACHED();
  }

  auto it = name_to_id.find(label);
  if (it != name_to_id.end()) {
    return it->second;
  } else {
    name_to_id[label] = next_id;
    id_to_name.push_back(label);
    return next_id++;
  }
}

Arrow GraphPlanar::ReadArrow(GraphPlanarParser::ArrowContext* arrow) {
  if (arrow->RIGHT_ARROW())
    return Arrow::RIGHT;
  if (arrow->NONE_ARROW())
    return Arrow::NONE;
  if (arrow->LEFT_RIGHT_ARROW())
    return Arrow::LEFT_RIGHT;
  if (arrow->LEFT_ARROW())
    return Arrow::LEFT;
  // NOTREACHED();
  return Arrow::RIGHT;
}

void GraphPlanar::ComputeArrowStyle() {
  // Compute ArrowStyle
  for (auto& v : vertex_) {
    switch (v.arrow) {
      case Arrow::RIGHT:
        arrow_style[v.from][v.to] = ArrowStyle::ARROW;
        arrow_style[v.to][v.from] = ArrowStyle::LINE;
        break;
      case Arrow::LEFT_RIGHT:
        arrow_style[v.from][v.to] = ArrowStyle::ARROW;
        arrow_style[v.to][v.from] = ArrowStyle::ARROW;
        break;
      case Arrow::LEFT:
        arrow_style[v.from][v.to] = ArrowStyle::LINE;
        arrow_style[v.to][v.from] = ArrowStyle::ARROW;
        break;
      case Arrow::NONE:
        arrow_style[v.from][v.to] = ArrowStyle::LINE;
        arrow_style[v.to][v.from] = ArrowStyle::LINE;
        break;
    }
  }
}

/// @brief Compute the planar embedding of a graph.
///
/// @param graph The graph to embed.
/// @param embedding_storage The storage for the embedding.
/// @param embedding The embedding.
///
/// @return True if the graph is planar.
bool PlanarEmbedding(const Graph& graph,
                     EdgePermutationStorage& embedding_storage,
                     EdgePermutation& embedding) {
  embedding_storage = EdgePermutationStorage(boost::num_vertices(graph));
  embedding = EdgePermutation(embedding_storage.begin(),
                              boost::get(boost::vertex_index, graph));
  const bool is_planar = boyer_myrvold_planarity_test(
      boost::boyer_myrvold_params::graph = graph,
      boost::boyer_myrvold_params::embedding = embedding);
  return is_planar;
}

void GraphPlanar::Write() {
  ComputeArrowStyle();

  if (id_to_name.size() <= 2) {
    output_ = "Graph contains less than 3 edges.\n";
    return;
  }

  const int num_vertices = id_to_name.size();

  // Create a graph.
  Graph graph(num_vertices);
  for (auto& it : vertex_) {
    // Check if the edge already exists. Apparently, boost graph do not support
    // it.
    if (boost::edge(it.from, it.to, graph).second) {
      continue;
    }

    add_edge(it.from, it.to, graph);
  }
  auto edge_index = get(boost::edge_index, graph);
  auto vertex_index = get(boost::vertex_index, graph);

  // The `edge_updater` is used to automatically update the edge index when a
  // new edge is added.
  using EdgeVisitor = boost::edge_index_update_visitor<
      boost::property_map<Graph, boost::edge_index_t>::type>;
  EdgeVisitor edge_updater(edge_index, boost::num_edges(graph));

  // Initialize the edge index.
  int edge_count = 0;
  auto [ei, ei_end] = edges(graph);
  while (ei != ei_end) {
    boost::put(edge_index, *(ei++), edge_count++);
  }

  // To apply the `chrobak_payne_straight_line_drawing` algorithm, we need to
  // add edges so that the graph is maximal planar. This can be achieve by
  // executing in sequence:
  // - `make_connected`
  // - `make_biconnected_planar`
  // - `make_maximal_planar`

  // After executing `make_connected`, edges are added to the graph, so we must
  // update the indexes and the embedding.
  boost::make_connected(graph, vertex_index, edge_updater);

  // Create the planar embedding
  auto embedding_storage = EdgePermutationStorage(num_vertices);
  auto embedding = EdgePermutation(embedding_storage.begin(), vertex_index);
  const bool is_planar_1 = PlanarEmbedding(graph, embedding_storage, embedding);
  if (!is_planar_1) {
    output_ = "Graph is not planar.\n";
    return;
  }

  // After executing `make_biconnected_planar` edges are added to the graph, so
  // we must update the indexes and the embedding.
  boost::make_biconnected_planar(
      graph, embedding, boost::get(boost::edge_index, graph), edge_updater);
  const bool is_planar_2 = PlanarEmbedding(graph, embedding_storage, embedding);
  assert(is_planar_2);
  std::ignore = is_planar_2;

  // After executing `make_maximal_planar` edges are added to the graph, so
  // we must update the indexes and the embedding.
  boost::make_maximal_planar(graph, embedding, vertex_index,
                             boost::get(boost::edge_index, graph),
                             edge_updater);
  const bool is_planar_3 = PlanarEmbedding(graph, embedding_storage, embedding);
  assert(is_planar_3);
  std::ignore = is_planar_3;

  // Find a canonical ordering.
  std::vector<size_t> ordering;
  boost::planar_canonical_ordering(graph, embedding,
                                   std::back_inserter(ordering));
  assert(ordering.size() == num_vertices);

  std::vector<Coordinates> straight_line_drawing_storage(num_vertices);
  auto drawing =
      StraightLineDrawing(straight_line_drawing_storage.begin(), vertex_index);

  // Compute the straight line drawing
  chrobak_payne_straight_line_drawing(graph, embedding, ordering.begin(),
                                      ordering.end(), drawing);

  auto compare_with = [&](int i) {
    return [&, i](int a, int b) {
      int a_dx = drawing[a].x - drawing[i].x;
      int a_dy = drawing[a].y - drawing[i].y;
      int b_dx = drawing[b].x - drawing[i].x;
      int b_dy = drawing[b].y - drawing[i].y;
      return a_dx * b_dy - b_dx * a_dy < 0;
    };
  };

  std::vector<size_t> inverse_ordering(num_vertices);
  for (int i = 0; i < inverse_ordering.size(); ++i) {
    inverse_ordering[ordering[i]] = i;
  }

  // Compute children.
  std::vector<std::vector<int>> children(num_vertices);
  for (int i = 0; i < num_vertices; ++i) {
    auto& down = children[i];
    auto adjacent = boost::adjacent_vertices(i, graph);
    for (auto j = adjacent.first; j != adjacent.second; ++j) {
      if (inverse_ordering[i] < inverse_ordering[*j]) {
        down.push_back(*j);
      }
    }
    std::sort(down.begin(), down.end(), compare_with(i));
  }

  // Compute Y.
  std::vector<size_t> y(num_vertices, 0);
  for (size_t i : ordering) {
    for (size_t j : children[i]) {
      if (inverse_ordering[i] < inverse_ordering[j]) {
        y[j] = std::max(y[j], y[i] + 1);
      }
    }
  }

  std::vector<DrawnVertex> drawn_vertices(num_vertices);
  std::vector<bool> is_drawn(num_vertices, false);
  std::vector<int> x(y.size(), -1);

  std::function<void(int)> refresh_x = [&](int y) {
    int i = y + 1;
    while (i < x.size() && x[i] < x[y]) {
      x[i] = x[y];
      ++i;
    };
  };

  std::function<void(int)> DrawNode = [&](int i) -> void {
    if (is_drawn[i])
      return;
    DrawnVertex& vertex = drawn_vertices[i];
    int child_left = -1;
    int child_right = -1;
    // Draw our childrens.
    for (auto j : children[i]) {
      DrawNode(j);

      auto& child = drawn_vertices[j];
      DrawnEdge edge;
      edge.vertex_up = i;
      edge.vertex_down = j;
      edge.y_up = y[i] + 1;
      edge.y_down = y[j] - 1;

      // There are no real edge between those two vertex. in this case, do not
      // draw anything.
      if (arrow_style[i][j] == ArrowStyle::NONE) {
        continue;
      }

      // Make the child bigger if we need to.
      child.right = std::max(child.right, x[y[j] - 1] + 3);
      x[y[j]] = child.right;
      refresh_x(y[j]);

      edge.x = child.left + 1;
      edge.x = std::max(edge.x, x[y[i]] + 2);
      if (y[j] != y[i] + 1) {
        edge.x = std::max(edge.x, x[child.y - 1] + 1);
      }
      vertex.edges.push_back(edge);

      x[y[i] + 1] = std::max(x[y[i] + 1], edge.x);
      refresh_x(y[i] + 1);

      if (child_left == -1) {
        child_left = edge.x - 1;
      }
      child_right = edge.x + 1;
    }

    // Draw ourself.
    vertex.text = id_to_name[i];
    vertex.y = y[i];
    vertex.left = std::max(0, std::max(child_left, x[y[i]] + 1));
    vertex.right =
        std::max(child_right, (int)(vertex.left + vertex.text.size() + 1));
    x[y[i]] = vertex.right;
    refresh_x(y[i]);

    // Mark ourself as drawn.
    is_drawn[i] = true;
  };

  for (int i : ordering) {
    if (y[i] == 0) {
      DrawNode(i);
    }
  }

  // Determine the screen dimension
  int width = 0;
  int height = 0;
  for (int i = 0; i < num_vertices; ++i) {
    if (!is_drawn[i])
      continue;
    width = std::max(width, drawn_vertices[i].right + 1);
    height = std::max(height, 3 * drawn_vertices[i].y + 3);
  }

  Screen screen(width, height);
  for (int i = 0; i < num_vertices; ++i) {
    if (!is_drawn[i])
      continue;
    drawn_vertices[i].Draw(screen);
  }
  for (int i = 0; i < num_vertices; ++i) {
    if (!is_drawn[i])
      continue;
    for (auto& edge : drawn_vertices[i].edges) {
      edge.Draw(screen, *this);
    }
  }

  if (ascii_only_)
    screen.ASCIIfy(1);
  output_ += screen.ToString();
}

void DrawnVertex::Draw(Screen& screen) {
  screen.DrawBox(left, 3 * y, right - left + 1, 3);
  int text_position = left + 1 + (right - left - 1 - text.size()) / 2;
  screen.DrawText(text_position, 3 * y + 1, text);
}

void DrawnEdge::Draw(Screen& screen, GraphPlanar& graph) {
  int top = 3 * y_up - 1;
  int bottom = 3 * y_down + 3;

  screen.DrawVerticalLine(top + 1, bottom - 1, x);

  if (graph.arrow_style[vertex_down][vertex_up] == ArrowStyle::LINE)
    screen.DrawPixel(x, top, L'┬');
  else
    screen.DrawPixel(x, top, L'△');

  if (graph.arrow_style[vertex_up][vertex_down] == ArrowStyle::LINE)
    screen.DrawPixel(x, bottom, L'┴');
  else
    screen.DrawPixel(x, bottom, L'▽');
}

std::string GraphPlanar::Highlight(const std::string& input) {
  std::stringstream out;

  antlr4::ANTLRInputStream input_stream(input);

  // Lexer.
  GraphPlanarLexer lexer(&input_stream);
  antlr4::CommonTokenStream tokens(&lexer);

  try {
    tokens.fill();
  } catch (...) {  // Ignore
  }

  size_t matched = 0;
  out << "<span class='GraphPlanar'>";
  for (antlr4::Token* token : tokens.getTokens()) {
    std::string text = token->getText();
    if (text == "<EOF>") {
      continue;
    }
    out << "<span class='";
    out << lexer.getVocabulary().getSymbolicName(token->getType());
    out << "'>";
    matched += text.size();
    out << std::move(text);
    out << "</span>";
  }

  out << input.substr(matched);
  out << "</span>";

  return out.str();
}

std::unique_ptr<Translator> GraphPlanarTranslator() {
  return std::make_unique<GraphPlanar>();
}
