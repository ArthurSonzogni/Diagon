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
    std::wstring name;
    std::vector<int> message_id;

    // Computed position.
    int left = 0;
    int center = 0;
    int right = 0;
  };
  std::vector<Actor> actors;

  struct Message {
    std::wstring from;
    std::wstring to;
    int id = -1;
    std::vector<std::wstring> messages;

    enum class Direction{Left, Right};
    Direction direction = Direction::Right;

    // Computed position.
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
    int width = 0;
    int line_left = 0;
    int line_right = 0;
  };
  std::vector<Message> messages;

  std::map<std::wstring, int> actor_index;

  // 1) Parse.
  void ComputeInternalRepresentation(const std::string& input);
  void AddCommand(SequenceParser::CommandContext* command);
  void AddMessage(SequenceParser::MessageContext* message);
  void AddActor(SequenceParser::ActorContext* actor_context);
  int GetMessageID(SequenceParser::MessageIDContext* message_id);
  std::vector<std::wstring> GetMessageText(
      SequenceParser::MessageTextContext* message_text);

  // 2) Clean the representation.
  void UniformizeInternalRepresentation();
  void UniformizeActors();
  void UniformizeMessageID();

  // 3) Layout.
  void Layout();
  void LayoutComputeMessageWidth();
  void LayoutComputeActorsPositions();
  void LayoutComputeMessagesPositions();

  // 4)
  void Draw();
  std::string output_;
};

#endif /* end of include guard: SEQUENCE_SEQUENCE_IMPL_H */
