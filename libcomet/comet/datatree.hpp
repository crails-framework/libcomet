#ifndef  FRONT_DATATREE_HPP
# define FRONT_DATATREE_HPP

# include <functional>
# include <vector>
# include <map>
# include <string>
# include <iostream>
# include <sstream>
# include "object.hpp"
# include <iostream>

class DataTree;

class Data
{
  friend class DataTree;
protected:
  Data(Comet::Object o) : object(o)
  {
  }

  Data(Comet::Object o, const std::string& k) : object(o), key(k)
  {
  }

public:
  Data() : object(Comet::Object())
  {
  }

  template<typename T>
  T operator[](const std::string& key) const
  {
    std::string as_string = as_object()[key.c_str()];
    std::stringstream stream;
    T output;

    stream << as_string;
    stream >> output;
    return output;
  }

  Data operator[](const std::string& k) const
  {
    if (is_null())
      object.set(key, Comet::Object());
    return Data(as_object(), k.c_str());
  }

  template<typename T>
  T operator[](const char* str) const { return operator[]<T>(std::string(str)); }
  Data operator[](const char* str) const { return operator[](std::string(str)); }

  Data at(unsigned int i) const;

  std::vector<std::string> find_missing_keys(const std::vector<std::string>& keys) const;
  bool require(const std::vector<std::string>& keys) const;

  const std::string& get_path() const { return key; }
  const std::string& get_key()  const { return key; }

  std::size_t count() const;

  template<typename T>
  T as() const
  {
    std::string as_string = as_object().apply("toString");
    std::stringstream stream;
    T value;

    stream << as_string;
    stream >> value;
    return value;
  }

  template<typename T>
  T defaults_to(const T def) const { return exists() ? as<T>() : def; }

  template<typename T>
  operator T() const
  {
    return as<T>();
  }

  template<typename T>
  std::vector<T> to_vector() const
  {
    return as_object().to_vector<T>();
  }

  template<typename T>
  void from_vector(const std::vector<T>& array)
  {
    Comet::ObjectImpl<client::Array> array_object;

    for (const T& v : array)
    {
      Comet::Object value_object(v);

      array_object->push(*value_object);
    }
    object.set(key, array_object);
  }

  template<typename T>
  operator std::vector<T>() const
  {
    return to_vector<T>();
  }

  template<typename T>
  Data& operator=(const T value)
  {
    Comet::Object value_object(value);

    object.set(key, value_object);
    return *this;
  }

  template<typename T>
  Data& operator=(const std::vector<T>& value)
  {
    from_vector(value);
    return *this;
  }

  Data& operator=(const Data& copy)
  {
    object = copy.object;
    key    = copy.key;
    return *this;
  }

  template<typename T>
  bool operator==(const T value) const
  {
    T self = as_object();

    return self == value;
  }

  template<typename T>
  bool operator!=(const T value) const { return !(Data::operator==(value)); }

  Data operator||(Data value) const
  {
    return exists() ? *this : value;
  }

  template<typename T>
  T operator||(const T value) const
  {
    return defaults_to<T>(value);
  }

  void push_back(Data data)
  {
    if (!(exists()))
      object.set(key, Comet::ObjectImpl<client::Array>());
    Comet::ObjectImpl<client::Array> array(*as_object());

    array->push(*data.as_object());
  }

  template<typename T>
  void push_back(const T value)
  {
    if (!(exists()))
      object.set(key, Comet::ObjectImpl<client::Array>());
    Comet::ObjectImpl<client::Array> array(*as_object());
    Comet::Object value_object(value);

    array->push(*value_object);
  }

  bool is_null() const;
  bool is_blank() const;
  bool is_array() const;
  bool exists() const;
  void destroy();

  void each(std::function<bool (Data)> functor);
  void each(std::function<bool (const Data)> functor) const;

  void output(std::ostream& out = std::cout) const;
  template<typename RETURN_TYPE = std::string>
  RETURN_TYPE to_json() const { return as_object().to_json<RETURN_TYPE>(); }
  std::string to_xml() const;

  void merge(Data data);
  void merge(DataTree datatree);

  Comet::Object as_object() const
  {
    if (key.length() == 0)
      return object;
    return object[key.c_str()];
  }

  std::vector<std::string> get_keys() const;

  mutable Comet::Object object;
  std::string                   key;
};

template<> bool Data::as<bool>() const;
template<> std::string Data::as<std::string>() const;
template<> std::wstring Data::as<std::wstring>() const;

class DataTree
{
public:
  DataTree() {}
  DataTree(Comet::Object o) : tree(o) {}

  operator   Data()                                   { return as_data();  }
  Data       as_data()                                { return Data(tree); }
  const Data as_data() const                          { return Data(tree); }
  Data       operator[](const std::string& key)       { return Data(tree, key.c_str()); }
  const Data operator[](const std::string& key) const { return Data(tree, key.c_str()); }
  void       clear()                                  { tree = Comet::Object(); }

  DataTree& from_map(const std::map<std::string, std::string>&);

  DataTree& from_json(std::stringstream& stream);
  DataTree& from_json(const std::string& str);
  DataTree& from_json_file(const std::string&);
  template<typename RETURN_TYPE = std::string>
  RETURN_TYPE to_json() const { return tree.to_json<RETURN_TYPE>(); }

  Comet::Object as_object() { return tree; }

private:
  Comet::Object tree;
};

#endif
