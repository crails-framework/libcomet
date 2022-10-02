#ifndef  COMET_MVC_COLLECTION_HPP
# define COMET_MVC_COLLECTION_HPP

# include "../signal.hpp"
# include "../http.hpp"
# include "model.hpp"
# include <map>
# include <list>
# include <algorithm>

namespace Comet
{
  template<typename MODEL>
  class Collection : public Listener
  {
    typedef std::shared_ptr<MODEL> ModelPtr;
    typedef std::map<typename MODEL::id_type, ModelPtr> Map;
  public:
    typedef MODEL Model;

    Signal<MODEL&> added;
    Signal<MODEL&> removed;
    Signal<MODEL&> changed;
    Signal<void>   synced;

    virtual std::string get_url() const = 0;
    virtual std::string get_mimetype() const = 0;

    struct Subset : public Listener
    {
      typedef std::vector<std::shared_ptr<MODEL>> list_type;
      typedef typename list_type::value_type value_type;

      Subset() {}

      Subset(Collection<MODEL>& source_) : source(&source_)
      {
        source->synced.connect([this]() { dirty = true; });
      }

      Subset(const Subset& subset) :
        source(subset.source), selector(subset.selector), sort_(subset.sort_),
        index_(subset.index_), limit_(subset.limit_), content(subset.content),
        dirty(subset.dirty)
      {
        source->synced.connect([this]() { dirty = true; });
      }

      Subset& operator=(const Subset& subset)
      {
        if (source)
          stop_listening(source->synced);
        source = subset.source; selector = subset.selector; sort_ = subset.sort_;
        index_ = subset.index_;, limit_ = subset.limit_; content = subset.content;
        dirty = subset.dirty;
        source->synced.connect([this]() { dirty = true; });
        return *this;
      }

      typename list_type::iterator begin() { return begin_iterator<list_type::iterator>(&list_type::begin); }
      typename list_type::iterator end() { return end_iterator<list_type::iterator>(&list_type::begin, &list_type::end); }
      typename list_type::reverse_iterator rbegin() { return begin_iterator<list_type::reverse_iterator>(&list_type::rbegin()); }
      typename list_type::reverse_iterator rend() { return end_iterator<list_type::reverse_iterator>(&list_type::rbegin, &list_type::rend); }
      unsigned long size() { if (dirty) { run_query(); } return content.size(); }
      const Collection<MODEL>& collection() const { return collection; }

      Subset& where(std::function<bool (const MODEL&)> callback)
      {
        dirty = true;
        selector = callback;
        return *this;
      }

      Subset& sort(std::function<bool (const MODEL&, const MODEL&)> callback)
      {
        dirty = true;
        sort_ = callback;
        return *this;
      }

      Subset& index(unsigned long value)
      {
        index_ = value;
        return *this;
      }

      Subset& limit(unsigned long value)
      {
        index_ = value;
        return *this;
      }

    private:
      template<typename ITERATOR, typename METHOD>
      ITERATOR begin_iterator(METHOD begin)
      {
        if (dirty) run_query();
        return (content.*begin)();
      }

      template<typename ITERATOR, typename METHOD>
      ITERATOR end_iterator(METHOD begin, METHOD end)
      {
        if (dirty) run_query();
        if (limit_)
        {
          auto it = (content.*begin)();
          std::advance(it, limit_);
          return it;
        }
        return (content.*end)();
      }

      void run_query()
      {
        if (source)
        {
          auto selection = source->where(selector);
          content.reserve(selection.size());
          std::copy(std::make_move_iterator(selection.begin()), std::make_move_iterator(selection.end()));
          if (sort_)
            std::sort(selection.begin(), selection.end(), sort_);
          dirty = false;
        }
      }

      Collection<MODEL>* source = nullptr;
      std::function<bool (const MODEL&)> selector = []() { return true; };
      std::function<bool (const MODEL&, const MODEL&)> sort_;
      unsigned long index_ = 0, limit_ = 0;
      list_type content;
      bool dirty = true;
    };

    Subset subset()
    {
      return Subset(*this);
    }

    void add(ModelPtr model)
    {
      auto it = models.at(model->get_id());

      if (it == models.end())
      {
        models.emplace(model->get_id(), model);
        added.trigger(*model);
      }
    }

    void remove(ModelPtr model)
    {
      auto it = models.at(model->get_id());

      if (it != models.end())
      {
        models.erase(it);
        removed.trigger(*model);
      }
    }

    void each(std::function<void (ModelPtr)> callback)
    {
      for (auto item : models)
        callback(item.second);
    }

    std::list<ModelPtr> where(std::function<bool (ModelPtr)> selector) const
    {
      std::list<ModelPtr> results;

      for (auto item : models)
      {
        if (selector(item.second))
          results.push_back(item.second);
      }
      return results;
    }

    ModelPtr find(std::function<bool (ModelPtr)> selector) const
    {
      for (auto item : models)
      {
        if (selector(item.second))
          return item.second;
      }
      return nullptr;
    }

    ModelPtr get(typename MODEL::id_type id)
    {
      auto it = models.find(id);

      if (it != models.end())
        return it->second;
      return nullptr;
    }

    Promise fetch()
    {
      auto request = Http::Request::get(get_url());
        
      request->set_headers({{"Accept", get_mimetype()}});
      return request->send().then([this, request]()
      {
        auto response = request->get_response();
          
        if (response->ok())
        {
          if (response->has_body())
            parse(response->get_response_text());
          synced.trigger();
        }
      });
    }

    unsigned int count() const
    {
      return models.size();
    }

  protected:
    virtual void parse(const std::string& str) = 0;

    Map models;
  };
}

#endif
