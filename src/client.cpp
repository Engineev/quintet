#include "client.h"

#include <chrono>

#include <boost/thread/thread_only.hpp>

#include <grpcpp/grpcpp.h>

#include "rpc/quintet.grpc.pb.h"
#include "rpc/client_impl.h"

namespace quintet {

class Client::Impl
    : public rpc::ClientImpl<rpc::PbExternalReply, Object, rpc::External> {
public:
  explicit Impl(const ServerId &target)
      : rpc::ClientImpl<rpc::PbExternalReply, Object, rpc::External>(target) {}

  boost::future<Object> asyncCall(ClientContext ctx, const std::string &opName,
                                  const std::string &args) {
    rpc::PbExternalMessage request;
    request.set_opname(opName);
    request.set_args(args);
    return asyncCallImpl(ctx, std::move(request));
  }

}; // class Client::Impl

} // namespace quintet

namespace quintet {

Client::Client(ServerId target)
    : pImpl(std::make_unique<Impl>(std::move(target))) {}

boost::future<Object>
Client::asyncCallImpl(ClientContext ctx, std::string opName, std::string args) {
  return pImpl->asyncCall(ctx, opName, args);
}

} // namespace quintet