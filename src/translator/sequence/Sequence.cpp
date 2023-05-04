// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "translator/sequence/Sequence.hpp"

#include <functional>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "screen/Screen.h"
#include "translator/antlr_error_listener.h"
#include "translator/sequence/Graph.hpp"

namespace {

void SplitString(std::wstring input,
                 const std::wstring& delimiter,
                 std::vector<std::wstring>* out) {
  size_t start = 0U;
  size_t end = input.find(delimiter);
  while (end != std::wstring::npos) {
    out->push_back(input.substr(start, end - start));
    start = end + delimiter.length();
    end = input.find(delimiter, start);
  }
  out->push_back(input.substr(start));
}

}  // namespace

void Actor::Draw(Screen& screen, int height) {
  screen.DrawBoxedText(left, 0, name);
  screen.DrawVerticalLine(3, height - 4, center);
  screen.DrawBoxedText(left, height - 3, name);
  screen.DrawPixel(center, 2, L'┬');
  screen.DrawPixel(center, height - 3, L'┴');
}

void Message::Draw(Screen& screen) {
  if (line_top == line_bottom) {
    screen.DrawHorizontalLine(line_left, line_right, line_top);
  } else {
    if (direction == Direction::Right) {
      screen.DrawHorizontalLine(line_left, line_left + offset, line_top);
      screen.DrawVerticalLine(line_top, line_bottom, line_left + offset);
      screen.DrawHorizontalLine(line_left + offset, line_right, line_bottom);
      screen.DrawPixel(line_left + offset, line_top, L'┐');
      screen.DrawPixel(line_left + offset, line_bottom, L'└');
    } else {
      screen.DrawHorizontalLine(line_right - offset, line_right, line_top);
      screen.DrawVerticalLine(line_top, line_bottom, line_right - offset);
      screen.DrawHorizontalLine(line_left, line_right - offset, line_bottom);
      screen.DrawPixel(line_right - offset, line_top, L'┌');
      screen.DrawPixel(line_right - offset, line_bottom, L'┘');
    }
  }

  // Tip of the arrow.
  if (direction == Direction::Right) {
    screen.DrawPixel(line_right, line_bottom, L'>');
  } else {
    screen.DrawPixel(line_left, line_bottom, L'<');
  }

  // The message
  int y = top;
  for (auto& line : messages) {
    screen.DrawText(left, y, line);
    ++y;
  }
}

bool Dependency::operator<(const Dependency& other) const {
  if (from < other.from)
    return true;
  if (from > other.from)
    return false;
  return to < other.to;
}

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
const char* Sequence::Identifier() {
  return "Sequence";
}
const char* Sequence::Name() {
  return "Sequence diagram";
}
const char* Sequence::Description() {
  return "Draw sequence diagram";
}

std::vector<Translator::OptionDescription> Sequence::Options() {
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
      {
          "interpret_backslash_n",
          {
              "false",
              "true",
          },
          "true",
          "Insert new lines at every occurence of '\\n' (backslash n) in the "
          "message field.",
          Widget::Checkbox,
      },
  };
}

std::vector<Translator::Example> Sequence::Examples() {
  return {
      {"1-basic",
       "Alice -> Bob: Hello Bob!\n"
       "Alice <- Bob: Hello Alice!"},
      {"2-More actors",
       "Renderer -> Browser: BeginNavigation()\n"
       "Browser -> Network: URLRequest()\n"
       "Browser <- Network: URLResponse()\n"
       "Renderer <- Browser: CommitNavigation()\n"
       "Renderer -> Browser: DidCommitNavigation()"},
      {"3-Actors order",
       "Actor 2 -> Actor 3: message 1\n"
       "Actor 1 -> Actor 2: message 2\n"
       "\n"
       "Actor 1:\n"
       "Actor 2:\n"
       "Actor 3:"},
      {"4-Message order",
       "2) Actor 2 -> Actor 3: message 1\n"
       "1) Actor 1 -> Actor 2: message 2\n"
       "\n"
       "Actor 1:\n"
       "Actor 2: 1<2\n"
       "Actor 3:"},
      {"5-Message crossing",
       "1) Renderer -> Browser: Message 1\n"
       "2) Renderer <- Browser: Message 2\n"
       "\n"
       "Renderer: 1<2\n"
       "Browser: 2<1"},
  };
}

