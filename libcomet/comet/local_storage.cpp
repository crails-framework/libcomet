#include "local_storage.hpp"
#include <cheerp/client.h>

using namespace std;
using namespace Comet;

LocalStorage::LocalStorage(client::Storage* storage) :
  storage(storage)
{
  initialize();
}

LocalStorage::LocalStorage()
{
  storage = client::window.get_localStorage();
  initialize();
}

LocalStorage::~LocalStorage()
{
  client::window.removeEventListener("storage", event_listener);
}

void LocalStorage::initialize()
{
  event_listener = cheerp::Callback([this](client::Event* _event)
  {
#ifndef USE_OLD_CLIENTLIB
    auto* event = _event->cast<client::StorageEvent*>();
#else
    auto* event = static_cast<client::StorageEvent*>(_event);
#endif

    if (event->get_storageArea() == storage)
      on_change.trigger(Change(event));
  });
  client::window.addEventListener("storage", event_listener);
}

LocalStorage::Change::Change(client::StorageEvent* event)
{
  key = (string)(*event->get_key());
#ifndef USE_OLD_CLIENTLIB
  if (event->get_oldValue())
    old_value = (string)(*(event->get_oldValue()->cast<client::String*>()));
  if (event->get_newValue())
    new_value = (string)(*(event->get_newValue()->cast<client::String*>()));
#else
  if (event->get_oldValue())
    old_value = (string)(*static_cast<client::String*>(event->get_oldValue()));
  if (event->get_newValue())
    new_value = (string)(*static_cast<client::String*>(event->get_newValue()));
#endif
}

template<>
void LocalStorage::set_item<const string&>(const string& key, const string& value)
{
  storage->setItem(key.c_str(), value.c_str());
}

template<>
void LocalStorage::set_item<string>(const string& key, string value)
{
  set_item<const string&>(key, value);
}

template<>
string LocalStorage::get_item<string>(const string& key) const
{
  client::Object* object = storage->getItem(key.c_str());

#ifndef USE_OLD_CLIENTLIB
  if (object)
    return (string)(*(object->cast<client::String*>()));
#else
  if (object)
    return (string)(*static_cast<client::String*>(object));
#endif
  return "";
}

void LocalStorage::remove_item(const string& key)
{
  storage->removeItem(key.c_str());
}

void LocalStorage::clear()
{
  storage->clear();
}
