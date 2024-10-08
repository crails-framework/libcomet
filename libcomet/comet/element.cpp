#include "element.hpp"
#include "object.hpp"
#include <algorithm>

using namespace std;
using namespace Comet;

static std::map<std::string, std::string> display_style_by_tag = {
  {"div", "block"}, {"tr", "table-row"}, {"td", "table-cell"}, {"th", "table-cell"}, {"table", "table"}
};

static std::string get_default_display_style_for_tag(std::string tag)
{
  transform(tag.begin(), tag.end(), tag.begin(), [](char c) -> char { return tolower(c); });
  if (display_style_by_tag.find(tag) != display_style_by_tag.end())
    return display_style_by_tag.at(tag);
  return "inline-block";
}

static std::vector<std::string> string_split(const std::string& src, char delimiter)
{
  vector<string> strings;
  istringstream  f(src.c_str());
  string         s;

  while (getline(f, s, delimiter))
    strings.push_back(s);
  return strings;
}

Element::Element() : ObjectImpl(client::document.createElement("div")), events(make_shared<JavascriptEvents>(static_cast<client::HTMLElement*>(ptr)))
{
}

Element::Element(const client::String& type, const map<string, string>& children) :
  ObjectImpl(client::document.createElement(type)), events(make_shared<JavascriptEvents>(static_cast<client::HTMLElement*>(ptr)))
{
  attr(children);
}

Element::Element(client::HTMLElement* el) : ObjectImpl(el), events(make_shared<JavascriptEvents>(static_cast<client::HTMLElement*>(ptr)))
{
}

void Element::destroy()
{
  if (has_parent())
    (*this)->get_parentElement()->removeChild(**this);
}

bool Element::is_visible() const
{
  auto* style       = (*this)->get_style();
  auto* css_display = style->get_display();
  auto* css_visible = style->get_visibility();

  return (!css_display || (string)(*css_display) != "none") &&
         (!css_visible || (string)(*css_visible) != "hidden");
}

Element& Element::visible(bool value, const string& _display)
{
  auto*  style = (*this)->get_style();
  string display;

  if (value)
  {
    if (!_display.length())
      display = get_default_display_style_for_tag(tagName());
    else
      display = _display;
  }
  else
    display = "none";
  style->set_display(display.c_str());
  return *this;
}

bool Element::is_attached() const
{
  auto* el = (*this)->get_parentElement();

  while (el)
  {
    if (el == client::document.get_body())
      return true;
    el = el->get_parentElement();
  }
  return false;
}

bool Element::has_parent() const
{
  return (*this)->get_parentElement() != 0;
}

Element Element::get_parent() const
{
  return Element((*this)->get_parentElement());
}

Element Element::get_next() const
{
  return Element((client::HTMLElement*)((*this)->get_nextElementSibling()));
}

Element& Element::attr(const std::map<std::string, std::string>& attrs)
{
  for (auto it = attrs.begin() ; it != attrs.end() ; ++it)
   (*this)->setAttribute(it->first.c_str(), it->second.c_str());
  return *this;
}

Element& Element::attr(const std::map<std::string, std::wstring>& attrs)
{
  for (auto it = attrs.begin() ; it != attrs.end() ; ++it)
   (*this)->setAttribute(it->first.c_str(), it->second.c_str());
  return *this;
}

Element& Element::attr(const std::map<std::string, client::String*>& attrs)
{
  for (auto it = attrs.begin() ; it != attrs.end() ; ++it)
   (*this)->setAttribute(it->first.c_str(), it->second);
  return *this;
}

Element& Element::attr(const string& name, const string& value)
{
  (*this)->setAttribute(name.c_str(), value.c_str());
  return *this;
}

string Element::attr(const string& name) const
{
  if ((*this)->hasAttribute(name.c_str()))
  {
    Comet::Object attribute = (*this)->getAttribute(name.c_str());

    return attribute;
  }
  return "";
}

static std::string generate_style_attribute(std::map<std::string, std::string> attrs)
{
  std::string result;

  for (auto entry : attrs)
    result += entry.first + ':' + entry.second + ';';
  return result;
}

Element& Element::css(const string& name, const string& value)
{
  auto css_attributes = css();

  css_attributes[name] = value;
  attr("style", generate_style_attribute(css_attributes));
  return *this;
}

Element& Element::css(const map<string,string>& attrs)
{
  auto css_attributes = css();

  for (auto entry : attrs)
    css_attributes[entry.first] = entry.second;
  attr("style", generate_style_attribute(css_attributes));
  return *this;
}

