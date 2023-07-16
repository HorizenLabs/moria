/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <iostream>
#include <random>

#include <gsl/gsl_util>
#include <openssl/pem.h>
#include <openssl/rand.h>

#include <app/network/secure.hpp>

namespace zenpp::network {

void print_ssl_error(unsigned long err, const log::Level severity) {
    if (!err) {
        return;
    }
    char buf[256];
    ERR_error_string_n(err, buf, sizeof(buf));
    log::BufferBase(severity, "SSL error", {"code", std::to_string(err), "reason", std::string(buf)});
}

EVP_PKEY* generate_random_rsa_key_pair(int bits) {
    EVP_PKEY* pkey{nullptr};
    BIGNUM* bn{BN_new()};
    if (bn == nullptr) {
        LOG_ERROR << "Failed to create BIGNUM";
        return nullptr;
    }
    auto bn_free{gsl::finally([bn]() { BN_free(bn); })};

    if (!BN_set_word(bn, static_cast<BN_ULONG>(RSA_F4))) {
        LOG_ERROR << "Failed to set BIGNUM";
        return nullptr;
    }

    RSA* rsa{RSA_new()};
    if (rsa == nullptr) {
        LOG_ERROR << "Failed to create RSA";
        return nullptr;
    }

    if (!RAND_poll()) {
        auto err{ERR_get_error()};
        print_ssl_error(err);
        LOG_ERROR << "Failed to initialize random number generator";
        RSA_free(rsa);
        return nullptr;
    }

    if (!RSA_generate_key_ex(rsa, bits, bn, nullptr)) {
        auto err{ERR_get_error()};
        print_ssl_error(err);
        LOG_ERROR << "Failed to initialize random number generator";
        RSA_free(rsa);
        return nullptr;
    }

    pkey = EVP_PKEY_new();
    if (pkey == nullptr) {
        LOG_ERROR << "Failed to create EVP_PKEY";
        RSA_free(rsa);
        return nullptr;
    }
    if (!EVP_PKEY_assign_RSA(pkey, rsa)) {
        auto err{ERR_get_error()};
        print_ssl_error(err);
        LOG_ERROR << "Failed to assign RSA to EVP_PKEY";
        EVP_PKEY_free(pkey);
        return nullptr;
    }

    return pkey;
}

X509* generate_self_signed_certificate(EVP_PKEY* pkey) {
    if (!pkey) {
        LOG_ERROR << "Invalid EVP_PKEY";
        return nullptr;
    }

    X509* x509_certificate = X509_new();
    if (!x509_certificate) {
        LOG_ERROR << "Failed to create X509 certificate";
        return nullptr;
    }

    // Generate random serial number
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution;
    long random_number{distribution(gen)};
    ASN1_INTEGER_set(X509_get_serialNumber(x509_certificate), random_number);

    // Set issuer, subject and validity
    X509_gmtime_adj(X509_get_notBefore(x509_certificate), 0);
    X509_gmtime_adj(X509_get_notAfter(x509_certificate), static_cast<long>(86400 * kCertificateValidityDays));

    X509_NAME* subject = X509_NAME_new();
    X509_NAME_add_entry_by_txt(subject, "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char*>("zend++.node"), -1,
                               -1, 0);
    X509_set_subject_name(x509_certificate, subject);
    X509_set_issuer_name(x509_certificate, subject);
    X509_NAME_free(subject);

    // Set public key
    if (!X509_set_pubkey(x509_certificate, pkey)) {
        auto err{ERR_get_error()};
        print_ssl_error(err);
        LOG_ERROR << "Failed to set public key";
        X509_free(x509_certificate);
        return nullptr;
    }

    // Sign certificate
    if (!X509_sign(x509_certificate, pkey, EVP_sha256())) {
        auto err{ERR_get_error()};
        print_ssl_error(err);
        LOG_ERROR << "Failed to sign certificate";
        X509_free(x509_certificate);
        return nullptr;
    }

    return x509_certificate;
}

bool store_rsa_key_pair(EVP_PKEY* pkey, const std::string& password, const std::filesystem::path& directory_path) {
    if (!pkey) {
        LOG_ERROR << "Invalid EVP_PKEY";
        return false;
    }

    if (directory_path.empty() || directory_path.is_relative() || !std::filesystem::is_directory(directory_path)) {
        LOG_ERROR << "Invalid path " << directory_path.string();
        return false;
    }

    auto file_path = directory_path / kPrivateKeyFileName;
    FILE* file{nullptr};

#if defined(_MSC_VER)
    fopen_s(&file, file_path.string().c_str(), "wb");
#else
    file = fopen(file_path.string().c_str(), "wb");
#endif

    if (!file) {
        LOG_ERROR << "Failed to open file " << file_path.string();
        return false;
    }
    auto file_close{gsl::finally([file]() { fclose(file); })};

    int result{0};
    if (!password.empty()) {
        result = PEM_write_PKCS8PrivateKey(file, pkey, EVP_aes_256_cbc(), const_cast<char*>(password.data()),
                                           static_cast<int>(password.size()), nullptr, nullptr);
    } else {
        result = PEM_write_PrivateKey(file, pkey, nullptr, nullptr, 0, nullptr, nullptr);
    }

    if (!result) {
        auto err{ERR_get_error()};
        print_ssl_error(err);
        LOG_ERROR << "Failed to write private key to file " << file_path.string();
        return false;
    }

    return true;
}
bool store_x509_certificate(X509* cert, const std::filesystem::path& directory_path) {
    if (!cert) {
        LOG_ERROR << "Invalid X509 certificate";
        return false;
    }

    if (directory_path.empty() || directory_path.is_relative() || !std::filesystem::is_directory(directory_path)) {
        LOG_ERROR << "Invalid path " << directory_path.string();
        return false;
    }

    auto file_path = directory_path / kCertificateFileName;
    FILE* file{nullptr};

#if defined(_MSC_VER)
    fopen_s(&file, file_path.string().c_str(), "wb");
#else
    file = fopen(file_path.string().c_str(), "wb");
#endif

    if (!file) {
        LOG_ERROR << "Failed to open file " << file_path.string();
        return false;
    }
    auto file_close{gsl::finally([file]() { fclose(file); })};

    if (!PEM_write_X509(file, cert)) {
        auto err{ERR_get_error()};
        print_ssl_error(err);
        LOG_ERROR << "Failed to write certificate to file " << file_path.string();
        return false;
    }

    return true;
}

EVP_PKEY* load_rsa_private_key(const std::filesystem::path& directory_path, const std::string& password) {
    if (!std::filesystem::exists(directory_path) || !std::filesystem::is_directory(directory_path)) {
        LOG_ERROR << "Invalid or not existing container directory " << directory_path.string();
        return nullptr;
    }

    auto file_path = directory_path / kPrivateKeyFileName;
    if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
        LOG_ERROR << "Invalid or not existing file " << file_path.string();
        return nullptr;
    }

