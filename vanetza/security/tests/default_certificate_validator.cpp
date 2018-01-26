#include <vanetza/common/clock.hpp>
#include <vanetza/common/runtime.hpp>
#include <vanetza/security/certificate_cache.hpp>
#include <vanetza/security/default_certificate_validator.hpp>
#include <vanetza/security/naive_certificate_provider.hpp>
#include <vanetza/security/trust_store.hpp>
#include <boost/variant/get.hpp>
#include <gtest/gtest.h>

using namespace vanetza;
using namespace vanetza::security;

class DefaultCertificateValidatorTest : public ::testing::Test
{
public:
    DefaultCertificateValidatorTest() :
        runtime(Clock::at("2016-08-01 00:00")),
        backend(create_backend("default")),
        cert_provider(runtime.now()),
        cert_cache(runtime),
        cert_validator(*backend, runtime.now(), trust_store, cert_cache)
    {
        trust_store.insert(cert_provider.root_certificate());
    }

protected:
    Runtime runtime;
    std::unique_ptr<Backend> backend;
    NaiveCertificateProvider cert_provider;
    std::vector<Certificate> roots;
    TrustStore trust_store;
    CertificateCache cert_cache;
    DefaultCertificateValidator cert_validator;
};

TEST_F(DefaultCertificateValidatorTest, validity_time)
{
    Certificate cert = cert_provider.generate_authorization_ticket([](Certificate& cert) {
        // remove any time constraint from certificate
        for (auto it = cert.validity_restriction.begin(); it != cert.validity_restriction.end(); ++it) {
            const ValidityRestriction& restriction = *it;
            ValidityRestrictionType type = get_type(restriction);
            switch (type) {
                case ValidityRestrictionType::Time_End:
                case ValidityRestrictionType::Time_Start_And_End:
                case ValidityRestrictionType::Time_Start_And_Duration:
                    it = cert.validity_restriction.erase(it);
                    break;
                default:
                    break;
            }
        }
    });

    CertificateValidity validity = cert_validator.check_certificate(cert);
    ASSERT_FALSE(validity);
    EXPECT_EQ(CertificateInvalidReason::BROKEN_TIME_PERIOD, validity.reason());
    // TODO: check that presence of exactly one time constraint is considered valid
}
