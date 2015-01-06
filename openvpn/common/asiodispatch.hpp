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

// These classes define function objects to be used as asynchronous callbacks
// for Asio methods.  Think of these as optimized special cases of function
// objects that could be more generally (but perhaps less optimally) defined
// with boost::bind.

#ifndef OPENVPN_COMMON_ASIODISPATCH_H
#define OPENVPN_COMMON_ASIODISPATCH_H

#include <openvpn/common/types.hpp>
#include <openvpn/common/rc.hpp>

namespace openvpn {
  // Dispatcher for asio async_write

  template <typename C, typename Handler>
  class AsioDispatchWrite
  {
  public:
    AsioDispatchWrite(Handler handle_write, C* obj)
      : handle_write_(handle_write), obj_(obj) {}

    void operator()(const boost::system::error_code& error, const size_t bytes_sent)
    {
      (obj_.get()->*handle_write_)(error, bytes_sent);
    }

  private:
    Handler handle_write_;
    boost::intrusive_ptr<C> obj_;
  };

  template <typename C, typename Handler>
  AsioDispatchWrite<C, Handler> asio_dispatch_write(Handler handle_write, C* obj)
  {
    return AsioDispatchWrite<C, Handler>(handle_write, obj);
  }

  // Dispatcher for asio async_read

  template <typename C, typename Handler, typename Data>
  class AsioDispatchRead
  {
  public:
    AsioDispatchRead(Handler handle_read, C* obj, Data data)
      : handle_read_(handle_read), obj_(obj), data_(data) {}

    void operator()(const boost::system::error_code& error, const size_t bytes_recvd)
    {
      (obj_.get()->*handle_read_)(data_, error, bytes_recvd);
    }

  private:
    Handler handle_read_;
    boost::intrusive_ptr<C> obj_;
    Data data_;
  };

  template <typename C, typename Handler, typename Data>
  AsioDispatchRead<C, Handler, Data> asio_dispatch_read(Handler handle_read, C* obj, Data data)
  {
    return AsioDispatchRead<C, Handler, Data>(handle_read, obj, data);
  }

  // Dispatcher for asio async_wait with argument

  template <typename C, typename Handler, typename Data>
  class AsioDispatchTimerArg
  {
  public:
    AsioDispatchTimerArg(Handler handler, C* obj, Data data)
      : handler_(handler), obj_(obj), data_(data) {}

    void operator()(const boost::system::error_code& error)
    {
      (obj_.get()->*handler_)(data_, error);
    }

  private:
    Handler handler_;
    boost::intrusive_ptr<C> obj_;
    Data data_;
  };

  template <typename C, typename Handler, typename Data>
  AsioDispatchTimerArg<C, Handler, Data> asio_dispatch_timer_arg(Handler handler, C* obj, Data data)
  {
    return AsioDispatchTimerArg<C, Handler, Data>(handler, obj, data);
  }

  // Dispatcher for asio async_wait without argument

  template <typename C, typename Handler>
  class AsioDispatchTimer
  {
  public:
    AsioDispatchTimer(Handler handler, C* obj)
      : handler_(handler), obj_(obj) {}

    void operator()(const boost::system::error_code& error)
    {
      (obj_.get()->*handler_)(error);
    }

  private:
    Handler handler_;
    boost::intrusive_ptr<C> obj_;
  };

  template <typename C, typename Handler>
  AsioDispatchTimer<C, Handler> asio_dispatch_timer(Handler handler, C* obj)
  {
    return AsioDispatchTimer<C, Handler>(handler, obj);
  }

  // Dispatcher for asio async_connect with argument

  template <typename C, typename Handler, typename Data>
  class AsioDispatchConnectArg
  {
  public:
    AsioDispatchConnectArg(Handler handler, C* obj, Data data)
      : handler_(handler), obj_(obj), data_(data) {}

    void operator()(const boost::system::error_code& error)
    {
      (obj_.get()->*handler_)(data_, error);
    }

  private:
    Handler handler_;
    boost::intrusive_ptr<C> obj_;
    Data data_;
  };

  template <typename C, typename Handler, typename Data>
  AsioDispatchConnectArg<C, Handler, Data> asio_dispatch_connect_arg(Handler handler, C* obj, Data data)
  {
    return AsioDispatchConnectArg<C, Handler, Data>(handler, obj, data);
  }

  // Dispatcher for asio async_connect without argument

  template <typename C, typename Handler>
  class AsioDispatchConnect
  {
  public:
    AsioDispatchConnect(Handler handler, C* obj)
      : handler_(handler), obj_(obj) {}

    void operator()(const boost::system::error_code& error)
    {
      (obj_.get()->*handler_)(error);
    }

  private:
    Handler handler_;
    boost::intrusive_ptr<C> obj_;
  };

