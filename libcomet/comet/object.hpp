#ifndef  CRAILS_FRONT_OBJECT_HPP
# define CRAILS_FRONT_OBJECT_HPP

# include <cheerp/client.h>
# include <cheerp/clientlib.h>
# include <map>
# include <vector>
# include <string>
# include <sstream>
# include <functional>

#define js_object(...) \
  Comet::Object(std::map<std::string, Comet::Object>({__VA_ARGS__}))
#define js_array(...) \
  Comet::Object(std::vector<Comet::Object>({__VA_ARGS__}))

namespace Comet
{
  template<typename CLIENT_TYPE>
  class ObjectImpl;

  template<typename ARG>
  struct ApplyParamsToString;

  std::wstring to_wstring(client::String*);

  class Object
  {
  protected:
    client::Object* ptr;
  public:
    Object() { ptr = new client::Object; }
    Object(client::Object* ptr) : ptr(ptr) {}
    Object(client::String* ptr) : ptr((client::Object*)ptr) {}
    Object(const char* str)           { ptr = new client::String(str); }
    Object(const wchar_t* str)        { ptr = new client::String(str); }
    Object(const std::string& str)    { ptr = new client::String(str.c_str()); }
    Object(const std::wstring& str)   { ptr = new client::String(str.c_str()); }
    Object(unsigned long number)      { ptr = new client::String(number); }
    Object(unsigned int number)       { ptr = new client::String(number); }
    Object(long number)               { ptr = new client::String(number); }
    Object(int number)                { ptr = new client::String(number); }
    Object(double number)             { ptr = new client::String(number); }
    Object(long long number)          { ptr = new client::String(static_cast<long>(number)); }
    Object(unsigned long long number) { ptr = new client::String(static_cast<unsigned long>(number)); }
    Object(long double number)        { ptr = new client::String(static_cast<double>(number)); }
    Object(bool boolean)              { ptr = new client::String((int)boolean); }
    Object(const std::map<std::string, Object>& items) { ptr = new client::Object; set(items); }

    Object(const std::vector<Object>& items)
    {
      auto* array = new client::Array;

      for (auto item : items)
        array->push(*item);
      ptr = array;
    }

    client::Object* native_object() const { return ptr; }

    template<typename CLIENT_TYPE>
    ObjectImpl<CLIENT_TYPE> cast() const
    {
#ifndef USE_OLD_CLIENTLIB
      return ObjectImpl<CLIENT_TYPE>(ptr->cast<CLIENT_TYPE>());
#else
      return ObjectImpl<CLIENT_TYPE>(static_cast<CLIENT_TYPE*>(ptr));
#endif
    }

    template<typename FUNCTYPE>
    Object(std::function<FUNCTYPE> func) { ptr = cheerp::Callback(func); }

    static Object from_json(client::String* str)
    {
      return client::JSON.parse(str);
    }

    static Object from_json(const std::string& str)
    {
      return from_json(new client::String(str.c_str()));
    }

    static Object from_json(const std::wstring& str)
    {
      return from_json(new client::String(str.c_str()));
    }

    template<typename RETURN_TYPE = std::string>
    RETURN_TYPE to_json() const
    {
#ifndef USE_OLD_CLIENTLIB
      return (RETURN_TYPE)(*(client::JSON.stringify(ptr)->cast<client::String*>()));
#else
      return (RETURN_TYPE)(*(static_cast<client::String*>(client::JSON.stringify(ptr))));
#endif
    }

    template<>
    std::wstring to_json<std::wstring>() const
    {
#ifndef USE_OLD_CLIENTLIB
      return to_wstring(client::JSON.stringify(ptr)->cast<client::String*>());
#else
      return to_wstring(static_cast<client::String*>(client::JSON.stringify(ptr)));
#endif
    }

    Object operator[](const char* key)
    {
      return Object(ptr->operator[](key));
    }

    template<typename CLIENT_TYPE>
    ObjectImpl<CLIENT_TYPE> get(const char* key)
    {
      return ObjectImpl<CLIENT_TYPE>(*(this->operator[](key)));
    }

    client::Object* operator*() const { return ptr; }
    client::Object* operator->() const { return ptr; }

    operator std::string() const
    {
      if (!is_of_type("String"))
        __asm__("throw 'Comet::Object cast to std::string, but type is not String'");
#ifndef USE_OLD_CLIENTLIB
      return (std::string)(*ptr->cast<client::String*>());
#else
      return (std::string)(*static_cast<client::String*>(ptr));
#endif
    }

    operator std::wstring() const
    {
      if (!is_of_type("String"))
        __asm__("throw 'Comet::Object cast to std::wstring, but type is not String'");
#ifndef USE_OLD_CLIENTLIB
      return to_wstring(ptr->cast<client::String*>());
#else
      return to_wstring(static_cast<client::String*>(ptr));
#endif
    }

    operator double()      const { return (double)(*ptr); }

    template<typename T>
    operator std::vector<T>() const
    {
      return to_vector<T>();
    }

