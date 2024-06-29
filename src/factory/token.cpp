#include "token.h"

#include <stdexcept>
#include <random>

#include <openssl/bn.h>
#include <openssl/bnerr.h>

namespace qingliao
{
    struct BigNumImpl
    {
        BIGNUM* bignum = nullptr;
        int is_prime = 0;
    };

    BigNum::BigNum(long long num):
        m_impl(std::make_shared<BigNumImpl>())
    {
        m_impl->bignum = BN_new();
        BN_set_word(m_impl->bignum, num);
    }

    BigNum::BigNum(const std::string& num, bool is_dec_not_hex) :
        m_impl(std::make_shared<BigNumImpl>())
    {
        m_impl->bignum = BN_new();
        
        if (is_dec_not_hex)
            BN_dec2bn(&m_impl->bignum, num.c_str());
        else
            BN_hex2bn(&m_impl->bignum, num.c_str());
    }

    BigNum::BigNum(const BigNum& b):
        m_impl(std::make_shared<BigNumImpl>())
    {
        m_impl->bignum = BN_new();
        m_impl->is_prime = b.m_impl->is_prime;

        if (!BN_copy(m_impl->bignum, b.m_impl->bignum))
            throw std::runtime_error("Copy error");
    }

    BigNum::BigNum(BigNum&& b) noexcept :
        m_impl(std::move(b.m_impl)) {}

    BigNum::~BigNum()
    {
        BN_free(m_impl->bignum);
    }

    std::string BigNum::getNum(bool is_dec_not_hex) const
    {
        if (is_dec_not_hex)
            return { BN_bn2dec(m_impl->bignum) };
        else
            return { BN_bn2hex(m_impl->bignum) };
    }

    BigNum BigNum::gcd(const BigNum& b) const
    {
        BigNum local;
        std::shared_ptr<BN_CTX> ctx(BN_CTX_new(),
            [](BN_CTX* p) { BN_CTX_free(p); });

        if (!BN_gcd(local.m_impl->bignum, m_impl->bignum, b.m_impl->bignum, ctx.get()))
            throw std::runtime_error("GCD error");

        return local;
    }

    BigNum BigNum::exp(const BigNum& b) const
    {
        BigNum local;
        std::shared_ptr<BN_CTX> ctx(BN_CTX_new(),
            [](BN_CTX* p) { BN_CTX_free(p); });

        if (!BN_exp(local.m_impl->bignum, m_impl->bignum, b.m_impl->bignum, ctx.get()))
            throw std::runtime_error("Exp error");

        return local;
    }

    BigNum BigNum::sqrt() const
    {
        BigNum local;
        std::shared_ptr<BN_CTX> ctx(BN_CTX_new(),
            [](BN_CTX* p) { BN_CTX_free(p); });

        if (!BN_sqr(local.m_impl->bignum, m_impl->bignum, ctx.get()))
            throw std::runtime_error("Sqrt error");

        return local;
    }

    BigNum BigNum::generate_rand(int bits)
    {
        BigNum local;
        std::mt19937 mt(std::random_device{}());
        if (!BN_rand(local.m_impl->bignum, bits, 1, mt()&1))
            throw std::runtime_error("Generate_rand error");

        return local;
    }

    BigNum BigNum::generate_prime(int bits)
    {
        BigNum local;
        if (!BN_generate_prime_ex(local.m_impl->bignum,
            bits, 1, nullptr, nullptr, nullptr))
            throw std::runtime_error("Generate_prime error");

        return local;
    }

    bool BigNum::is_prime() const
    {
        if (m_impl->is_prime)
            return m_impl->is_prime == 2;

        std::shared_ptr<BN_CTX> ctx(BN_CTX_new(),
            [](BN_CTX* p) { BN_CTX_free(p); });
        int code = BN_is_prime_ex(m_impl->bignum, 10, ctx.get(), nullptr);
        if (code == -1)
            throw std::runtime_error("Check_prime error");
        m_impl->is_prime = code == 1 ? 2 : 1;

        return code == 1;
    }

    BigNum& BigNum::operator=(const BigNum& b)
    {
        if (!BN_copy(m_impl->bignum, b.m_impl->bignum))
            throw std::runtime_error("Copy error");
        m_impl->is_prime = b.m_impl->is_prime;

        return *this;
    }

    BigNum& BigNum::operator=(BigNum&& b) noexcept
    {
        BN_free(m_impl->bignum);
        m_impl = std::move(b.m_impl);

        return *this;
    }

    BigNum& BigNum::operator++()
    {
        static BigNum one(1);
        m_impl->is_prime = 0;

        if (!BN_add(m_impl->bignum, m_impl->bignum, one.m_impl->bignum))
            throw std::runtime_error("Add error");

        return *this;
    }