map<string, string> Element::css() const
{
  map<string, string> results;
  string style = attr("style");
  auto   css_attributes = string_split(style, ';');

  for (auto css_attribute : css_attributes)
  {
    auto delimiter = css_attribute.find(':');

    if (delimiter != string::npos)
      results.emplace(css_attribute.substr(0, delimiter), css_attribute.substr(delimiter + 1));
  }
  return results;
}

void Element::append_to(client::HTMLElement* el)
{
  el->appendChild(static_cast<client::HTMLElement*>(ptr));
}

void Element::append_to(Element& el)
{
  el->appendChild(static_cast<client::HTMLElement*>(ptr));
}

void Element::insert_before(client::HTMLElement* el)
{
  if (el)
  {
    auto* parent = el->get_parentElement();

    if (parent)
      parent->insertBefore(**this, el);
  }
}

void Element::insert_before(Element& el)
{
  insert_before(static_cast<client::HTMLElement*>(el.ptr));
}

void Element::insert_after(client::HTMLElement* el)
{
  if (el)
  {
    Comet::Element wrapper(el);
    auto* nextSibling = el->get_nextSibling();

    if (nextSibling)
      insert_before(static_cast<client::HTMLElement*>(nextSibling));
    else if (wrapper.has_parent())
    {
      auto parent = wrapper.get_parent();
      append_to(parent);
    }
  }
}

void Element::insert_after(Element& el)
{
  insert_after(static_cast<client::HTMLElement*>(el.ptr));
}

bool Element::contains(const client::HTMLElement* source)
{
  bool result = false;

  if (**this == source)
    return true;
  each([&result, source](Element& el) -> bool
  {
    result = el.contains(source);
    return !result;
  });
  return result;
}

std::list<Element> Element::find(const std::string& selector)
{
  client::NodeList* node_list = (*this)->querySelectorAll(selector.c_str());
  std::list<Element> results;

  for (double i = 0 ; i < node_list->get_length() ; ++i)
  {
    client::Node* node = node_list->item(i);

    if (node->get_nodeType() == 1)
      results.push_back(Element(static_cast<client::HTMLElement*>(node)));
  }
  return results;
}

Element Element::find_one(const std::string& selector)
{
  return Element(static_cast<client::HTMLElement*>((*this)->querySelector(selector.c_str())));
}

void Element::each(std::function<bool (Element&)> func)
{
  auto* source = (*this)->get_childNodes();
  vector<client::Node*> list;

  list.reserve(source->get_length());
  for (double i = 0 ; i < source->get_length() ; ++i)
  {
    if (source->item(i)->get_nodeType() == 1)
      list.push_back(source->item(i));
  }
  for (client::Node* node : list)
  {
    Element child(static_cast<client::HTMLElement*>(node));

    if (func(child) == false)
      break ;
  }
}

Element& Element::empty()
{
  auto* node_list = (*this)->get_childNodes();

  while (node_list->get_length())
  {
    (*this)->removeChild(node_list->item(0));
    node_list = (*this)->get_childNodes();
  }
  return *this;
}

void Element::toggle_class(const std::string& str, bool set)
{
  if (has_class(str))
  {
    if (!set)
      remove_class(str);
  }
  else if (set)
    add_class(str);
}

void Element::add_class(const std::string& str)
{
  auto* class_list = (*this)->get_classList();

  class_list->add(str.c_str());
}

void Element::remove_class(const std::string& str)
{
  auto* class_list = (*this)->get_classList();

  class_list->remove(str.c_str());
}

bool Element::has_class(const std::string& str) const
{
  auto* class_list = (*this)->get_classList();

  return class_list->contains(str.c_str());
}

bool Element::has_attribute(const std::string& key) const
{
  return (*this)->hasAttribute(key.c_str());
}

std::string Element::get_attribute(const std::string& key) const
{
  client::String* attr_ptr = (*this)->getAttribute(key.c_str());

  return (std::string)(*attr_ptr);
}

void Element::remove_attribute(const std::string& key)
{
  (*this)->removeAttribute(key.c_str());
}

std::string Element::get_value() const
{
  auto* input_el      = static_cast<client::HTMLInputElement*>(**this);
  auto* client_string = input_el->get_value();

  return (std::string)(*client_string);
}

std::string Element::get_text() const
{
  auto* client_string = (*this)->get_innerText();

  return (std::string)(*client_string);
}

std::string Element::get_inner_html() const
{
  auto* client_string = (*this)->get_innerHTML();

  return (std::string)(*client_string);
}
