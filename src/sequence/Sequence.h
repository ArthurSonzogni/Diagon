#ifndef SEQUENCE_SEQUENCE_H
#define SEQUENCE_SEQUENCE_H

#include <memory>
#include <string>

class Sequence {
 public:
  virtual ~Sequence();
  static std::unique_ptr<Sequence> Create();
  virtual void Process(const std::string& input) = 0;
  virtual std::string Output() = 0;

 protected:
  Sequence();
};

#endif /* end of include guard: SEQUENCE_SEQUENCE_H */
