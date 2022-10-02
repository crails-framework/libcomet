#ifndef  COMET_MVC_MODEL_HPP
# define COMET_MVC_MODEL_HPP

# include "../signal.hpp"
# include "../promise.hpp"
# include "../http.hpp"
# include "id_type.hpp"
# include <memory>
# include <functional>

namespace Comet
{
  namespace Http { class Response; }
  typedef std::function<void (std::shared_ptr<Http::Response>)> HttpResponseCallback;

  template<typename ID_TRAIT = LongIdTrait>
  class Model : public Listener
  {
  public:
    typedef typename ID_TRAIT::type id_type;

    Signal<void> removed;
    Signal<void> synced;

    virtual id_type     get_id() const { return id; }
    virtual std::string get_url() const = 0;
    virtual std::string get_resource_name() const { return ""; }

    Promise fetch()
    {
      auto request = Http::Request::get(get_url());

      request->set_headers({{"Accept", get_content_type()}});
      return request->send().then([this, request]()
      {
        auto response = request->get_response();

        if (response->ok())
        {
          if (response->has_body())
            parse(response->get_response_text());
          synced.trigger();
        }
      });
    }

    Promise save()
    {
      auto request = Http::Request::make(get_id() == ID_TRAIT::null_id ? "post" : "put", get_url());

      request->set_headers({
        {"Content-Type", get_content_type()},
        {"Accept",       get_content_type()}
      });
      request->set_body(get_payload());
      return request->send().then([this, request]()
      {
        auto response = request->get_response();

        if (response->ok())
        {
          if (response->has_body())
            parse(response->get_response_text());
          synced.trigger();
        }
      });
    }

    Promise destroy(HttpResponseCallback callback = HttpResponseCallback())
    {
      auto request = Http::Request::_delete(get_url());

      request->set_headers({{"Accept", get_content_type()}});
      return request->send().then([this, request, callback]()
      {
        auto response = request->get_response();

        if (response->ok())
          removed.trigger();
        if (callback)
          callback(response);
      });
    }

    virtual void parse(const std::string& str) = 0;

  protected:
    virtual std::string get_payload() = 0;
    virtual std::string get_content_type() const = 0;

    id_type id = ID_TRAIT::null_id;
  };
}

#endif
