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
      IArchive         archive;
      std::list<MODEL> list;

      try
      {
        archive.set_data(str);
        archive & list;
        for (auto it = list.begin() ; it != list.end() ; ++it)
          Collection<MODEL>::models.emplace(it->get_id(), std::make_shared<MODEL>(*it));
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
