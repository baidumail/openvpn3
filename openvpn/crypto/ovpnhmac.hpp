//    OpenVPN -- An application to securely tunnel IP networks
//               over a single port, with support for SSL/TLS-based
//               session authentication and key exchange,
//               packet encryption, packet authentication, and
//               packet compression.
//
//    Copyright (C) 2012-2015 OpenVPN Technologies, Inc.
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License Version 3
//    as published by the Free Software Foundation.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program in the COPYING file.
//    If not, see <http://www.gnu.org/licenses/>.

// OpenVPN HMAC classes

#ifndef OPENVPN_CRYPTO_OVPNHMAC_H
#define OPENVPN_CRYPTO_OVPNHMAC_H

#include <string>

#include <openvpn/common/types.hpp>
#include <openvpn/common/exception.hpp>
#include <openvpn/common/rc.hpp>
#include <openvpn/common/memneq.hpp>
#include <openvpn/crypto/static_key.hpp>
#include <openvpn/crypto/cryptoalgs.hpp>

namespace openvpn {

  // OpenVPN protocol HMAC usage for HMAC/CBC integrity checking and tls-auth

  template <typename CRYPTO_API>
  class OvpnHMAC
  {
  public:
    OPENVPN_SIMPLE_EXCEPTION(ovpn_hmac_context_digest_size);
    OPENVPN_SIMPLE_EXCEPTION(ovpn_hmac_context_bad_sizing);

  public:
    OvpnHMAC() {}

    OvpnHMAC(const CryptoAlgs::Type digest, const StaticKey& key)
    {
      init(digest, key);
    }

    bool defined() const { return ctx.is_initialized(); }

    // size of out buffer to pass to hmac
    size_t output_size() const
    {
      return ctx.size();
    }

    void init(const CryptoAlgs::Type digest, const StaticKey& key)
    {
      const CryptoAlgs::Alg& alg = CryptoAlgs::get(digest);

      // check that key is large enough
      if (key.size() < alg.size())
	throw ovpn_hmac_context_digest_size();

      // initialize HMAC context with digest type and key
      ctx.init(digest, key.data(), alg.size());
    }

    void hmac(unsigned char *out, const size_t out_size,
	      const unsigned char *in, const size_t in_size)
    {
      ctx.reset();
      ctx.update(in, in_size);
      ctx.final(out);
    }

    // Special HMAC for OpenVPN control packets

    void ovpn_hmac_gen(unsigned char *data, const size_t data_size,
		       const size_t l1, const size_t l2, const size_t l3)
    {
      if (ovpn_hmac_pre(data, data_size, l1, l2, l3))
	ctx.final(data + l1);
      else
	throw ovpn_hmac_context_bad_sizing();
    }

    // verify the HMAC generated by ovpn_hmac_gen, return true if verified
    bool ovpn_hmac_cmp(const unsigned char *data, const size_t data_size,
		       const size_t l1, const size_t l2, const size_t l3)
    {
      unsigned char local_hmac[CRYPTO_API::HMACContext::MAX_HMAC_SIZE];
      if (ovpn_hmac_pre(data, data_size, l1, l2, l3))
	{
	  ctx.final(local_hmac);
	  return !crypto::memneq(data + l1, local_hmac, l2);
	}
      else
	return false;
    }

  private:
    // Convoluting OpenVPN control channel packets for HMAC:
    // <-- L1  -->   <-L2>   <L3>
    // [OP]  [PSID]  [HMAC]  [PID] [...]  -> canonical order
    //
    // [HMAC] [PID] [OP] [PSID] [...]     -> HMAC order

    bool ovpn_hmac_pre(const unsigned char *data, const size_t data_size,
		       const size_t l1, const size_t l2, const size_t l3)
    {
      const size_t lsum = l1 + l2 + l3;
      if (lsum > data_size || l2 != ctx.size())
	return false;
      ctx.reset();
      ctx.update(data + l1 + l2, l3);
      ctx.update(data, l1);
      ctx.update(data + lsum, data_size - lsum);
      return true;
    }

    typename CRYPTO_API::HMACContext ctx;
  };

  // OvpnHMAC wrapper API using dynamic polymorphism

  class OvpnHMACInstance : public RC<thread_unsafe_refcount>
  {
  public:
    typedef boost::intrusive_ptr<OvpnHMACInstance> Ptr;

    virtual void init(const StaticKey& key) = 0;

    virtual size_t output_size() const = 0;

    virtual void ovpn_hmac_gen(unsigned char *data, const size_t data_size,
			       const size_t l1, const size_t l2, const size_t l3) = 0;

    virtual bool ovpn_hmac_cmp(const unsigned char *data, const size_t data_size,
			       const size_t l1, const size_t l2, const size_t l3) = 0;
  };

  class OvpnHMACContext : public RC<thread_unsafe_refcount>
  {
  public:
    typedef boost::intrusive_ptr<OvpnHMACContext> Ptr;

    virtual size_t size() const = 0;

    virtual OvpnHMACInstance::Ptr new_obj() = 0;
  };

  class OvpnHMACFactory : public RC<thread_unsafe_refcount>
  {
  public:
    typedef boost::intrusive_ptr<OvpnHMACFactory> Ptr;

    virtual OvpnHMACContext::Ptr new_obj(const CryptoAlgs::Type digest_type) = 0;
  };

  // OvpnHMAC wrapper implementation using dynamic polymorphism

  template <typename CRYPTO_API>
  class CryptoOvpnHMACInstance : public OvpnHMACInstance
  {
  public:
    CryptoOvpnHMACInstance(const CryptoAlgs::Type digest_arg)
      : digest(digest_arg)
    {
    }

    virtual void init(const StaticKey& key)
    {
      ovpn_hmac.init(digest, key);
    }

    virtual size_t output_size() const
    {
      return ovpn_hmac.output_size();
    }

    virtual void ovpn_hmac_gen(unsigned char *data, const size_t data_size,
			       const size_t l1, const size_t l2, const size_t l3)
    {
      ovpn_hmac.ovpn_hmac_gen(data, data_size, l1, l2, l3);
    }

    virtual bool ovpn_hmac_cmp(const unsigned char *data, const size_t data_size,
			       const size_t l1, const size_t l2, const size_t l3)
    {
      return ovpn_hmac.ovpn_hmac_cmp(data, data_size, l1, l2, l3);
    }

  private:
    typename CryptoAlgs::Type digest;
    OvpnHMAC<CRYPTO_API> ovpn_hmac;
  };

  template <typename CRYPTO_API>
  class CryptoOvpnHMACContext : public OvpnHMACContext
  {
  public:
    CryptoOvpnHMACContext(const CryptoAlgs::Type digest_type)
      : digest(CryptoAlgs::legal_dc_digest(digest_type))
    {
    }

    virtual size_t size() const
    {
      return CryptoAlgs::size(digest);
    }

    virtual OvpnHMACInstance::Ptr new_obj()
    {
      return new CryptoOvpnHMACInstance<CRYPTO_API>(digest);
    }

  private:
    CryptoAlgs::Type digest;
  };

  template <typename CRYPTO_API>
  class CryptoOvpnHMACFactory : public OvpnHMACFactory
  {
  public:
    virtual OvpnHMACContext::Ptr new_obj(const CryptoAlgs::Type digest_type)
    {
      return new CryptoOvpnHMACContext<CRYPTO_API>(digest_type);
    }
  };

}

#endif
