#include "sequence/SequenceImpl.h"

#include <queue>
#include <set>
#include <sstream>
#include <functional>

#include "sequence/SequenceLexer.h"
#include "sequence/SequenceParser.h"
#include "screen/Screen.h"

bool SequenceImpl::Dependency::operator<(
    const SequenceImpl::Dependency& other) const {
  if (from < other.from)
    return true;
  if (from > other.from)
    return false;
  return to < other.to;
}

void SequenceImpl::Process(const std::string& input) {
  ComputeInternalRepresentation(input);
  UniformizeInternalRepresentation();
  Layout();
  Draw();
}

void SequenceImpl::ComputeInternalRepresentation(const std::string& input) {
  std::cout << "input = " << std::endl << input << std::endl;
  antlr4::ANTLRInputStream input_stream(input);

  // Lexer.
  SequenceLexer lexer(&input_stream);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  for (auto token : tokens.getTokens()) {
    std::cout << token->toString() << std::endl;
  }

  // Parser.
  SequenceParser parser(&tokens);

  // Print the tree.
  auto program = parser.program();
  std::cout << program->toStringTree(&parser) << std::endl << std::endl;

  for (SequenceParser::CommandContext* command : program->command()) {
    AddCommand(command);
  }
}

void SequenceImpl::UniformizeInternalRepresentation() {
  UniformizeActors();
  UniformizeMessageID();
}

void SequenceImpl::UniformizeActors() {
  // Look at missing Actors.
  std::set<std::wstring> declared_actors;
  for (auto& actor : actors) {
    declared_actors.insert(actor.name);
  }
  // Add the missing Actors.
  for (auto& message : messages) {
    for (auto& actor : {message.from, message.to}) {
      if (!actor_index.count(actor)) {
        actor_index[actor] = actors.size();
        Actor a;
        a.name = actor;
        actors.push_back(a);
      }
    }
  }
}

void SequenceImpl::UniformizeMessageID() {
  // Remove duplicate in message.id
  {
    std::set<int> used;
    for (auto& message : messages) {
      if (message.id != -1) {
        if (used.count(message.id)) {
          message.id = -1;
          // TODO(arthursonzogni): Add warning.
        } else {
          used.insert(message.id);
        }
      }
    }
  }

  // Remove the actors.id that are for message that do not include the author.
  {
    std::map<int, int> message_index;
    for (int i = 0; i < messages.size(); ++i) {
      if (messages[i].id != -1) {
        message_index[messages[i].id] = i;
      }
    }
    for (Actor& actor : actors) {
      const auto is_dependency_invalid =
          [&message_index, this, &actor](const Dependency& dependency) -> bool {
        for (int id : {dependency.from, dependency.to}) {
          auto it = message_index.find(id);
          if (it == message_index.end()) {
            return true;
          }

          const Message& message = messages[it->second];
          if (actor.name != message.from &&  //
              actor.name != message.to) {
            return true;
          }
        }
        return false;
      };
      auto it = actor.dependencies.begin();
      while (it != actor.dependencies.end()) {
        if (is_dependency_invalid(*it)) {
          it = actor.dependencies.erase(it);
        } else {
          ++it;
        }
      }
    }
  }

  // Assign an ID to the messages with no ID.
  {
    int max_id = -1;
    for (const auto& it : messages) {
      max_id = std::max(max_id, it.id);
    }
    for (auto& it : messages) {
      if (it.id == -1) {
        it.id = ++max_id;
      }
    }
  }

  message_index.clear();
  for (int i = 0; i < messages.size(); ++i) {
    if (messages[i].id != -1) {
      message_index[messages[i].id] = i;
    }
  }
}

void SequenceImpl::AddCommand(SequenceParser::CommandContext* command) {
  if (auto message = command->message()) {
    AddMessage(message);
    return;
  }

  if (auto actor = command->actor()) {
    AddActor(actor);
    return;
  }
}

