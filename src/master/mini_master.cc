// Copyright (c) 2013, Cloudera, inc.

#include "master/mini_master.h"

#include <glog/logging.h>
#include <string>

#include "gutil/strings/substitute.h"
#include "server/rpc_server.h"
#include "server/webserver.h"
#include "master/master.h"
#include "util/net/sockaddr.h"
#include "util/status.h"

using strings::Substitute;

namespace kudu {
namespace master {

MiniMaster::MiniMaster(Env* env, const string& fs_root)
  : started_(false),
    env_(env),
    fs_root_(fs_root) {
}

MiniMaster::~MiniMaster() {
}

Status MiniMaster::Start() {
  CHECK(!started_);

  return StartOnPorts(0, 0);
}

Status MiniMaster::Shutdown() {
  RETURN_NOT_OK(master_->Shutdown());
  started_ = false;
  master_.reset();
  return Status::OK();
}

Status MiniMaster::StartOnPorts(uint16_t rpc_port, uint16_t web_port) {
  CHECK(!started_);

  MasterOptions opts;

  // Start RPC server on loopback.
  opts.rpc_opts.rpc_bind_addresses = Substitute("127.0.0.1:$0", rpc_port);
  opts.webserver_opts.port = web_port;

  gscoped_ptr<Master> server(new Master(opts));
  RETURN_NOT_OK(server->Init());
  RETURN_NOT_OK(server->Start());

  master_.swap(server);
  started_ = true;
  return Status::OK();
}

Status MiniMaster::Restart() {
  CHECK(started_);

  Sockaddr prev_rpc = bound_rpc_addr();
  Sockaddr prev_http = bound_http_addr();
  RETURN_NOT_OK_PREPEND(Shutdown(), "Could not shut down prior master");

  return StartOnPorts(prev_rpc.port(), prev_http.port());
}

const Sockaddr MiniMaster::bound_rpc_addr() const {
  CHECK(started_);
  return master_->first_rpc_address();
}

const Sockaddr MiniMaster::bound_http_addr() const {
  CHECK(started_);
  return master_->first_http_address();
}

} // namespace master
} // namespace kudu
