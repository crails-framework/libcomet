#ifndef  COMET_MVC_ID_TYPE_HPP
# define COMET_MVC_ID_TYPE_HPP

namespace Comet
{
  struct LongIdTrait
  {
    typedef unsigned long type;
    constexpr static unsigned long null_id = 0;
  };

  struct StringIdTrait
  {
    typedef std::string type;
    constexpr static const char* null_id = "";
  };
}

#endif