  template <typename C, typename Handler>
  AsioDispatchConnect<C, Handler> asio_dispatch_connect(Handler handler, C* obj)
  {
    return AsioDispatchConnect<C, Handler>(handler, obj);
  }

  // Dispatcher for asio async_connect (ComposedConnectHandler) without argument

  template <typename C, typename Handler>
  class AsioDispatchComposedConnect
  {
  public:
    AsioDispatchComposedConnect(Handler handler, C* obj)
      : handler_(handler), obj_(obj) {}

    void operator()(const boost::system::error_code& error,
		    boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    {
      (obj_.get()->*handler_)(error, endpoint_iterator);
    }

  private:
    Handler handler_;
    boost::intrusive_ptr<C> obj_;
  };

  template <typename C, typename Handler>
  AsioDispatchComposedConnect<C, Handler> asio_dispatch_composed_connect(Handler handler, C* obj)
  {
    return AsioDispatchComposedConnect<C, Handler>(handler, obj);
  }

  // Dispatcher for asio async_accept with argument

  template <typename C, typename Handler, typename Data>
  class AsioDispatchAcceptArg
  {
  public:
    AsioDispatchAcceptArg(Handler handler, C* obj, Data data)
      : handler_(handler), obj_(obj), data_(data) {}

    void operator()(const boost::system::error_code& error)
    {
      (obj_.get()->*handler_)(data_, error);
    }

  private:
    Handler handler_;
    boost::intrusive_ptr<C> obj_;
    Data data_;
  };

  template <typename C, typename Handler, typename Data>
  AsioDispatchAcceptArg<C, Handler, Data> asio_dispatch_accept_arg(Handler handler, C* obj, Data data)
  {
    return AsioDispatchAcceptArg<C, Handler, Data>(handler, obj, data);
  }

  // Dispatcher for asio post with argument

  template <typename C, typename Handler, typename Data>
  class AsioDispatchPostArg
  {
  public:
    AsioDispatchPostArg(Handler handler, C* obj, Data data)
      : handler_(handler), obj_(obj), data_(data) {}

    void operator()()
    {
      (obj_.get()->*handler_)(data_);
    }

  private:
    Handler handler_;
    boost::intrusive_ptr<C> obj_;
    Data data_;
  };

  template <typename C, typename Handler, typename Data>
  AsioDispatchPostArg<C, Handler, Data> asio_dispatch_post_arg(Handler handler, C* obj, Data data)
  {
    return AsioDispatchPostArg<C, Handler, Data>(handler, obj, data);
  }

  // Dispatcher for asio post without argument

  template <typename C, typename Handler>
  class AsioDispatchPost
  {
  public:
    AsioDispatchPost(Handler handler, C* obj)
      : handler_(handler), obj_(obj) {}

    void operator()()
    {
      (obj_.get()->*handler_)();
    }

  private:
    Handler handler_;
    boost::intrusive_ptr<C> obj_;
  };

  template <typename C, typename Handler>
  AsioDispatchPost<C, Handler> asio_dispatch_post(Handler handler, C* obj)
  {
    return AsioDispatchPost<C, Handler>(handler, obj);
  }

  // Dispatcher for asynchronous resolver

  template <typename C, typename Handler, typename EndpointIterator>
  class AsioDispatchResolve
  {
  public:
    AsioDispatchResolve(Handler handler, C* obj)
      : handler_(handler), obj_(obj) {}

    void operator()(const boost::system::error_code& error, EndpointIterator iter)
    {
      (obj_.get()->*handler_)(error, iter);
    }

  private:
    Handler handler_;
    boost::intrusive_ptr<C> obj_;
  };

  // Dispatcher for asio signal

  template <typename C, typename Handler>
  class AsioDispatchSignal
  {
  public:
    AsioDispatchSignal(Handler handler, C* obj)
      : handler_(handler), obj_(obj) {}

    void operator()(const boost::system::error_code& error, int signal_number)
    {
      (obj_.get()->*handler_)(error, signal_number);
    }

  private:
    Handler handler_;
    boost::intrusive_ptr<C> obj_;
  };

  template <typename C, typename Handler>
  AsioDispatchSignal<C, Handler> asio_dispatch_signal(Handler handler, C* obj)
  {
    return AsioDispatchSignal<C, Handler>(handler, obj);
  }

  // General purpose dispatcher with data

  template <typename C, typename Handler, typename Data>
  class SimpleDispatch
  {
  public:
    SimpleDispatch(Handler handler, C* obj)
      : handler_(handler), obj_(obj) {}

    void operator()(Data data)
    {
      (obj_->*handler_)(data);
    }

  private:
    Handler handler_;
    C* obj_;
  };

} // namespace openvpn

#endif // OPENVPN_COMMON_ASIODISPATCH_H
