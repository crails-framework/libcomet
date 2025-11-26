#ifndef  COMET_LEXICAL_CAST_HPP
# define COMET_LEXICAL_CAST_HPP

# include <string>
# include <string_view>
# include <sstream>
# include "from_string.hpp"

namespace Comet
{
  template<typename OUTPUT, typename INPUT, bool SAME = std::is_same<OUTPUT, INPUT>::value >
  struct lexical_caster
  {
    static inline OUTPUT _(INPUT input)
    {
      std::stringstream stream;
      OUTPUT output;

      stream << input;
      stream >> output;
      return output;
    }
  };

  template<typename OUTPUT, typename INPUT>
  struct lexical_caster<OUTPUT, INPUT, true>
  {
    static inline OUTPUT _(INPUT input) { return input; }
  };

  template<typename OUTPUT>
  struct lexical_caster<OUTPUT, std::string, false>
  {
    static inline OUTPUT _(const std::string& input) { return Comet::from_string<OUTPUT>(input); }
  };

  template<>
  struct lexical_caster<std::string, std::string_view, false>
  {
    static inline std::string _(std::string_view input) { return std::string(input.data(), input.length()); }
  };

  template<>
  struct lexical_caster<std::string, const char*, false>
  {
    static inline std::string _(const char* input) { return std::string(input); }
  };

  template<>
  struct lexical_caster<std::string, char*, false>
  {
    static inline std::string _(char* input) { return std::string(input); }
  };

  template<typename INPUT>
  struct lexical_caster<std::string, INPUT, false>
  {
    static inline std::string _(INPUT input) { return std::to_string(input); }
  };

  template<typename OUTPUT, typename INPUT>
  OUTPUT lexical_cast(INPUT input)
  {
    return lexical_caster<OUTPUT, INPUT, std::is_same<OUTPUT, INPUT>::value >::_(input);
  }
}

#endif
