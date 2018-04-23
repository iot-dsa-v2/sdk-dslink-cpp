#include "dsa_common.h"

#include "certificate.h"

#include <boost/filesystem.hpp>
#include "module/storage.h"
#include "util/string.h"

#include <cmath>
#include <memory>
using std::unique_ptr;
#include <random>
#include <string>

#if defined(_MSC_VER)
#include <openssl/applink.c>
#endif

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509v3.h>

using BIO_FILE_ptr = std::unique_ptr<BIO, decltype(&::BIO_free)>;
using BN_ptr = std::unique_ptr<BIGNUM, decltype(&::BN_free)>;
using EVP_KEY_ptr = std::unique_ptr<EVP_PKEY, decltype(&::EVP_PKEY_free)>;
using RSA_ptr = std::unique_ptr<RSA, decltype(&::RSA_free)>;
using X509_ptr = std::unique_ptr<X509, decltype(&::X509_free)>;

#include "module/logger.h"

namespace dsa {

void generate_certificate() {
  int rc;

  RSA_ptr rsa(RSA_new(), ::RSA_free);
  BN_ptr bn(BN_new(), ::BN_free);

  if (rc = BN_set_word(bn.get(), RSA_F4) != 1) {
    LOG_FATAL(__FILENAME__,
              LOG << "BN_set_word failed!"
                  << " rc=" << rc);
  }

  if (rc = RSA_generate_key_ex(rsa.get(), 4098, bn.get(), NULL) != 1) {
    LOG_FATAL(__FILENAME__,
              LOG << "RSA_generate_key_ex failed!"
                  << " rc=" << rc);
  }

  EVP_KEY_ptr pkey(EVP_PKEY_new(), ::EVP_PKEY_free);
  if (rc = EVP_PKEY_set1_RSA(pkey.get(), rsa.get()) != 1) {
    LOG_FATAL(__FILENAME__,
              LOG << "EVP_PKEY_set1_RSA failed!"
                  << " rc=" << rc);
  }

#if 0
    // write public key
    BIO_FILE_ptr pubkey(BIO_new_file("pubkey.pem", "w"), ::BIO_free);
    if (rc = PEM_write_bio_PUBKEY(pubkey.get(), pkey.get()) != 1) {
      LOG_FATAL(__FILENAME__, LOG << "PEM_write_bio_PUBKEY failed!" << " rc=" << rc);
    }
#endif

  // write private key
  std::string privkey_fname("key.pem");
  BIO_FILE_ptr privkey(BIO_new_file(privkey_fname.c_str(), "w"), ::BIO_free);
  if (rc = PEM_write_bio_RSAPrivateKey(privkey.get(), rsa.get(), NULL, NULL, 0,
                                       NULL, NULL) != 1) {
    LOG_FATAL(__FILENAME__,
              LOG << "PEM_write_bio_RSAPrivate failed!"
                  << " rc=" << rc);
  }

  namespace fs = boost::filesystem;
  fs::permissions(privkey_fname.c_str(), fs::owner_read | fs::owner_write);

  // generate certificate
  X509_ptr x509(X509_new(), ::X509_free);

  X509_set_version(x509.get(), 2L);

  if (rc = X509_set_pubkey(x509.get(), pkey.get()) != 1) {
    LOG_FATAL(__FILENAME__,
              LOG << "X509_set_pubkey failed!"
                  << " rc=" << rc);
  }

  std::random_device rd;
  std::mt19937_64 e2(rd());
  std::uniform_int_distribution<int64_t> dist(std::llround(std::pow(2, 61)),
                                              std::llround(std::pow(2, 62)));

  ASN1_INTEGER_set(X509_get_serialNumber(x509.get()), dist(e2));

  X509_gmtime_adj((ASN1_TIME *)X509_get_notBefore(x509.get()), 0);
  X509_gmtime_adj((ASN1_TIME *)X509_get_notAfter(x509.get()), 0x7fffffff);

  X509_NAME *name;
  name = X509_get_subject_name(x509.get());
  X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *)"US", -1,
                             -1, 0);
  X509_NAME_add_entry_by_txt(name, "ST", MBSTRING_ASC,
                             (unsigned char *)"California", -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "L", MBSTRING_ASC,
                             (unsigned char *)"Berkeley", -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC,
                             (unsigned char *)"Acuity Brands, Inc.", -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "OU", MBSTRING_ASC,
                             (unsigned char *)"Customers", -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"*", -1,
                             -1, 0);
  if (X509_set_issuer_name(x509.get(), name) != 1) {
    LOG_FATAL(__FILENAME__, LOG << "X509_set_issuer_name failed!");
  }

  if (!X509_sign(x509.get(), pkey.get(), EVP_sha256())) {
    LOG_FATAL(__FILENAME__, LOG << "X509_sign failed!");
  }

  FILE *f = fopen("certificate.pem", "wb");
  if (!PEM_write_X509(f, x509.get())) {
    fclose(f);
    LOG_FATAL(__FILENAME__, LOG << "PEM_write_X509 failed!");
  }
  fclose(f);

  return;
}

void load_root_certificate(ssl::context &context,
                           boost::system::error_code &error_code) {
  auto &config_bucket = Storage::get_config_bucket();
  string_ certificate = string_from_storage("certificate.pem", config_bucket);
  context.add_certificate_authority(
      boost::asio::buffer(certificate.data(), certificate.size()), error_code);

  if (error_code && ERR_GET_REASON(error_code.value()) != X509_R_CERT_ALREADY_IN_HASH_TABLE) {
      LOG_FATAL(__FILENAME__, LOG << "Client failed to verify certificate");
  }
}

bool load_server_certificate(ssl::context &context,
                             boost::system::error_code &error_code) {
  auto &config_bucket = Storage::get_config_bucket();

  string_ certificate = string_from_storage("certificate.pem", config_bucket);
  context.use_certificate_chain(
      boost::asio::buffer(certificate.data(), certificate.size()), error_code);
  if (error_code != boost::system::errc::success) {
    LOG_INFO(__FILENAME__, LOG << "Server failed to load certificate.pem");
    return false;
  }

  string_ privkey = string_from_storage("key.pem", config_bucket);
  context.use_private_key(boost::asio::buffer(privkey.data(), privkey.size()),
                          boost::asio::ssl::context::pem, error_code);
  if (error_code != boost::system::errc::success) {
    LOG_INFO(__FILENAME__, LOG << "Server failed to load key.pem");
    return false;
  }

  return true;
}

}  // namespace dsa