    FILE* file{nullptr};

#if defined(_MSC_VER)
    fopen_s(&file, file_path.string().c_str(), "rb");
#else
    file = fopen(file_path.string().c_str(), "rb");
#endif

    if (!file) {
        LOG_ERROR << "Failed to open file " << file_path.string();
        return nullptr;
    }
    auto file_close{gsl::finally([file]() { fclose(file); })};

    EVP_PKEY* pkey{nullptr};
    if (!password.empty()) {
        pkey = PEM_read_PrivateKey(file, nullptr, nullptr,
                                   const_cast<void*>(reinterpret_cast<const void*>(password.data())));
    } else {
        pkey = PEM_read_PrivateKey(file, nullptr, nullptr, nullptr);
    }

    if (!pkey) {
        LOG_ERROR << "Failed to read private key from file " << file_path.string();
        EVP_PKEY_free(pkey);
        return nullptr;
    }

    return pkey;
}

X509* load_x509_certificate(const std::filesystem::path& directory_path) {
    if (!std::filesystem::exists(directory_path) || !std::filesystem::is_directory(directory_path)) {
        LOG_ERROR << "Invalid or not existing container directory " << directory_path.string();
        return nullptr;
    }

    auto file_path = directory_path / kCertificateFileName;
    if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
        LOG_ERROR << "Invalid or not existing file " << file_path.string();
        return nullptr;
    }

    FILE* file{nullptr};

#if defined(_MSC_VER)
    fopen_s(&file, file_path.string().c_str(), "rb");
#else
    file = fopen(file_path.string().c_str(), "rb");
