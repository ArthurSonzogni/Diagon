#include "sequence/SequenceImpl.h"

#include <set>
#include <sstream>

#include "sequence/SequenceLexer.h"
#include "sequence/SequenceParser.h"

void SequenceImpl::Process(const std::string& input) {
  ComputeInternalRepresentation(input);
  UniformizeInternalRepresentation();
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
  std::set<std::string> used_actors;
  for (auto& message : messages) {
    used_actors.insert(message.from);
    used_actors.insert(message.to);
  }
  std::set<std::string> declared_actors;
  for (auto& actor : actors) {
    declared_actors.insert(actor.name);
  }
  // Add the missing Actors.
  for (auto& actor : used_actors) {
    if (!declared_actors.count(actor)) {
      Actor a;
      a.name = actor;
      actors.push_back(a);
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

  message.from = message_context->Words(0)->getSymbol()->getText();
  message.to = message_context->Words(1)->getSymbol()->getText();
  message.messages = GetMessageText(message_context->messageText());
  messages.push_back(message);
}

void SequenceImpl::AddActor(SequenceParser::ActorContext* actor_context) {
  Actor actor;
  actor.name = actor_context->Words()->getSymbol()->getText();

  for (const auto& message_id : actor_context->Number()) {
    actor.message_id.push_back(std::stoi(message_id->getSymbol()->getText()));
  }
  actors.push_back(actor);
}

int SequenceImpl::GetMessageID(SequenceParser::MessageIDContext* message_id) {
  return std::stoi(message_id->Number()->getSymbol()->getText());
}

std::vector<std::string> SequenceImpl::GetMessageText(
    SequenceParser::MessageTextContext* message_text) {
  std::vector<std::string> messages;
  if (message_text->getRuleIndex() == 0) {
    messages.push_back(
        static_cast<SequenceParser::SingleLineTextContext*>(message_text)
            ->Words()
            ->getSymbol()
            ->getText());
  } else {
    auto m = static_cast<SequenceParser::MultiLineTextContext*>(message_text);
    for (const auto& line : m->Words()) {
      messages.push_back(line->getSymbol()->getText());
    }
  }
  return messages;
}

void SequenceImpl::Draw() {
  std::stringstream ss;

  for (auto& actor : actors) {
    ss << "Actor " << actor.name << std::endl;
    for (auto& i : actor.message_id) {
      ss << " " << i << std::endl;
    }
  }

  for (auto& message : messages) {
    ss << "Message " << std::endl;
    ss << "  From " << message.from << std::endl;
    ss << "  To   " << message.to << std::endl;
    ss << "  Id   " << message.id << std::endl;
    for (auto& i : message.messages) {
      ss << "    " << i << std::endl;
    }
  }

  output_ = ss.str();
}

std::string SequenceImpl::Output() {
  return output_;
}
