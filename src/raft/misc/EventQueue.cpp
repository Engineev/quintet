#include "misc/EventQueue.h"

namespace quintet {

EventQueue::EventQueue() : pImpl(std::make_unique<EventQueue::Impl>()) {}

EventQueue::~EventQueue() = default;

struct EventQueue::Impl {};

} // namespace quintet