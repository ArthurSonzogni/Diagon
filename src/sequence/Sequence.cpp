#include "sequence/Sequence.h"
#include "sequence/SequenceImpl.h"

Sequence::Sequence() {}
Sequence::~Sequence() {}

// static
std::unique_ptr<Sequence> Sequence::Create() {
  return std::make_unique<SequenceImpl>();
}
