#ifndef  CRAILS_FRONT_SLOT_ELEMENT_HPP
# define CRAILS_FRONT_SLOT_ELEMENT_HPP

# include "anchorable_element.hpp"
# include "bindable.hpp"
# include <type_traits>

namespace Comet
{
  class SlotElement : public AnchorableElement
  {
    std::shared_ptr<IBindableView> element;
    IBindableView*                 element_ptr = nullptr;

  public:
    bool has_element() const
    {
      return element_ptr != nullptr;
    }

    IBindableView* get_element() const
    {
      return element_ptr;
    }

    void attach()
    {
      std::vector<Comet::Element*> element_list;

      element_list.push_back(get_element());
      attach_elements(element_list);
    }

    void set_element(IBindableView& el)
    {
      cleanup();
      element_ptr = &el;
      if (is_anchorable())
        attach();
    }

    template<typename ELEMENT>
    void set_element(std::shared_ptr<ELEMENT> pointer)
    {
      if (!std::is_base_of<IBindableView, ELEMENT>())
        throw std::logic_error("SlotElement::set_element called with a type not deriving from IBindableView.");
      if (pointer)
      {
        set_element(*static_cast<IBindableView*>(pointer.get()));
        element = std::static_pointer_cast<IBindableView>(pointer);
      }
      else
        cleanup();
    }

  private:
    void cleanup()
    {
      if (has_element())
        get_element()->destroy();
      element = nullptr;
      element_ptr = nullptr;
    }
  };
}

#endif
