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
      typedef typename list_type::const_iterator const_iterator;
      typedef typename list_type::const_reverse_iterator const_reverse_iterator;

      Subset() {}

      Subset(Collection<MODEL>& source_)
      {
        set_source(&source_);
      }

      Subset(const Subset& subset) :
        selector(subset.selector), sort_(subset.sort_), index_(subset.index_),
        limit_(subset.limit_), content(subset.content), dirty(subset.dirty)
      {
        set_source(subset.source);
      }

      Subset& operator=(const Subset& subset)
      {
        selector = subset.selector; sort_ = subset.sort_; index_ = subset.index_;
        limit_ = subset.limit_; content = subset.content; dirty = subset.dirty;
        set_source(subset.source);
        return *this;
      }

      const_iterator begin() const { return begin_iterator<const_iterator>(&list_type::begin); }
      const_iterator end() const { return end_iterator<const_iterator>(&list_type::begin, &list_type::end); }
      const_reverse_iterator rbegin() const { return begin_iterator<const_reverse_iterator>(&list_type::rbegin()); }
      const_reverse_iterator rend() const { return end_iterator<const_reverse_iterator>(&list_type::rbegin, &list_type::rend); }
      std::size_t size() const { if (dirty) { run_query(); } return content.size(); }
      Collection<MODEL>& collection() { return *source; }

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
      void set_source(Collection<MODEL>* source_)
      {
        std::function<void(const MODEL&)> set_dirty([this](const MODEL&){dirty = true;});
        if (source)
        {
          stop_listening(source->synced);
          stop_listening(source->added);
          stop_listening(source->removed);
        }
        source = source_;
        listen_to(source->synced, [this]() { dirty = true; });
        listen_to(source->added, set_dirty);
        listen_to(source->removed, set_dirty);
      }

      template<typename ITERATOR>
      ITERATOR begin_iterator(ITERATOR (list_type::*begin)() const) const
      {
        if (dirty) run_query();
        return (content.*begin)();
      }

      template<typename ITERATOR>
      ITERATOR end_iterator(ITERATOR (list_type::*begin)() const, ITERATOR (list_type::*end)() const) const
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

      void run_query() const
      {
        if (source)
        {
          auto selection = source->where(selector);
          content.clear();
          content.reserve(selection.size());
          std::copy(std::make_move_iterator(selection.begin()), std::make_move_iterator(selection.end()), std::back_inserter(content));
          if (sort_)
            std::sort(content.begin(), content.end(), [this](const ModelPtr& a, const ModelPtr& b) { return sort_(*a, *b); });
          dirty = false;
        }
      }

      Collection<MODEL>* source = nullptr;
      std::function<bool (const MODEL&)> selector = [](const MODEL&) { return true; };
      std::function<bool (const MODEL&, const MODEL&)> sort_;
      unsigned long index_ = 0, limit_ = 0;
      mutable list_type content;
      mutable bool dirty = true;
    };

    Subset subset()
    {
      return Subset(*this);
    }

    void add(ModelPtr model)
    {
      auto it = models.find(model->get_id());

      if (it == models.end())
      {
        models.emplace(model->get_id(), model);
        added.trigger(*model);
      }
    }

    void remove(ModelPtr model)
    {
      auto it = models.find(model->get_id());

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

    void each(std::function<void (MODEL&)> callback)
    {
      for (auto item : models)
        callback(*item.second);
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

    std::list<ModelPtr> where(std::function<bool (MODEL&)> selector) const
    {
      std::list<ModelPtr> results;

      for (auto item : models)
      {
        if (selector(*item.second))
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

      return fetch(request);
    }

    Promise fetch(Comet::Http::Request::Ptr request)
    {
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

    std::size_t count() const
    {
      return models.size();
    }

  protected:
    virtual void parse(const std::string& str) = 0;

    Map models;
  };
}

#endif