std::string Sequence::Translate(const std::string& input,
                                const std::string& options_string) {
  *this = Sequence();

  auto options = SerializeOption(options_string);
  ascii_only_ = (options["ascii_only"] == "true");
  interpret_backslash_n_ = (options["interpret_backslash_n"] != "false");

  ComputeInternalRepresentation(input);
  UniformizeInternalRepresentation();
  if (actors.size() == 0)
    return "";

  SplitByBackslashN();
  Layout();
  return Draw();
}

void Sequence::SplitByBackslashN() {
  if (!interpret_backslash_n_) {
    return;
  }
  for (auto& message : messages) {
    std::vector<std::wstring> old = std::move(message.messages);
    for(auto& it : old) {
      SplitString(it, L"\\n", &message.messages);
    }
  }
}

void Sequence::ComputeInternalRepresentation(const std::string& input) {
  antlr4::ANTLRInputStream input_stream(input);

  // Lexer.
  SequenceLexer lexer(&input_stream);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  // Parser.
  AntlrErrorListener error_listener;
  SequenceParser parser(&tokens);
  parser.addErrorListener(&error_listener);

  SequenceParser::ProgramContext* program = nullptr;
  try {
    program = parser.program();
  } catch (...) {
    return;
  }

  for (SequenceParser::CommandContext* command : program->command()) {
    AddCommand(command);
  }
}

void Sequence::UniformizeInternalRepresentation() {
  UniformizeActors();
  UniformizeMessageID();
}

