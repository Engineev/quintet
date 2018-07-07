#pragma once

#include <memory>

#include <boost/thread/future.hpp>

#include "client_context.h"
#include "./common.h"

namespace quintet {
namespace raft {
namespace rpc {

class Client {
public:
  explicit Client(const ServerId & target);
  ~Client();

  Reply
  callRpcAppendEntries(ClientContext ctx, const AppendEntriesMessage &msg);

  boost::future<Reply>
  asyncCallRpcAppendEntries(ClientContext ctx, const AppendEntriesMessage &msg);

  Reply
  callRpcRequestVote(ClientContext ctx, const RequestVoteMessage &msg);

  boost::future<Reply>
  asyncCallRpcRequestVote(ClientContext ctx, const RequestVoteMessage &msg);

private:
  class Impl;
  std::unique_ptr<Impl> pImpl;

}; // class Client

} // namespace quintet
} // namespace raft
} // namespace rpc