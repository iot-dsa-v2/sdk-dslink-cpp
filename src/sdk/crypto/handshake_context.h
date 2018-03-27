#ifndef DSA_SDK_HANDSHAKR_CONTEXT_H
#define DSA_SDK_HANDSHAKR_CONTEXT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <string>

#include "ecdh.h"

namespace dsa {

class HandshakeContext {
 private:
  ECDH _ecdh;

  std::vector<uint8_t> _salt;
  string_ _dsid;
  std::vector<uint8_t> _auth;

  std::vector<uint8_t> _remote_public_key;
  std::vector<uint8_t> _remote_salt;
  string_ _remote_dsid;
  std::vector<uint8_t> _remote_auth;

  std::vector<uint8_t> _shared_secret;

 public:
  explicit HandshakeContext(string_ dsid_prefix, ECDH &&ecdh);
  explicit HandshakeContext(string_ dsid_prefix);

  const std::vector<uint8_t> &public_key() const {
    return _ecdh.get_public_key();
  };
  const std::vector<uint8_t> &salt() const { return _salt; };
  const string_ &dsid() const { return _dsid; };
  const string_ &remote_dsid() const { return _remote_dsid; };
  const std::vector<uint8_t> &auth() const { return _auth; };
  const std::vector<uint8_t> &remote_auth() const { return _remote_auth; };

  const ECDH &ecdh() const { return _ecdh; };

  void set_remote(string_ &&dsid, std::vector<uint8_t> &&public_key,
                  std::vector<uint8_t> &&salt);
  void compute_secret();
};

}  // namespace dsa

#endif  // DSA_SDK_HANDSHAKR_CONTEXT_H
