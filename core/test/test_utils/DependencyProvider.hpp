#ifndef TEST_DEPENDENCYPROVIDER_HPP_DEFINED
#define TEST_DEPENDENCYPROVIDER_HPP_DEFINED

#include "heightmap/FlatElevationProvider.hpp"
#include "index/StringTable.hpp"
#include "mapcss/StyleProvider.hpp"

#include "test_utils/MapCssUtils.hpp"
#include "MapCssUtils.hpp"

#include <cstdio>
#include <memory>

using namespace utymap::heightmap;
using namespace utymap::index;
using namespace utymap::mapcss;

class DependencyProvider 
{
 public:
  DependencyProvider() { }

  ~DependencyProvider() 
  {
    stringTable_.reset();
    std::remove("string.idx");
    std::remove("string.dat");
  }

  std::shared_ptr<StringTable> getStringTable() 
  {
    if (stringTable_== nullptr) {
      stringTable_ = std::shared_ptr<StringTable>(new StringTable(""));
    }
    return stringTable_;
  }

  std::shared_ptr<FlatElevationProvider> getElevationProvider() 
  {
    if (eleProvider_== nullptr) {
      eleProvider_ = std::shared_ptr<FlatElevationProvider>(new FlatElevationProvider());
    }
    return eleProvider_;
  }

  std::shared_ptr<StyleProvider> getStyleProvider(const std::string& declaration) 
  {
      if (styleProvider_ == nullptr) {
          styleProvider_ = MapCssUtils::createStyleProviderFromString(*getStringTable(), declaration);
      }
      return styleProvider_;
  }

 private:
  std::shared_ptr<FlatElevationProvider> eleProvider_;
  std::shared_ptr<StringTable> stringTable_;
  std::shared_ptr<StyleProvider> styleProvider_;
};

#endif  // TEST_DEPENDENCYPROVIDER_HPP_DEFINED
