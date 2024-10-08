#include "datatree.hpp"
#include "globals.hpp"
#include "exception.hpp"
#include <iostream>

using namespace std;

DataTree& DataTree::from_map(const std::map<std::string, std::string>& vars)
{
  for (auto it = vars.begin() ; it != vars.end() ; ++it)
  {
    Data data = Data(tree, it->first);
    data = it->second;
  }
  return *this;
}

DataTree& DataTree::from_json(stringstream& stream)
{
  return from_json(stream.str());
}

DataTree& DataTree::from_json(const string& str)
{
  Comet::String js_str(str);

  tree = Comet::Object::from_json(*js_str);
  return *this;
}

DataTree& DataTree::from_json_file(const string&)
{
  Comet::raise(std::logic_error("DataTree::from_json_file not available from Comet.cpp"));
  return *this;
}

void Data::each(function<bool (Data)> functor)
{
  for (auto _key : get_keys())
  {
    Data data(as_object(), _key);

    if (!(functor(data)))
      break ;
  }
}

void Data::each(function<bool (const Data)> functor) const
{
  for (auto _key : get_keys())
  {
    const Data data(as_object(), _key);

    if (!(functor(data)))
      break ;
  }
}

Data Data::at(unsigned int i) const
{
  std::stringstream stream;
  std::string stringified;

  stream << i;
  stream >> stringified;
  return Data(as_object(), stringified);
}

bool Data::exists() const
{
  if (key.length())
  {
    Comet::String js_key(key);

    return object->hasOwnProperty(*js_key);
  }
  return true;
}

bool Data::is_blank() const
{
  return !exists() || as<string>() == "";
}

bool Data::is_null() const
{
  return !exists() || as<string>() == "null";
}

vector<string> Data::get_keys() const
{
  return Comet::window["Object"].apply("keys", as_object());
}

vector<string> Data::find_missing_keys(const vector<string>& keys) const
{
  vector<string> missing_keys;
  string         path_prefix;

  for (string key : keys)
  {
    Comet::String js_key(key);

    if (!(as_object()->hasOwnProperty(*js_key)))
      missing_keys.push_back(key);
  }
  return missing_keys;
}

bool Data::require(const vector<string>& keys) const
{
  return find_missing_keys(keys).size() == 0;
}

bool Data::is_array() const
{
  return *(as_object()["constructor"]) == *(Comet::window["Array"]);
}

void Data::merge(Data data)
{
  if (data.exists())
  {
    if (key.length() > 0)
    {
     if (data.is_array())
        object.set(key, data.as_object().apply("slice", 0));
      else
        object.set(key, Comet::Object::from_json(data.as_object().to_json<wstring>()));
    }
    else
    {
      for (const auto& key : data.get_keys())
        (*this)[key].merge(data[key]);
    }
  }
}

void Data::merge(DataTree datatree)
{
  merge(datatree.as_data());
}

void Data::output(ostream& out) const
{
  out << to_json();
}

string Data::to_xml() const
{
  Comet::raise(std::logic_error("Data::to_xml not available from Comet.cpp"));
  return "";
}

size_t Data::count() const
{
  if (is_array())
    return as_object()["length"];
  return get_keys().size();
}

void Data::destroy()
{
  if (key.length() > 0)
    object.unset(key);
  else
  {
    for (auto subkey : get_keys())
      object.unset(subkey);
  }
}

template<>
bool Data::as<bool>() const
{
  std::string as_string = as_object().apply("toString");
  std::stringstream stream;
  int numerical_value;

  if (as_string == "true" || as_string == "false")
    return as_string == "true";
  stream << as_string;
  stream >> numerical_value;
  return numerical_value != 0;
}

template<>
string Data::as<string>() const
{
  return as_object().apply("toString");
}

template<>
wstring Data::as<wstring>() const
{
  return as_object().apply("toString");
}
