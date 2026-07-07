#include "static_server.h"

namespace loralink::webserver {

bool StaticServer::begin(uint16_t) { return true; }
void StaticServer::handleClient() {}
void StaticServer::stop() {}

} // namespace loralink::webserver