void SequenceImpl::AddMessage(SequenceParser::MessageContext* message_context) {
  Message message;
  if (auto message_id = message_context->messageID()) {
    message.id = GetMessageID(message_id);
  }

  message.from = to_wstring(message_context->Words(0)->getSymbol()->getText());
  message.to = to_wstring(message_context->Words(1)->getSymbol()->getText());

  if (message_context->arrow()->NormalLeftArrow()) {
    std::swap(message.from, message.to);
  }

  message.messages = GetMessageText(message_context->messageText());
  messages.push_back(message);
}

void SequenceImpl::AddActor(SequenceParser::ActorContext* actor_context) {
  std::wstring name =
      to_wstring(actor_context->Words()->getSymbol()->getText());
  if (!actor_index.count(name)) {
    actor_index[name] = actors.size();
    actors.emplace_back();
  }
  Actor& actor = actors[actor_index[name]];
  actor.name = name;

  std::vector<int> numbers;
  for (const auto& number : actor_context->Number()) {
    numbers.push_back(std::stoi(number->getSymbol()->getText()));
  }
  for (int i = 0, j = 1; j < numbers.size(); ++i, ++j) {
    actor.dependencies.insert(Dependency{numbers[i], numbers[j]});
  }
}

int SequenceImpl::GetMessageID(SequenceParser::MessageIDContext* message_id) {
  return std::stoi(message_id->Number()->getSymbol()->getText());
}

std::vector<std::wstring> SequenceImpl::GetMessageText(
    SequenceParser::MessageTextContext* message_text) {
  std::vector<std::wstring> messages;
  if (message_text->getRuleIndex() == 0) {
    messages.push_back(to_wstring(
        static_cast<SequenceParser::SingleLineTextContext*>(message_text)
            ->Words()
            ->getSymbol()
            ->getText()));
  } else {
    auto m = static_cast<SequenceParser::MultiLineTextContext*>(message_text);
    for (const auto& line : m->Words()) {
      messages.push_back(to_wstring(line->getSymbol()->getText()));
    }
  }
  return messages;
}

void SequenceImpl::Layout() {
  LayoutComputeMessageWidth();
  LayoutComputeActorsPositions();
  LayoutComputeMessagesPositions();
}

void SequenceImpl::LayoutComputeMessageWidth() {
  for (auto& message : messages) {
    for (auto& text : message.messages) {
      message.width = std::max(message.width, int(text.size()));
    }
  }
}

// Define minimum space between two actors.
struct ActorSpace {
  int a;
  int b;
  int space;
};

void SequenceImpl::LayoutComputeActorsPositions() {
  std::vector<ActorSpace> spaces;
  for (int i = 0, j = 1; j < actors.size(); ++i, ++j) {
    int size_1 = actors[i].name.size();
    int size_2 = actors[j].name.size();
    spaces.push_back({
        i,                                        //
        i + 1,                                    //
        size_1 / 2 + size_2 / 2 + size_2 % 2 + 2  //
    });
  }

  for (auto& message : messages) {
    std::cout << "from = " << to_string(message.from) << " to "
              << to_string(message.to) << std::endl;
    std::cout << "from = " << actor_index[message.from] << " to "
              << actor_index[message.to] << std::endl;
    ActorSpace space{actor_index[message.from],  //
                     actor_index[message.to],    //
                     message.width + 1};         //
    if (space.a > space.b)
      std::swap(space.a, space.b);
    spaces.push_back(space);
  }

  std::cout << "spaces.size()  = " << spaces.size() << std::endl;
  for (auto& it : spaces) {
    std::cout << " * " << it.a;
    std::cout << " , " << it.b;
    std::cout << " , " << it.space;
    std::cout << std::endl;
  }

  actors[0].center = actors[0].name.size() / 2 + 1;

  bool modified = true;
  int i = 0;
  while (modified) {
    modified = false;
    for (const ActorSpace& s : spaces) {
      if (actors[s.b].center - actors[s.a].center < s.space) {
        actors[s.b].center = actors[s.a].center + s.space;
        modified = true;
      }
    }
    if (i++ > 500)
      break;
  }

  for (auto& actor : actors) {
    actor.left = actor.center - actor.name.size() / 2 - 1;
    actor.right = actor.left + actor.name.size() + 2;
  }
}

