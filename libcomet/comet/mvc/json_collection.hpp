#ifndef  COMET_JSON_COLLECTION_HPP
# define COMET_JSON_COLLECTION_HPP

# include "collection.hpp"
# include "../datatree.hpp"

namespace Comet
{
  template<typename MODEL>
  class JsonCollection : public Collection<MODEL>
  {
  public:
    virtual std::string get_mimetype() const
    {
      return "application/json";
    }

    virtual std::string get_resource_name() const
    {
      return "";
    }

  protected:
    virtual void parse(const std::string& str)
    {
      DataTree   payload;
      Data       models_json;
      const auto resource_name = get_resource_name();

      payload.from_json(str);
      if (resource_name.length() == 0)
        models_json = payload.as_data();
      else
        models_json = payload[resource_name];
      from_data(models_json);
    }

    virtual void from_data(Data models_json)
    {
      models_json.each([this](Data model_json) -> bool
      {
        auto ptr = std::make_shared<MODEL>();
        ptr->from_json(model_json);
        Collection<MODEL>::models.emplace(ptr->get_id(), ptr);
        return true;
      });
    }
  };
}

#endif
