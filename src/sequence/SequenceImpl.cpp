#include "sequence/SequenceImpl.h"

#include <set>
#include <sstream>

#include "sequence/SequenceLexer.h"
#include "sequence/SequenceParser.h"
#include "screen/Screen.h"

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
      if (declared_actors.count(actor) == 0) {
        declared_actors.insert(actor);
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
        } else {
          used.insert(message.id);
        }
      }
    }
  }

  // Remove duplicate in actors.message_id
  {
    for (auto& actor : actors) {
      std::set<int> used;
      for (auto& id : actor.message_id) {
        if (used.count(id) == 0) {
          used.insert(id);
        } else {
          id = -1;
        }
      }
      actor.message_id.erase(
          std::remove_if(actor.message_id.begin(), actor.message_id.end(),
                         [](int a) { return a == -1; }),
          actor.message_id.end());
    }
  }

  // Find the greatest id.
  int max_id;
  {
    std::set<int> used_message_id;
    for (auto& message : messages) {
      used_message_id.insert(message.id);
    }
    for (auto& actor : actors) {
      for (auto& id : actor.message_id) {
        used_message_id.insert(id);
      }
    }
    max_id = *used_message_id.rbegin();
  }

  // Assign an ID for every messages.
  {
    for (auto& message : messages) {
      if (message.id == -1) {
        message.id = ++max_id;
      }
    }
  }

  // Fill the actors.message_id list.
  {
    std::set<int> used_message_id;
    for (auto& message : messages) {
      used_message_id.insert(message.id);
    }
    for (auto& actor : actors) {
      for (auto& id : used_message_id) {
        if (std::find(actor.message_id.begin(), actor.message_id.end(), id) ==
            actor.message_id.end()) {
          actor.message_id.push_back(id);
        }
      }
    }
  }

  // Remove unused id in actors.message.id.
  {
    std::set<int> used_message_id;
    for (auto& message : messages) {
      used_message_id.insert(message.id);
    }
    for (auto& actor : actors) {
      actor.message_id.erase(
          std::remove_if(actor.message_id.begin(), actor.message_id.end(),
                         [&](int a) { return used_message_id.count(a) == 0; }),
          actor.message_id.end());
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
  Actor actor;
  actor.name = to_wstring(actor_context->Words()->getSymbol()->getText());

  for (const auto& message_id : actor_context->Number()) {
    actor.message_id.push_back(std::stoi(message_id->getSymbol()->getText()));
  }
  actors.push_back(actor);
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
  for (int i = 0; i < actors.size(); ++i)
    actor_index[actors[i].name] = i;

  std::vector<ActorSpace> spaces;
  for (int i = 0; i < actors.size() - 1; ++i) {
    int size_1 = actors[i].name.size();
    int size_2 = actors[i + 1].name.size();
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

  actors[0].center = actors[0].name.size() / 2 + 1;

  bool modified = true;
  while (modified) {
    modified = false;
    for (const ActorSpace& s : spaces) {
      if (actors[s.b].center - actors[s.a].center < s.space) {
        actors[s.b].center = actors[s.a].center + s.space;
        modified = true;
      }
    }
  }

  for (auto& actor : actors) {
    actor.left = actor.center - actor.name.size() / 2 - 1;
    actor.right = actor.left + actor.name.size() + 2;
  }
}

void SequenceImpl::LayoutComputeMessagesPositions() {
  int y = 4;

  for (auto& message : messages) {
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
    y = message.bottom + 2;
  }
}

void SequenceImpl::Draw() {
  int width = actors.back().right;
  int height = messages.back().bottom+1+3;
  std::vector<std::wstring> lines(height, std::wstring(width, U' '));

  Screen screen(width, height);

  int x = 0;
  for (auto& actor : actors) {
    screen.DrawBoxedText(actor.left, 0, actor.name);
    screen.DrawVerticalLine(3, height - 4, actor.center);
    screen.DrawBoxedText(actor.left, height - 3, actor.name);
    screen.DrawPixel(actor.center, 2, U'┬');
    screen.DrawPixel(actor.center, height-3, U'┴');
  }

  for (auto& message : messages) {
    int y = message.top;
    for (auto& line : message.messages) {
      screen.DrawText(message.left, y, line);
      ++y;
    }
    screen.DrawHorizontalLine(message.line_left, message.line_right, y);
    if (message.direction == Message::Direction::Left) {
      screen.DrawPixel(message.line_left, y, U'<');
    } else {
      screen.DrawPixel(message.line_right, y, U'>');
    }
  }

  output_ = screen.ToString();

  // for (auto& actor : actors) {
  // ss << "Actor " << actor.name << std::endl;
  // for (auto& i : actor.message_id) {
  // ss << " " << i << std::endl;
  //}
  //}

  // for (auto& message : messages) {
  // ss << "Message " << std::endl;
  // ss << "  From " << message.from << std::endl;
  // ss << "  To   " << message.to << std::endl;
  // ss << "  Id   " << message.id << std::endl;
  // for (auto& i : message.messages) {
  // ss << "    " << i << std::endl;
  //}
  //}
}

std::string SequenceImpl::Output() {
  return output_;
}