    BigNum& BigNum::operator--()
    {
        static BigNum one(1);
        m_impl->is_prime = 0;

        if (!BN_sub(m_impl->bignum, m_impl->bignum, one.m_impl->bignum))
            throw std::runtime_error("Sub error");

        return *this;
    }

    BigNum operator+(const BigNum& a, const BigNum& b)
    {
        BigNum local(a);
        local.m_impl->is_prime = 0;

        if (!BN_add(local.m_impl->bignum, a.m_impl->bignum, b.m_impl->bignum))
            throw std::runtime_error("Add error");

        return local;
    }

    BigNum operator+(const BigNum& b, BigNum&& a)
    {
        BigNum local(std::move(a));
        local.m_impl->is_prime = 0;

        if (!BN_add(local.m_impl->bignum, local.m_impl->bignum, b.m_impl->bignum))
            throw std::runtime_error("Add error");

        return local;
    }

    BigNum operator+(BigNum&& a, const BigNum& b)
    {
        BigNum local(std::move(a));
        local.m_impl->is_prime = 0;

        if (!BN_add(local.m_impl->bignum, local.m_impl->bignum, b.m_impl->bignum))
            throw std::runtime_error("Add error");

        return local;
    }

    BigNum operator+(BigNum&& a, BigNum&& b)
    {
        BigNum local(std::move(a));
        local.m_impl->is_prime = 0;

        if (!BN_add(local.m_impl->bignum, local.m_impl->bignum, b.m_impl->bignum))
            throw std::runtime_error("Add error");

        return local;
    }

    BigNum operator-(const BigNum& a, const BigNum& b)
    {
        BigNum local(a);
        local.m_impl->is_prime = 0;

        if (!BN_sub(local.m_impl->bignum, a.m_impl->bignum, b.m_impl->bignum))
            throw std::runtime_error("Sub error");

        return local;
    }

    BigNum operator-(const BigNum& a, BigNum&& b)
    {
        BigNum local(a);
        local.m_impl->is_prime = 0;

        if (!BN_sub(local.m_impl->bignum, local.m_impl->bignum, b.m_impl->bignum))
            throw std::runtime_error("Sub error");

        return local;
    }

    BigNum operator-(BigNum&& a, const BigNum& b)
    {
        BigNum local(std::move(a));
        local.m_impl->is_prime = 0;

        if (!BN_sub(local.m_impl->bignum, local.m_impl->bignum, b.m_impl->bignum))
            throw std::runtime_error("Sub error");

        return local;
    }

    BigNum operator-(BigNum&& a, BigNum&& b)
    {
        BigNum local(std::move(a));
        local.m_impl->is_prime = 0;

        if (!BN_sub(local.m_impl->bignum, local.m_impl->bignum, b.m_impl->bignum))
            throw std::runtime_error("Sub error");

        return local;
    }

    BigNum operator*(const BigNum& a, const BigNum& b)
    {
        BigNum local(a);
        local.m_impl->is_prime = 0;

        std::shared_ptr<BN_CTX> ctx(BN_CTX_new(),
            [](BN_CTX* p) { BN_CTX_free(p); });

        if (!BN_mul(local.m_impl->bignum, a.m_impl->bignum, b.m_impl->bignum, ctx.get()))
            throw std::runtime_error("Mul error");

        return local;
    }

    BigNum operator/(const BigNum& a, const BigNum& b)
    {
        BigNum local(a);
        local.m_impl->is_prime = 0;

        std::shared_ptr<BN_CTX> ctx(BN_CTX_new(),
            [](BN_CTX* p) { BN_CTX_free(p); });

        if (!BN_div(local.m_impl->bignum, nullptr, a.m_impl->bignum, b.m_impl->bignum, ctx.get()));
            throw std::runtime_error("Div error");

        return local;
    }

    BigNum operator%(const BigNum& a, const BigNum& b)
    {
        BigNum local(a);
        local.m_impl->is_prime = 0;

        std::shared_ptr<BN_CTX> ctx(BN_CTX_new(),
            [](BN_CTX* p) { BN_CTX_free(p); });

        if (!BN_mod(local.m_impl->bignum, a.m_impl->bignum, b.m_impl->bignum, ctx.get()));
        throw std::runtime_error("Div error");

        return local;
    }

    bool operator==(const BigNum& a, const BigNum& b)
    {
        return !BN_cmp(a.m_impl->bignum, b.m_impl->bignum);
    }

    bool operator<(const BigNum& a, const BigNum& b)
    {
        return BN_cmp(a.m_impl->bignum, b.m_impl->bignum) == -1;
    }

    bool operator>(const BigNum& a, const BigNum& b)
    {
        return BN_cmp(a.m_impl->bignum, b.m_impl->bignum) == 1;
    }

    bool operator!=(const BigNum& a, const BigNum& b)
    {
        return !operator==(a, b);
    }

    void swap(BigNum& a, BigNum& b)
    {
        std::swap(a.m_impl, b.m_impl);
    }
}