    template<typename T>
    std::vector<T> to_vector() const
    {
      std::vector<T> result;

      if (is_of_type("Array"))
      {
#ifndef USE_OLD_CLIENTLIB
        const client::Array& array = *(ptr->cast<client::Array*>());
#else
        const client::Array& array = *(static_cast<client::Array*>(ptr));
#endif
        result.resize(array.get_length());
        for (int i = 0 ; i < array.get_length() ; ++i)
          result[i] = (T)(Object(array[i]));
      }
      return result;
    }

    bool is_of_type(const std::string& type) const { return is_of_type(type.c_str()); }
    bool is_of_type(const char* type) const;

    void set(const std::string_view str, Object object)
    {
      set(std::string(str.data(), str.length()), object);
    }

    void set(const std::string& str, Object object)
    {
      ptr->set_(str.c_str(), object.native_object());
    }

    void set(const char* str, Object object)
    {
      ptr->set_(str, object.native_object());
    }

    void set(const std::map<std::string_view, Object>& items)
    {
      for (auto item : items)
        set(item.first, item.second);
    }

    void set(const std::map<std::string, Object>& items)
    {
      for (auto item : items)
        set(item.first, item.second);
    }

    void unset(const std::string_view str)
    {
      unset(std::string(str.data(), str.length()));
    }

    void unset(const std::string& str)
    {
      ptr->set_(str.c_str(), nullptr);
    }

    void unset(const char* str)
    {
      ptr->set_(str, nullptr);
    }

    template<typename ...ARGS>
    Object apply(const std::string& name, ARGS... args)
    {
      std::string str      = "_comet_." + name + '(';
      std::string str_args = _apply_params('a', args...);

      str = str + str_args;
      str = str + ')';
      set_global("_comet_", *this);
      return client::eval(str.c_str());
    }

    bool is_undefined() const;

    static void set_global(const std::string&, Comet::Object);

  private:
    std::string _apply_params(char) { return ""; }

    template<typename ARG>
    std::string _apply_params(char i, ARG arg)
    {
      return ApplyParamsToString<ARG>::func(arg, i);
    }

    template<typename ARG, typename ...ARGS>
    std::string _apply_params(char i, ARG arg, ARGS... args)
    {
      return ApplyParamsToString<ARG>::func(arg, i)
        + ',' + _apply_params<ARGS...>(i + 1, args...);
    }
  };
  template<>
  std::vector<std::wstring> Object::to_vector() const;

  template<typename ARG>
  struct ApplyParamsToString
  {
    static std::string func(ARG arg, char)
    {
      std::stringstream stream;

      stream << arg;
      return stream.str();
    }
  };

  template<>
  struct ApplyParamsToString<Object>
  {
    static std::string func(Object object, char i)
    {
      std::string varname = "_comet_arg_0_";

      varname[11] = i;
      Object::set_global(varname.c_str(), object);
      return varname;
    }
  };

  template<typename ARG>
  struct ApplyParamsToString<ObjectImpl<ARG> >
  {
    static std::string func(ObjectImpl<ARG> arg, char i)
    {
      return ApplyParamsToString<Object>::func(Object((client::Object*)(*arg)), i);
    }
  };

  template<>
  struct ApplyParamsToString<client::Object*>
  {
    static std::string func(client::Object* arg, char i)
    {
      return ApplyParamsToString<Object>::func(Object(arg), i);
    }
  };

  template<>
  struct ApplyParamsToString<client::String*>
  {
    static std::string func(client::String* arg, char i)
    {
      return ApplyParamsToString<Object>::func(Object((client::Object*)arg), i);
    }
  };

  template<>
  struct ApplyParamsToString<const char*>
  {
    static std::string func(const char* arg, char i)
    {
      std::string varval("\"");

      varval = varval + arg;
      varval = varval + "\"";
      return varval;
    }
  };

  template<>
  struct ApplyParamsToString<std::string>
  {
    static std::string func(std::string arg, char i)
    {
      return '"' + arg + '"';
    }
  };

  template<typename CLIENT_TYPE>
  class ObjectImpl : public Object
  {
  public:
    ObjectImpl() { ptr = new CLIENT_TYPE; }
    ObjectImpl(CLIENT_TYPE* ptr) { this->ptr = ptr; }
    ObjectImpl(client::Object* ptr) { this->ptr = ptr; }

    CLIENT_TYPE* native_object() const
    {
#ifndef USE_OLD_CLIENTLIB
      return ptr->cast<CLIENT_TYPE*>();
#else
      return reinterpret_cast<CLIENT_TYPE*>(ptr);
#endif
    }
    CLIENT_TYPE* operator*() const { return native_object(); }
    CLIENT_TYPE* operator->() const { return native_object(); }
  };

  class String : public ObjectImpl<client::String>
  {
  public:
    String() { ptr = new client::String; }
    String(client::String* ptr) { this->ptr = ptr; }
    String(client::Object* ptr) { this->ptr = ptr; }
    String(const char* str)         { ptr = new client::String(str); }
    String(const wchar_t* str)      { ptr = new client::String(str); }
    String(const std::string& str)  { ptr = new client::String(str.c_str()); }
    String(const std::wstring& str) { ptr = new client::String(str.c_str()); }
  };

  template<>
  struct ApplyParamsToString<String>
  {
    static std::string func(String arg, char i)
    {
      return ApplyParamsToString<Object>::func(Object((client::Object*)(*arg)), i);
    }
  };
}

#endif
