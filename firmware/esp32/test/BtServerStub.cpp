// Stub BtServer implementation for native unit testing.
// Only the methods called by MessageProcessor are needed.
#include "BtServer.h"

BtServer::BtServer(MessageProcessor* /*mp*/, Display* /*display*/) {}

void BtServer::setClientAuthenticated(bool authenticated) {
    clientAuthenticated = authenticated;
}

bool BtServer::isClientAuthenticated() {
    return clientAuthenticated;
}

void BtServer::disableAllCharacteristics() {}
void BtServer::enableAllCharacteristics() {}
void BtServer::resetAuthenticationState() { clientAuthenticated = false; }
void BtServer::checkPasswordTimeout() {}
void BtServer::setPasswordTimeout(uint32_t timeoutMs) { passwordTimeoutMs = timeoutMs; }
