// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <map>
#include <set>
#include <string>
#include <vector>
#include "translator/Translator.h"
#include "translator/sequence/SequenceLexer.h"
#include "translator/sequence/SequenceParser.h"

class Screen;

enum class Direction {
  Left,
  Right,
};

struct Dependency {
  int from;
  int to;

  bool operator<(const Dependency& other) const;
  Dependency& operator=(const Dependency&) = default;
};

struct Actor {
  std::wstring name;
  std::set<Dependency> dependencies;

  // Computed position.
  int left = 0;
  int center = 0;
  int right = 0;

  void Draw(Screen& screen, int height);
};

struct Message {
  std::wstring from;
  std::wstring to;
  int id = -1;
  std::vector<std::wstring> messages;

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

// Define minimum space between two actors.
struct ActorSpace {
  int a;
  int b;
  int space;
};

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

class Sequence : public Translator {
 public:
  virtual ~Sequence() = default;

 private:
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

  void SplitByBackslashN();

  // 3) Layout.
  void Layout();
  void LayoutComputeMessageWidth();
  void LayoutComputeActorsPositions();
  void LayoutComputeMessagesPositions();

  // 4)
  std::string Draw();
  std::string output_;

  const char* Name() final;
  const char* Identifier() final;
  const char* Description() final;
  std::vector<OptionDescription> Options() final;
  std::vector<Example> Examples() final;
  std::string Translate(const std::string& input,
                        const std::string& options_string) override;

  std::vector<Actor> actors;
  std::vector<Message> messages;

  std::map<std::wstring, int> actor_index;
  std::map<int, int> message_index;

  bool ascii_only_;
  bool interpret_backslash_n_ = false;
};
