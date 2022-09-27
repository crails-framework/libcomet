#ifndef  COMET_MVC_ARCHIVE_MODEL_HPP
# define COMET_MVC_ARCHIVE_MODEL_HPP

# include <crails/archive.hpp>
# include "model.hpp"
# include "helpers.hpp"

namespace Comet
{
  class ArchiveModel : public Model
  {
  public:
    virtual void serialize(OArchive&) = 0;
    virtual void serialize(IArchive&) = 0;

    void parse(const std::string& str) override
    {
      IArchive archive;

      archive.set_data(str);
      serialize(archive);
    }

  protected:
    std::string get_payload() override
    {
      OArchive archive;

      serialize(archive);
      return archive.as_string();
    }

    std::string get_content_type() const override { return Archive::mimetype; }
  };
}

#endif
