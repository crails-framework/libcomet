#ifndef  CRAILS_FRONT_ROUTER_HPP
# define CRAILS_FRONT_ROUTER_HPP

# include <crails/router_base.hpp>
# include "signal.hpp"
# include <cheerp/client.h>
# include <map>
# include <memory>
# include <typeindex>

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
      std::shared_ptr<CONTROLLER> controller;

      if (router.template is_current_controller<CONTROLLER>())
      {
        controller = router.template get_current_controller<CONTROLLER>();
        controller->update_params(params);
      }
      else
      {
        controller = std::make_shared<CONTROLLER>(params);
        router.template set_current_controller<CONTROLLER>(controller);
      }
      controller->initialize().then([controller, method]()
      {
        ((controller.get())->*method)();
        controller->finalize();
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

    template<typename CONTROLLER>
    bool is_current_controller() const
    {
      return current_controller_type == std::type_index(typeid(CONTROLLER)) && current_controller != nullptr;
    }

    template<typename CONTROLLER>
    std::shared_ptr<CONTROLLER> get_current_controller() const
    {
      return std::static_pointer_cast<CONTROLLER>(current_controller);
    }

    template<typename CONTROLLER>
    void set_current_controller(std::shared_ptr<CONTROLLER> controller)
    {
      current_controller_type = std::type_index(typeid(CONTROLLER));
      current_controller = std::static_pointer_cast<void>(controller);
    }

    void reset_current_controller()
    {
      current_controller.reset();
      current_controller_type = std::type_index(typeid(void));
    }

  private:
    void on_hash_changed();

    std::string last_hash;
    std::shared_ptr<void> current_controller;
    std::type_index current_controller_type{typeid(void)};
  };
}

# define match_action(path, controller, action) \
  add_action_route<controller>(path, &controller::action)

#endif
