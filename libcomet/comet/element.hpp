#ifndef  COMET_ELEMENT_HPP
# define COMET_ELEMENT_HPP

# include <cheerp/client.h>
# include <cheerp/clientlib.h>
# include <map>
# include <list>
# include "object.hpp"
# include "events.hpp"
# include "append_semantics.hpp"

namespace Comet
{
  class Element : public ObjectImpl<client::HTMLElement>
  {
  public:
    std::shared_ptr<JavascriptEvents> events;

    Element();
    Element(const client::String& str, const std::map<std::string, std::string>& attrs = {});
    Element(client::HTMLElement*);

    Element& attr(const std::map<std::string, std::string>& attrs);
    Element& attr(const std::map<std::string, std::wstring>& attrs);
    Element& attr(const std::map<std::string, client::String*>& attrs);
    Element& attr(const std::string& name, const std::string& value);
    std::string attr(const std::string& name) const;

    Element& css(const std::string& name, const std::string& value);
    Element& css(const std::map<std::string, std::string>& attrs);
    std::map<std::string, std::string> css() const;

    Element& html(client::String* content)      { (*this)->set_innerHTML(content); return *this; }
    Element& html(const std::string& content)   { _html<std::string>(content); return *this; }
    Element& html(const std::wstring& content)  { _html<std::wstring>(content); return *this; }
    Element& text(client::String* content)      { (*this)->set_textContent(content); return *this; }
    Element& text(const std::string& content)   { _text<std::string>(content); return *this; }
    Element& text(const std::wstring& content)  { _text<std::wstring>(content); return *this; }
    Element& value(client::String* val)         { static_cast<client::HTMLInputElement*>(**this)->set_value(val); return *this; }
    Element& value(const std::string& val)      { _value<std::string>(val); return *this; }
    Element& value(const std::wstring& val)     { _value<std::wstring>(val); return *this; }
    template<typename T>
    Element& value(const T val) { std::stringstream stream; stream << val; _value<std::string>(stream.str()); return *this; }
    Element& checked(bool val)                  { static_cast<client::HTMLInputElement*>(**this)->set_checked(val); return *this; }
    Element& selected(bool val)                 { static_cast<client::HTMLOptionElement*>(**this)->set_selected(val); return *this; }

    std::string html()    const { return Comet::Object((*this)->get_innerHTML()); }
    std::string tagName() const { return Comet::Object((*this)->get_tagName()); }

    template<typename T>
    T value() const
    {
      T var;
      std::stringstream stream;
      stream << get_value();
      stream >> var;
      return var;
    }

    template<>
    std::string value<std::string>() const
    {
      return get_value();
    }

    template<>
    std::wstring value<std::wstring>() const
    {
      auto* input_el = static_cast<client::HTMLInputElement*>(**this);
      auto* client_string = input_el->get_value();

      return to_wstring(client_string);
    }

    inline Element& inner(const std::vector<Element>& els)                  { return operator>(els); }
    inline Element& inner(const std::vector<Element*>& els)                 { return operator>(els); }
    inline Element& inner(const std::vector<std::shared_ptr<Element> > els) { return operator>(els); }

    template<typename TYPE>
    Element& operator>(const TYPE& list)
    {
      ElementListAppender<TYPE, is_array_container<TYPE>::value>::append_list(list, **this);
      return *this;
    }

    Element& empty();

    void append_to(client::HTMLElement* el);
    void append_to(Element& el);
    void insert_before(client::HTMLElement* el);
    void insert_before(Element& el);
    void insert_after(client::HTMLElement* el);
    void insert_after(Element& el);
    void destroy();

    bool    is_attached() const;
    bool    has_parent() const;
    Element get_parent() const;
    Element get_next() const;

    std::list<Element>   find(const std::string& selector);
    Element              find_one(const std::string& selector);
    bool                 contains(const client::HTMLElement*);
    void                 each(std::function<bool (Element&)>);

    void                 toggle_class(const std::string& str, bool set);
    void                 add_class(const std::string& str);
    void                 remove_class(const std::string& str);
    bool                 has_class(const std::string& str) const;

    bool                 has_attribute(const std::string& key) const;
    std::string          get_attribute(const std::string& key) const;
    void                 remove_attribute(const std::string& key);

    bool                 is_visible() const;
    Element&             visible(bool, const std::string& display = "");
    void                 set_visible(bool val) { visible(val); }

    std::string          get_inner_html() const;
    std::string          get_text() const;
    std::string          get_value() const;
    bool                 get_checked() const { return static_cast<client::HTMLInputElement*>(**this)->get_checked(); }
    bool                 get_selected() const { return static_cast<client::HTMLOptionElement*>(**this)->get_selected(); }

  private:
    template<typename TYPE>
    void _html(const TYPE& content)
    {
      (*this)->set_innerHTML(content.c_str());
    }

    template<typename TYPE>
    void _text(const TYPE& content)
    {
      (*this)->set_textContent(content.c_str());
    }

    template<typename TYPE>
    void _value(const TYPE& val)
    {
      static_cast<client::HTMLInputElement*>(**this)->set_value(val.c_str());
    }
  };

  typedef std::vector<Element> Elements;
}

#endif

