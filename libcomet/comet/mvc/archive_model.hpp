#ifndef  COMET_MVC_ARCHIVE_MODEL_HPP
# define COMET_MVC_ARCHIVE_MODEL_HPP

# include <iostream>
# include <crails/archive.hpp>
# include "model.hpp"
# include "helpers.hpp"

namespace Comet
{
  template<typename ID_TRAIT = LongIdTrait>
  class ArchiveModel : public Model<ID_TRAIT>
  {
  public:
    virtual void serialize(OArchive&) = 0;
    virtual void serialize(IArchive&) = 0;

    void parse(const std::string& str) override
    {
      IArchive archive;

      archive.set_data(str);
      try
      {
        archive & *this;
      }
      catch (const ArchiveException& error)
      {
        std::cerr << "Failed to parse `" << Model<ID_TRAIT>::get_url() << "`: "
          << error.what() << std::endl << error.dump << std::endl;
        throw;
      }
    }

  protected:
    std::string get_payload() override
    {
      OArchive archive;

      archive & *this;
      return archive.as_string();
    }

    std::string get_content_type() const override { return Archive::mimetype; }
  };
}

#endif
