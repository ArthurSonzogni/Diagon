#ifndef TRANSLATOR_SEQUENCE_SEQUENCE_H
#define TRANSLATOR_SEQUENCE_SEQUENCE_H

#include <string>
#include <vector>

#include "translator/Translator.h"
#include "translator/sequence/Sequence.h"
#include "translator/sequence/SequenceLexer.h"
#include "translator/sequence/SequenceParser.h"

class Screen;

class Sequence : public Translator {
 public:
  virtual ~Sequence() = default;
  std::string operator()(const std::string& input,
                         const std::string& options) override;

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

  bool ascii_only_;

  // 1) Parse.
  void ComputeInternalRepresentation(const std::string& input);
  void AddCommand(SequenceParser::CommandContext* command);
  void AddMessageCommand(SequenceParser::MessageCommandContext* message);
  void AddDependencyCommand(
      SequenceParser::DependencyCommandContext* actor_context);
  int GetNumber(SequenceParser::NumberContext* number);
  std::wstring GetText(SequenceParser::TextContext* text);

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
  std::string Draw();
  std::string output_;
};

#endif /* end of include guard: TRANSLATOR_SEQUENCE_SEQUENCE_H */
