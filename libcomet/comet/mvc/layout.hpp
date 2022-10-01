#ifndef  COMET_MVC_LAYOUT_HPP
# define COMET_MVC_LAYOUT_HPP

# include "../globals.hpp"
# include "../exception.hpp"

namespace Comet
{
  template<typename ELEMENT>
  class Layout : public ELEMENT
  {
    Element get_container(const std::string& application_name)
    {
      auto container = body.find_one("[comet-app='" + application_name + "']");

      return container.is_undefined() ? body : container;
    }

  public:
    void attach_as_layout(const std::string& application_name)
    {
      auto container = get_container(application_name);

      if (container.is_undefined())
        throw std::runtime_error("Could not find the application container element");
      container.empty();
      ELEMENT::append_to(container);
      ELEMENT::bind_attributes();
      ELEMENT::trigger_binding_updates();
    }

    template<typename VIEW_ELEMENT>
    std::shared_ptr<VIEW_ELEMENT> render(std::shared_ptr<VIEW_ELEMENT> el)
    {
      ELEMENT::slot_content.set_element(el);
      ELEMENT::bind_attributes();
      ELEMENT::trigger_binding_updates();
      return el;
    }

    template<typename VIEW_ELEMENT, typename... ARGS>
    std::shared_ptr<VIEW_ELEMENT> render(ARGS... args)
    {
      auto view = std::make_shared<VIEW_ELEMENT>(args...);

      render(view);
      return view;
    }
  };
}

#endif
