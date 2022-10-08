#ifndef  COMET_ARCHIVE_COLLECTION_HPP
# define COMET_ARCHIVE_COLLECTION_HPP

# include "collection.hpp"
# include <libtext-archive/archive.hpp>
# include <iostream>

namespace Comet
{
  template<typename MODEL>
  class ArchiveCollection : public Collection<MODEL>
  {
  public:
    virtual std::string get_mimetype() const
    {
      return Archive::mimetype;
    }

  protected:
    virtual void parse(const std::string& str)
    {
      IArchive      archive;
      unsigned long size;

      archive.set_data(str);
      try
      {
        archive & size;
        for (unsigned long i = 0 ; i < size ; ++i)
        {
          auto ptr = std::make_shared<MODEL>();
          ptr->serialize(archive);
          Collection<MODEL>::models.emplace(ptr->get_id(), ptr);
        }
      }
      catch (const ArchiveException& error)
      {
        std::cerr << "Failed to parse `Collection`: "
                  << error.what() << std::endl << error.dump << std::endl;
        throw;
      }
    }
  };
}

#endif