#endif

    if (!file) {
        LOG_ERROR << "Failed to open file " << file_path.string();
        return nullptr;
    }
    auto file_close{gsl::finally([file]() { fclose(file); })};

    X509* cert{PEM_read_X509(file, nullptr, nullptr, nullptr)};
    if (!cert) {
        LOG_ERROR << "Failed to read certificate from file " << file_path.string();
        X509_free(cert);
        return nullptr;
    }

    return cert;
}
bool validate_server_certificate(X509* cert, EVP_PKEY* pkey) {
    if (!cert) {
        LOG_ERROR << "Invalid X509 certificate";
        return false;
    }

    if (!pkey) {
        LOG_ERROR << "Invalid EVP_PKEY";
        return false;
    }

    if (X509_verify(cert, pkey) != 1) {
        LOG_ERROR << "Failed to verify certificate";
        return false;
    }

    return true;
}
SSL_CTX* generate_tls_context(TLSContextType type, const std::filesystem::path& directory_path,
                              const std::string& key_password) {
    SSL_CTX* ctx{nullptr};
    switch (type) {
        case TLSContextType::kServer: {
            ctx = SSL_CTX_new(TLS_server_method());
            break;
        }
        case TLSContextType::kClient: {
            ctx = SSL_CTX_new(TLS_client_method());
            break;
        }
        default: {
            LOG_ERROR << "Invalid TLS context type";
            return nullptr;
        }
    }

    if (!ctx) {
        LOG_ERROR << "Failed to create SSL context";
        return nullptr;
    }

    SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
    SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
    SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);
    SSL_CTX_set_options(ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
    SSL_CTX_set_options(ctx, SSL_OP_NO_RENEGOTIATION);
    SSL_CTX_set_ecdh_auto(ctx, 1);

    if (type == TLSContextType::kServer) {
        auto x509_cert{load_x509_certificate(directory_path)};
        auto rsa_pkey{load_rsa_private_key(directory_path, key_password)};

        if (!x509_cert || !rsa_pkey) {
            LOG_ERROR << "Failed to load certificate or private key from " << directory_path.string();
            if (x509_cert) X509_free(x509_cert);
            if (rsa_pkey) EVP_PKEY_free(rsa_pkey);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        if (!validate_server_certificate(x509_cert, rsa_pkey)) {
            LOG_ERROR << "Failed to validate certificate (mismatching private key)";
            X509_free(x509_cert);
            EVP_PKEY_free(rsa_pkey);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        if (SSL_CTX_use_certificate(ctx, x509_cert) != 1) {
            auto err{ERR_get_error()};
            print_ssl_error(err);
            LOG_ERROR << "Failed to use certificate for SSL server context";
            X509_free(x509_cert);
            EVP_PKEY_free(rsa_pkey);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        if (SSL_CTX_use_PrivateKey(ctx, rsa_pkey) != 1) {
            auto err{ERR_get_error()};
            print_ssl_error(err);
            LOG_ERROR << "Failed to use private key for SSL server context";
            X509_free(x509_cert);
            EVP_PKEY_free(rsa_pkey);
            SSL_CTX_free(ctx);
            return nullptr;
        }
    }

    return ctx;
}
bool validate_tls_requirements(const std::filesystem::path& directory_path, const std::string& key_password) {
    auto cert_path = directory_path / kCertificateFileName;
    auto key_path = directory_path / kPrivateKeyFileName;

    if (std::filesystem::exists(cert_path) && std::filesystem::is_regular_file(cert_path) &&
        std::filesystem::exists(key_path) && std::filesystem::is_regular_file(key_path)) {
        auto pkey{load_rsa_private_key(directory_path, key_password)};
        auto x509_cert{load_x509_certificate(directory_path)};
        if (pkey && x509_cert && validate_server_certificate(x509_cert, pkey)) {
            EVP_PKEY_free(pkey);
            X509_free(x509_cert);
            return true;
        }

        LOG_ERROR << "Failed to load certificate or private key from " << directory_path.string();
        if (pkey) EVP_PKEY_free(pkey);
        if (x509_cert) X509_free(x509_cert);
    }

    std::cout << "\n============================================================================================\n"
              << "A certificate (cert.pem) and or a private key (key.pem) are missing or invalid from \n"
              << directory_path.string() << std::endl;
    if (!ask_user_confirmation("Do you want me to (re)generate a new certificate and key ?")) {
        return false;
    }

    std::filesystem::remove(cert_path);  // Ensure removed
    std::filesystem::remove(key_path);

    LOG_TRACE << "Generating new certificate and key";
    auto pkey{generate_random_rsa_key_pair(static_cast<int>(kCertificateKeyLength))};
    if (!pkey) {
        LOG_ERROR << "Failed to generate RSA key pair";
        return false;
    }
    auto pkey_free{gsl::finally([pkey]() { EVP_PKEY_free(pkey); })};

    LOG_TRACE << "Generating self signed certificate";
    auto cert{generate_self_signed_certificate(pkey)};
    if (!cert) {
        LOG_ERROR << "Failed to generate self signed certificate";
        return false;
    }
    auto cert_free{gsl::finally([cert]() { X509_free(cert); })};

    LOG_TRACE << "Validating certificate";
    if (!validate_server_certificate(cert, pkey)) {
        LOG_ERROR << "Failed to validate certificate (mismatching private key)";
        return false;
    }

    LOG_TRACE << "Saving certificate and private key to files";
    if (!store_x509_certificate(cert, directory_path)) {
        LOG_ERROR << "Failed to save certificate to file " << cert_path.string();
        return false;
    }
    LOG_TRACE << "Saving private key to file";
    if (!store_rsa_key_pair(pkey, key_password, directory_path)) {
        LOG_ERROR << "Failed to save private key to file " << key_path.string();
        return false;
    }

    return true;
}
}  // namespace zenpp::network
