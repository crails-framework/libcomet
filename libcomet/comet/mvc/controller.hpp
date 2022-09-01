#ifndef  COMET_MVC_CONTROLLER_HPP
# define COMET_MVC_CONTROLLER_HPP

# include "../promise.hpp"
# include "../http.hpp"
# include "../router.hpp"
# include "layout.hpp"
# include <memory>

namespace Comet
{
  class Controller : public std::enable_shared_from_this<Controller>
  {
  protected:
    const Params& params;

  public:
    Controller(const Params& p) : params(p)
    {
    }

    virtual ~Controller()
    {
    }

    virtual std::string get_application_name()
    {
      return "main";
    }

    template<typename ELEMENT>
    Layout<ELEMENT>& use_layout()
    {
      auto& layout = get_layout<ELEMENT>();

      layout.attach_as_layout(get_application_name());
      return layout;
    }

    template<typename ELEMENT>
    Layout<ELEMENT>& get_layout()
    {
      static Layout<ELEMENT> layout;

      return layout;
    }

    Comet::Promise initialize() { return Promise::solved_promise(); }
    Comet::Promise finalize()   { return Promise::solved_promise(); }
  };
}

#endif
