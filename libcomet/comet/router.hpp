#ifndef  CRAILS_FRONT_ROUTER_HPP
# define CRAILS_FRONT_ROUTER_HPP

# include <crails/router_base.hpp>
# include "signal.hpp"
# include <cheerp/client.h>
# include <map>

namespace Comet
{
  typedef std::map<std::string, std::string> Params;
  typedef Crails::RouterBase<Params, std::function<void (const Params&)> > RouterBase;

  template<typename CONTROLLER, typename ROUTER>
  class ActionRoute
  {
  public:
    typedef void (CONTROLLER::*Method)();
    static void trigger(ROUTER& router, const Params& params, Method method)
    {
      auto controller = new CONTROLLER(params);
      auto listener = new Listener();

      controller->initialize().then([controller, method]()
      {
        (controller.get()->*method)();
        controller->finalize();
      });
      listener->listen_to(router.on_before_route_execution, [controller, listener](const std::string&)
      {
        delete controller;
        delete listener;
      });
    }
  };

  class Router : public RouterBase
  {
  public:
    Router();

    Signal<const std::string&> on_before_route_execution;
    Signal<const std::string&> on_route_executed;
    Signal<const std::string&> on_route_not_found;

    bool navigate(const std::string& path, bool trigger = true);

    void initialize();
    void start();
    std::string get_current_path() const;

    Router& match(const std::string& path, RouterBase::Action callback)
    {
      RouterBase::match("", path, callback);
      return *this;
    }

    template<typename CONTROLLER>
    Router& add_action_route(const std::string& path, typename ActionRoute<CONTROLLER, Router>::Method method)
    {
      return match(path, [this, method](const Comet::Params& params)
      { ActionRoute<CONTROLLER, Router>::trigger(*this, params, method); });
    }

  private:
    void on_hash_changed();

    std::string last_hash;
  };
}

# define match_action(path, controller, action) \
  add_action_route<controller>(path, &controller::action)

#endif
