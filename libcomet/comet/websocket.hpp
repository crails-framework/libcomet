#ifndef  COMET_WEBSOCKET_HPP
# define COMET_WEBSOCKET_HPP

# include <cheerp/clientlib.h>
# include "events.hpp"
# include "object.hpp"

namespace Comet
{
  class WebSocket : public ObjectImpl<client::WebSocket>
  {
    JavascriptEvents events;
  public:
    explicit WebSocket(const Comet::String& url) :
      ObjectImpl<client::WebSocket>(new client::WebSocket(*url)),
      events(**this)
    {
      events.on("open",    std::bind(&WebSocket::on_open, this));
      events.on("close",   std::bind(&WebSocket::on_close, this));
      events.on("message", std::bind(&WebSocket::on_received_data, this, std::placeholders::_1));
      events.on("error",   std::bind(&WebSocket::on_error, this, std::placeholders::_1));
    }
    WebSocket(const WebSocket&) = delete;

    virtual void on_open() {}
    virtual void on_close() {}
    virtual void on_message(const std::string& message) {}
    virtual void on_error(const client::ErrorEvent*) {}

  private:
    void on_received_data(client::Event* event)
    {
      Comet::String message(reinterpret_cast<client::MessageEvent*>(event)->get_data<client::String*>());

      on_message(message);
    }
  };
}

#endif
