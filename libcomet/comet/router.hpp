#ifndef  CRAILS_FRONT_ROUTER_HPP
# define CRAILS_FRONT_ROUTER_HPP

# include <crails/router_base.hpp>
# include "signal.hpp"
# include <cheerp/client.h>
# include <map>

namespace Comet
{
  typedef std::map<std::string, std::string> Params;

  class Router : public Crails::RouterBase<Params, std::function<void (const Params&)> >
  {
  public:
    Router();

    Signal<const std::string&> on_route_executed;
    Signal<const std::string&> on_route_not_found;

    bool navigate(const std::string& path, bool trigger = true);

    void initialize();
    void start();
    std::string get_current_path() const;

  private:
    void on_hash_changed();

    std::string last_hash;
  };

  template<typename CONTROLLER>
  class ActionRoute
  {
    typedef void (CONTROLLER::*Method)();
  public:
    static void trigger(const Params& params, Method method)
    {
      auto controller = std::make_shared<CONTROLLER>(params);

      controller->initialize().then([controller, method]()
      {
        (controller.get()->*method)();
        controller->finalize();
      });
    }
  };
}

# define match_action(path, controller, action) \
  match("", path, [](const Comet::Params& params) { ActionRoute<controller>::trigger(params, &controller::action); })

#endif