namespace graph {

struct Message {
  int id;    // message.
  int from;  // actor.
  int to;    // actor.
};

using Dependency = SequenceImpl::Dependency;

struct Node {
  int actor;
  int message;

  Node(int actor, int message) : actor(actor), message(message){};
};

struct Vertex {
  Node from;
  Node to;

  Vertex(const Node& from, const Node& to) : from(from), to(to) {}
  Vertex(const Message& message)
      : from(Node(message.from, message.id)),
        to(Node(message.to, message.id)) {}
};

using Graph = std::set<Vertex>;

bool operator<(const Node& a, const Node& b) {
  if (a.actor < b.actor)
    return true;
  if (b.actor < a.actor)
    return false;
  return a.message < b.message;
}

bool operator<(const Vertex& a, const Vertex& b) {
  if (a.from < b.from)
    return true;
  if (b.from < a.from)
    return false;
  return a.to < b.to;
}

struct MessageDependencies {
  std::set<int> messages;
  std::set<Dependency> dependencies;
};

struct MessageSetWithWeight {
  std::set<int> messages;
  size_t weight;
  bool operator<(const MessageSetWithWeight& other) const {
    return weight > other.weight;
  }
};

std::vector<std::set<int>> Cut(const MessageDependencies& message_dependencies,
                               std::function<bool(int, int)> preference) {
  std::vector<std::set<int>> output;

  // Groups the nodes that independants on each other.
  std::vector<MessageDependencies> independants;
  {
    std::map<int, std::set<int>> neighbours;
    for (const Dependency& dependency : message_dependencies.dependencies) {
      neighbours[dependency.from].insert(dependency.to);
      neighbours[dependency.to].insert(dependency.from);
    }
    std::set<int> non_used;
    for (int message : message_dependencies.messages) {
      non_used.insert(message);
    }
    while (!non_used.empty()) {
      MessageDependencies dependant;
      std::queue<int> tasks;
      tasks.push(*non_used.begin());
      while (!tasks.empty()) {
        // Take a new task
        int message = tasks.front();
        tasks.pop();

        // Check there is work to do for this task.
        {
          auto it = non_used.find(message);
          if (it == non_used.end())
            continue;
          non_used.erase(it);
        }

        dependant.messages.insert(message);
        for (int next : neighbours[message]) {
          tasks.push(next);
        }
      }
      independants.push_back(std::move(dependant));
    }

    // Split the dependencies between each independant groups.
    std::map<int, int> index;
    for (int i = 0; i < independants.size(); ++i) {
      for (int j : independants[i].messages)
        index[j] = i;
    }
    for (const Dependency& dependency : message_dependencies.dependencies) {
      independants[index[dependency.from]].dependencies.insert(dependency);
    }

    // Sort indepents groups such that messages are displayed in the order they
    // are written.
    std::sort(
        independants.begin(), independants.end(),
        [&](const MessageDependencies left, const MessageDependencies right) {
          return preference(*left.messages.begin(), *right.messages.begin());
        });
  }

  std::cout << "independants.size() = " << independants.size() << std::endl;

  for (auto& dependant : independants) {
    // Find cycles.
    std::vector<MessageSetWithWeight> cycles;
    {
      // Compute initial state.
      std::map<int, std::set<int>> reachable_from;
      for (int message : dependant.messages) {
        reachable_from[message].insert(message);
      }
      std::map<int, std::set<int>> new_reachable_from(reachable_from);

      // Find the closure.
      while (true) {
        for (const Dependency& dependency : dependant.dependencies) {
          const auto& addition = reachable_from[dependency.to];
          new_reachable_from[dependency.from].insert(addition.cbegin(),
                                                     addition.cend());
        }

        if (reachable_from == new_reachable_from)
          break;
        reachable_from = new_reachable_from;
      }

      // Group elements that can reach the same set of elements.
      std::map<std::set<int>, std::set<int>> groups;
      for (const auto& it : reachable_from) {
        groups[it.second].insert(it.first);
      }

      for (auto& group : groups) {
        cycles.push_back(
            MessageSetWithWeight{std::move(group.second), group.first.size()});
      }

      // This is implicit, but it is indeed a topological sort.
      sort(cycles.begin(), cycles.end());
    }

    std::cout << "cycles.size() = " << cycles.size() << std::endl;
    for (auto& it : cycles)
      output.push_back(std::move(it.messages));
  }

  // Separate cycles th
  
  // std::set<Dependency> external_dependencies;
  // for(const Dependency& dependency : message_dependencies.dependencies) {
  // bool found = false;
  // for(MessageDependencies& cycle : output) {
  // if (cycle.messages.count(dependency.from) &&
  // cycle.messages.count(dependency.to)) {
  // cycle.dependencies.insert(dependency);
  // found = true;
  // break;
  //}
  //}
  // if (!found) {
  // external_dependencies.insert(dependency);
  //}
  //}

  // TODO: sort the cycles.

  return output;
}

std::vector<Node> FindTopologicalOrder(const Graph& graph) {
  std::map<Node, int> weight;
  bool work_to_do = true;
  int iteration = 0;
  while (work_to_do) {
    work_to_do = false;
    for (const auto& vertex : graph) {
      if (weight[vertex.to] <= weight[vertex.from]) {
        weight[vertex.to] = weight[vertex.from] + 1;
        work_to_do = true;
      }
    }

    iteration++;
    if (iteration >= 1000)
      break;
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

}  // namespace Graph

void SequenceImpl::LayoutComputeMessagesPositions() {
  // Build graph
  graph::MessageDependencies message_dependencies;
  for (int a = 0; a < actors.size(); ++a) {
    auto& actor = actors[a];
    for (const Dependency& dependency : actor.dependencies) {
      message_dependencies.dependencies.insert(dependency);
    }
    for (const Message& message : messages) {
      message_dependencies.messages.insert(message.id);
    }
  }

  int y = 4;

  auto add_message = [&](Message& message, int& y, int& offset) {
    message.left = actors[actor_index[message.from]].center;
    message.right = actors[actor_index[message.to]].center;
    if (message.left > message.right) {
      std::swap(message.left, message.right);
      message.direction = Message::Direction::Left;
    } else {
      message.direction = Message::Direction::Right;
    }
    message.left += 1;
    message.right -= 1;
    message.line_left = message.left;
    message.line_right = message.right;
    int k = 0;
    while (message.right - message.left > message.width) {
      if (++k % 2)
        message.left++;
      else
        message.right--;
    }

    message.top = y;
    message.bottom = message.top + message.messages.size();
    message.line_bottom = message.bottom;

    if (message.is_separated) {
      message.offset = offset++;
    } else {
      message.line_top = message.bottom;
    }
    y = message.bottom + 2;
  };

  auto preference = [&](int a, int b) -> bool {
    return message_index[a] < message_index[b];
  };

  for (auto& cut : graph::Cut(message_dependencies, preference)) {
    int offset = 1;

    // Fast path: Only one message, no crossing.
    if (cut.size() == 1) {
      int id = *(cut.begin());
      auto& message = messages[message_index[id]];
      add_message(message, y, offset);
      continue;
    }

    // Build graph;
    graph::Graph graph;
    for (int a = 0; a < actors.size(); ++a) {
      auto& actor = actors[a];
      for (const Dependency& dependency : actor.dependencies) {
        if (cut.count(dependency.from) && cut.count(dependency.to)) {
          graph.insert(graph::Vertex(graph::Node(a, dependency.from),  //
                                     graph::Node(a, dependency.to)));
        }
      }
    }

    for (auto& message : messages) {
      if (cut.count(message.id)) {
        graph::Node from{actor_index[message.from], message.id};
        graph::Node to{actor_index[message.to], message.id};
        graph.insert(graph::Vertex(from, to));
      }
    }
    std::set<int> started_message;
    auto topological_order = FindTopologicalOrder(graph);
    for (int i = 0; i < topological_order.size(); ++i) {
      const graph::Node& node = topological_order[i];
      auto& message = messages[message_index[node.message]];

      bool is_separated = true;
      if (started_message.count(node.message) == 0) {
        started_message.insert(node.message);
        if (i + 1 < 0 ||
            topological_order[i + 1].message !=
                topological_order[i + 0].message) {
          message.line_top = y;
          message.is_separated = true;
          y += 1;
        }
        continue;
      }

      add_message(message, y, offset);
    }
  }
}

void SequenceImpl::Actor::Draw(Screen& screen, int height) {
  screen.DrawBoxedText(left, 0, name);
  screen.DrawVerticalLine(3, height - 4, center);
  screen.DrawBoxedText(left, height - 3, name);
  screen.DrawPixel(center, 2, U'┬');
  screen.DrawPixel(center, height - 3, U'┴');
}

void SequenceImpl::Message::Draw(Screen& screen) {
  if (line_top == line_bottom) {
    screen.DrawHorizontalLine(line_left, line_right, line_top);
  } else {
    if (direction == Message::Direction::Right) {
      screen.DrawHorizontalLine(line_left, line_left + offset, line_top);
      screen.DrawVerticalLine(line_top, line_bottom, line_left + offset);
      screen.DrawHorizontalLine(line_left + offset, line_right, line_bottom);
      screen.DrawPixel(line_left + offset, line_top, U'┐');
      screen.DrawPixel(line_left + offset, line_bottom, U'└');
    } else {
      screen.DrawHorizontalLine(line_right - offset, line_right, line_top);
      screen.DrawVerticalLine(line_top, line_bottom, line_right - offset);
      screen.DrawHorizontalLine(line_left, line_right - offset, line_bottom);
      screen.DrawPixel(line_right - offset, line_top, U'┌');
      screen.DrawPixel(line_right - offset, line_bottom, U'┘');
    }
  }

  // Tip of the arrow.
  if (direction == Message::Direction::Right) {
    screen.DrawPixel(line_right, line_bottom, U'>');
  } else {
    screen.DrawPixel(line_left, line_bottom, U'<');
  }

  // The message
  int y = top;
  for (auto& line : messages) {
    screen.DrawText(left, y, line);
    ++y;
  }
}

void SequenceImpl::Draw() {
  // Estimate output dimension.
  int width = actors.back().right;
  int height = 0;
  for (const Message& message : messages) {
    height = std::max(height, message.bottom);
  }
  height += 4;

  Screen screen(width, height);

  int x = 0;
  for (auto& actor : actors) {
    actor.Draw(screen, height);
  }

  for (auto& message : messages) {
    message.Draw(screen);
  }

  output_ = screen.ToString();

  std::wstringstream ss;
  for (auto& actor : actors) {
    ss << "Actor " << actor.name << '\n';
    for (auto& dependency : actor.dependencies) {
      ss << " * " << dependency.from << "<" << dependency.to << '\n';
    }
    ss << " - left = " << actor.left << '\n';
    ss << " - center = " << actor.center << '\n';
    ss << " - right = " << actor.right << '\n';
    ss << '\n';
  }

  for (auto& message : messages) {
    ss << "Message " << '\n';
    ss << "  From " << message.from << '\n';
    ss << "  To   " << message.to << '\n';
    ss << "  Id   " << message.id << '\n';
    for (auto& i : message.messages) {
      ss << "    " << i << '\n';
    }
    ss << '\n';
  }

  for (auto& it : message_index) {
    ss << "(" << it.first << "," << it.second << ")\n";
  }
  for (auto& it : actor_index) {
    ss << "(" << it.first << "," << it.second << ")\n";
  }

  ss << " width = " << width << " height = " << height << "\n";

  output_ += '\n' + to_string(ss.str());
}

std::string SequenceImpl::Output() {
  return output_;
}
