#ifndef  COMET_MVC_HELPERS_HPP
# define COMET_MVC_HELPERS_HPP

# include "../signal.hpp"

# define add_model_property(type, name) \
  Crails::Signal<type> name##_changed; \
  type get_##name() const { return name; } \
  void set_##name(type val) \
  { \
    bool is_different = name != val; \
    name = val; \
    if (is_different) \
    { \
      changed.trigger(); \
      name##_changed.trigger(val); \
    } \
  }

# define add_model_ref_property(type, name) \
  Crails::Signal<const type&> name##_changed; \
  const type& get_##name() const { return name; } \
  void set_##name(const type& val) \
  { \
    bool is_different = name != val; \
    name = val; \
    if (is_different) \
    { \
      changed.trigger(); \
      name##_changed.trigger(name); \
    } \
  }

#endif
