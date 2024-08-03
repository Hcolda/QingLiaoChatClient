#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <memory>

namespace qingliao
{
    struct BigNumImpl;

    class BigNum
    {
    public:
        BigNum(long long num = 0);
        BigNum(const std::string& num, bool is_dec_not_hex);
        BigNum(const BigNum&);
        BigNum(BigNum&&) noexcept;
        ~BigNum();

        std::string getNum(bool is_dec_not_hex) const;

        BigNum gcd(const BigNum&) const;
        BigNum exp(const BigNum&) const;
        BigNum sqrt() const;

        static BigNum generate_rand(int bits);
        static BigNum generate_prime(int bits);

        bool is_prime() const;

        BigNum& operator=(const BigNum&);
        BigNum& operator=(BigNum&&) noexcept;

        BigNum& operator++();
        BigNum& operator--();

        friend BigNum operator+(const BigNum&, const BigNum&);
        friend BigNum operator+(const BigNum&, BigNum&&);
        friend BigNum operator+(BigNum&&, const BigNum&);
        friend BigNum operator+(BigNum&&, BigNum&&);

        friend BigNum operator-(const BigNum&, const BigNum&);
        friend BigNum operator-(const BigNum&, BigNum&&);
        friend BigNum operator-(BigNum&&, const BigNum&);
        friend BigNum operator-(BigNum&&, BigNum&&);

        friend BigNum operator*(const BigNum&, const BigNum&);
        /*friend BigNum operator*(const BigNum&, BigNum&&);
        friend BigNum operator*(BigNum&&, const BigNum&);
        friend BigNum operator*(BigNum&&, BigNum&&);*/

        friend BigNum operator/(const BigNum&, const BigNum&);

        friend BigNum operator%(const BigNum&, const BigNum&);

        // friend BigNum operator&(const BigNum&, const BigNum&);

        // friend BigNum operator|(const BigNum&, const BigNum&);

        // friend BigNum operator^(const BigNum&, const BigNum&);

        friend bool operator==(const BigNum&, const BigNum&);
        friend bool operator<(const BigNum&, const BigNum&);
        friend bool operator>(const BigNum&, const BigNum&);
        friend bool operator!=(const BigNum&, const BigNum&);

        friend void swap(BigNum&, BigNum&);

    private:
        std::shared_ptr<BigNumImpl> m_impl;
    };

    template<class TokenType = size_t>
    class TokenMaker
    {
    public:
        TokenMaker() = default;
        ~TokenMaker() = default;

        TokenType makeNewToken()
        {
            return m_token_recorder++;
        }

    private:
        TokenType m_token_recorder;
    };
}

#endif // !TOKEN_H
