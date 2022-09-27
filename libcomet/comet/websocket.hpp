#ifndef  COMET_WEBSOCKET_HPP
# define COMET_WEBSOCKET_HPP

# include <cheerp/clientlib.h>
# include "events.hpp"
# include "object.hpp"
# include "signal.hpp"

namespace Comet
{
  class WebSocket : public ObjectImpl<client::WebSocket>
  {
    JavascriptEvents events;
  public:
    Signal<std::string> message_received;
    Signal<void>        opened;
    Signal<void>        closed;

    explicit WebSocket(const Comet::String& url) :
      ObjectImpl<client::WebSocket>(new client::WebSocket(*url)),
      events(**this)
    {
      events.on("open",    [this](client::Event* e) { on_open(e); });
      events.on("close",   [this](client::Event* e) { on_close(e); });
      events.on("message", [this](client::MessageEvent* e) { on_received_data(e); });
      events.on("error",   [this](client::ErrorEvent* e) { on_error(e); });
    }

    virtual ~WebSocket()
    {
      (*this)->close();
    }

    WebSocket(const WebSocket&) = delete;

    virtual void send(Comet::String message) { (*this)->send(*message); }
    virtual void on_open(client::Event*) { opened.trigger(); }
    virtual void on_close(client::Event*) { closed.trigger(); }
    virtual void on_error(client::ErrorEvent*) {}
    virtual void on_received_data(client::MessageEvent* event)
    {
      Comet::String message(event->get_data<client::String*>());

      message_received.trigger(message);
    }
  };
}

#endif