void Sequence::UniformizeActors() {
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

void Sequence::UniformizeMessageID() {
  // Remove duplicate in message.id
  {
    std::set<int> used;
    for (auto& message : messages) {
      if (message.id != -1) {
        if (used.count(message.id)) {
          std::cerr << "Found two messages with the same id: " << message.id
                    << std::endl;
          message.id = -1;
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
            std::wcerr << "* Ignored dependency: \"" << actor.name << ": "
                       << dependency.from << " < " << dependency.to << "\"."
                       << std::endl;
            std::wcerr << "  It cannot be used because the message ID \"" << id
                       << "\" doesn't exist" << std::endl;
            return true;
          }

          const Message& message = messages[it->second];
          if (actor.name != message.from &&  //
              actor.name != message.to) {
            std::wcerr << "* Ignored dependency: \"" << actor.name << ": "
                       << dependency.from << " < " << dependency.to << "\"."
                       << std::endl;
            std::wcerr << "  It cannot be used because the message \""
                       << message.from << " -> " << message.to << ": "
                       << message.messages[0]
                       << "\" has nothing to do with actor " << actor.name
                       << std::endl;
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

void Sequence::AddCommand(SequenceParser::CommandContext* command) {
  if (auto message_command = command->messageCommand()) {
    AddMessageCommand(message_command);
    return;
  }

  if (auto dependency_command = command->dependencyCommand()) {
    AddDependencyCommand(dependency_command);
    return;
  }
}

void Sequence::AddMessageCommand(
    SequenceParser::MessageCommandContext* message_command) {
  Message message;
  if (auto dependency_id = message_command->dependencyID()) {
    message.id =
        std::stoi(dependency_id->number()->NUMBER()->getSymbol()->getText());
  }

  message.from = GetText(message_command->text(0));
  message.to = GetText(message_command->text(1));

  if (message.from == message.to) {
    std::cerr << "Self messages are not supported yet. It has been ignored."
              << std::endl;
    std::cerr << "See https://github.com/ArthurSonzogni/Diagon/issues/63"
              << std::endl;
    return;
  }

  if (message_command->arrow()->ARROW_LEFT()) {
    std::swap(message.from, message.to);
  }

  message.messages.push_back(GetText(message_command->text(2)));
  messages.push_back(message);
}

void Sequence::AddDependencyCommand(
    SequenceParser::DependencyCommandContext* dependency_command) {
  std::wstring name = GetText(dependency_command->text());
  if (!actor_index.count(name)) {
    actor_index[name] = actors.size();
    actors.emplace_back();
  }
  Actor& actor = actors[actor_index[name]];
  actor.name = name;

  for (auto dependency : dependency_command->dependencies()->dependency()) {
    auto numbers = dependency->number();
    auto comparison = dependency->comparison();
    for (int i = 0; i < comparison.size(); ++i) {
      int left = std::stoi(numbers[i]->NUMBER()->getSymbol()->getText());
      int right = std::stoi(numbers[i + 1]->NUMBER()->getSymbol()->getText());
      if (comparison[i]->GREATER()) {
        std::swap(left, right);
      }
      actor.dependencies.insert(Dependency{left, right});
    }
  }
}

std::wstring Sequence::GetText(SequenceParser::TextContext* text) {
  return to_wstring(text->textInternal()->getText());
}

int Sequence::GetNumber(SequenceParser::NumberContext* number) {
  return std::stoi(number->NUMBER()->getSymbol()->getText());
}

void Sequence::Layout() {
  LayoutComputeMessageWidth();
  LayoutComputeActorsPositions();
  LayoutComputeMessagesPositions();
}

void Sequence::LayoutComputeMessageWidth() {
  for (auto& message : messages) {
    for (auto& text : message.messages) {
      message.width = std::max(message.width, int(text.size()));
    }
  }
}

void Sequence::LayoutComputeActorsPositions() {
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
    ActorSpace space{actor_index[message.from],  //
                     actor_index[message.to],    //
                     message.width + 1};         //
    if (space.a > space.b)
      std::swap(space.a, space.b);
    spaces.push_back(space);
  }

  if (actors.size() != 0)
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
    if (i++ > 500) {
      std::cerr << "Something went wrong!" << std::endl;
      break;
    }
  }

  for (auto& actor : actors) {
    actor.left = actor.center - actor.name.size() / 2 - actor.name.size() % 2;
    actor.right = actor.left + actor.name.size() + 2;
  }
}

void Sequence::LayoutComputeMessagesPositions() {
  // Build graph
  MessageDependencies message_dependencies;
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
      message.direction = Direction::Left;
    } else {
      message.direction = Direction::Right;
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
      message.offset = offset;
      offset += 2;
    } else {
      message.line_top = message.bottom;
    }
    y = message.bottom + 2;
  };

  auto preference = [&](int a, int b) -> bool {
    return message_index[a] < message_index[b];
  };

  for (auto& cut : Cut(message_dependencies, preference)) {
    int offset = 2;

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
          graph.insert({{a, dependency.from}, {a, dependency.to}});
        }
      }
    }

    for (auto& message : messages) {
      if (cut.count(message.id)) {
        graph::Node from = {actor_index[message.from], message.id};
        graph::Node to = {actor_index[message.to], message.id};
        graph.insert(graph::Edge(from, to));
      }
    }
    std::set<int> started_message;
    auto topological_order = FindTopologicalOrder(graph);
    for (int i = 0; i < topological_order.size(); ++i) {
      const graph::Node& node = topological_order[i];
      auto& message = messages[message_index[node.message]];

      if (started_message.count(node.message) == 0) {
        started_message.insert(node.message);
        if (i + 1 < 0 || topological_order[i + 1].message !=
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

  // Sort message by y.
  std::sort(messages.begin(), messages.end(),
            [](const Message& a, const Message& b) {
              return a.line_bottom > b.line_bottom;
            });
}

std::string Sequence::Draw() {
  // Estimate output dimension.
  int width = actors.back().right;
  int height = 0;
  for (const Message& message : messages) {
    height = std::max(height, message.bottom);
  }
  height += 4;

  Screen screen(width, height);

  for (auto& actor : actors)
    actor.Draw(screen, height);

  for (auto message : messages)
    message.Draw(screen);

  if (ascii_only_)
    screen.ASCIIfy(0);
  return screen.ToString();
}

std::unique_ptr<Translator> SequenceTranslator() {
  return std::make_unique<Sequence>();
}

std::string Sequence::Highlight(const std::string& input) {
  std::stringstream out;

  antlr4::ANTLRInputStream input_stream(input);

  // Lexer.
  SequenceLexer lexer(&input_stream);
  antlr4::CommonTokenStream tokens(&lexer);

  try {
    tokens.fill();
  }
  catch (...) {  // Ignore
  }

  size_t matched = 0;
  out << "<span class='Sequence'>";
  for(antlr4::Token* token : tokens.getTokens()) {
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
