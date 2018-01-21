#ifndef SEQUENCE_SEQUENCE_IMPL_H
#define SEQUENCE_SEQUENCE_IMPL_H

#include <string>
#include <vector>

#include "sequence/Sequence.h"
#include "sequence/SequenceLexer.h"
#include "sequence/SequenceParser.h"

class SequenceImpl : public Sequence {
 public:
  SequenceImpl() = default;
  virtual ~SequenceImpl() = default;
  void Process(const std::string& input) override;
  std::string Output() override;

 private:
  struct Actor {
    std::string name;
    std::vector<int> message_id;
  };
  std::vector<Actor> actors;

  struct Message {
    std::string from;
    std::string to;
    int id = -1;
    std::vector<std::string> messages;
  };
  std::vector<Message> messages;

  // 
  void ComputeInternalRepresentation(const std::string& input);
  void AddCommand(SequenceParser::CommandContext* command);
  void AddMessage(SequenceParser::MessageContext* message);
  void AddActor(SequenceParser::ActorContext* actor_context);
  int GetMessageID(SequenceParser::MessageIDContext* message_id);
  std::vector<std::string> GetMessageText(
      SequenceParser::MessageTextContext* message_text);


  void UniformizeInternalRepresentation();
  void UniformizeActors();
  void UniformizeMessageID();

  void Draw();
  std::string output_;
};

#endif /* end of include guard: SEQUENCE_SEQUENCE_IMPL_H */
