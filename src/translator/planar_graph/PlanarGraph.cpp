#include <iostream>
#include <sstream>

#include "screen/Screen.h"
#include "translator/planar_graph/PlanarGraph.h"
#include "translator/planar_graph/PlanarGraphLexer.h"
#include "translator/planar_graph/PlanarGraphParser.h"

std::unique_ptr<Translator> PlanarGraphTranslator() {
  return std::make_unique<PlanarGraph>();
}

void PlanarGraph::Process(const std::string& input) {
  Read(input);
  Write();
}

void PlanarGraph::Read(const std::string& input) {
  antlr4::ANTLRInputStream input_stream(input);

  // Lexer.
  PlanarGraphLexer lexer(&input_stream);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  // for (auto token : tokens.getTokens()) {
  // std::cout << token->toString() << std::endl;
  //}

  // Parser.
  //output_ = parser.graph()->toStringTree(&parser);

  PlanarGraphParser parser(&tokens);
  ReadGraph(parser.graph());
}

void PlanarGraph::ReadGraph(PlanarGraphParser::GraphContext* graph) {
  for (PlanarGraphParser::EdgesContext* edges : graph->edges()) {
    ReadEdges(edges);
  }
}

void PlanarGraph::ReadEdges(PlanarGraphParser::EdgesContext* edges) {
  std::vector<int> nodes;
  std::vector<Arrow> arrows;
  for (PlanarGraphParser::NodeContext* node : edges->node()) {
    nodes.push_back(ReadNode(node));
  }
  for (PlanarGraphParser::ArrowContext* arrow : edges->arrow()) {
    arrows.push_back(ReadArrow(arrow));
  }
  for(int i = 0; i<arrows.size(); ++i) {
    vertex.push_back(Vertex{nodes[i], nodes[i+1], arrows[i]});
  }
}

int PlanarGraph::ReadNode(PlanarGraphParser::NodeContext* node) {
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

PlanarGraph::Arrow PlanarGraph::ReadArrow(PlanarGraphParser::ArrowContext *
                                          arrow) {
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

std::wstring PlanarGraph::ArrowToString(PlanarGraph::Arrow arrow) {
  switch(arrow) {
    case Arrow::RIGHT: return L"->";
    case Arrow::NONE: return L"--";
    case Arrow::LEFT_RIGHT: return L"<->";
    case Arrow::LEFT: return L"<-";
  }
}

void PlanarGraph::Write() {
  int width = 30;
  int height = 30;
  Screen screen(width, height);

  int y = 0;
  for(int i = 0; i<id_to_name.size(); ++i) {
    screen.DrawBoxedText(0,y,id_to_name[i]);
    y+=4;
  }
  output_ = screen.ToString();
}
