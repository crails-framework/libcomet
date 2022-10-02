#ifndef  COMET_JSON_MODEL_HPP
# define COMET_JSON_MODEL_HPP

# include "model.hpp"
# include <comet/datatree.hpp>
# include <sstream>

namespace Comet
{
  template<typename ID_TRAIT = LongIdTrait>
  class JsonModel : public Model<ID_TRAIT>
  {
    typedef Model<ID_TRAIT> Super;
  public:
    virtual void from_json(Data) = 0;
    virtual std::string to_json() const = 0;
    virtual std::string get_resource_name() const override { return Super::get_resource_name(); }

    void parse(const std::string& str) override
    {
      DataTree data;

      data.from_json(str);
      if (get_resource_name().length() > 0 && data[get_resource_name()].exists())
        from_json(data[get_resource_name()]);
      else
        from_json(data);
    }

  protected:
    std::string get_payload() override
    {
      if (get_resource_name().length() > 0)
      {
        std::stringstream stream;
        stream << '{' << '"' << get_resource_name() << '"' << ':' << to_json() << '}';

        return stream.str();
      }
      return to_json();
    }

    std::string get_content_type() const override
    {
      return "application/json";
    }
  };
}

#endif
