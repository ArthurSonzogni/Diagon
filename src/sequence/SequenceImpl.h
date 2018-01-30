#ifndef SEQUENCE_SEQUENCE_IMPL_H
#define SEQUENCE_SEQUENCE_IMPL_H

#include <string>
#include <vector>

#include "sequence/Sequence.h"
#include "sequence/SequenceLexer.h"
#include "sequence/SequenceParser.h"

class Screen;

class SequenceImpl : public Sequence {
 public:
  SequenceImpl() = default;
  virtual ~SequenceImpl() = default;
  void Process(const std::string& input) override;
  std::string Output() override;

 public: // Not really public.
  struct Dependency {
    int from;
    int to;
    bool operator<(const Dependency&) const;
    Dependency& operator=(const Dependency&) = default;
  };

 private:
  struct Actor {
    std::wstring name;
    std::set<Dependency> dependencies;

    // Computed position.
    int left = 0;
    int center = 0;
    int right = 0;
    void Draw(Screen& screen, int height);
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
    int line_top = 0;
    int line_bottom = 0;
    bool is_separated = false;
    int offset = 0;
    void Draw(Screen& screen);
  };
  std::vector<Message> messages;

  std::map<std::wstring, int> actor_index;
  std::map<int, int> message_index;

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
