#include "symbols.hh"
#include <cassert>
#include <lithium.hh>
#include <string_view>

using namespace li;

int main() {

  const auto json = R"({
  "panelId": 1,
  "range": {
    "from": "2016-10-31T06:33:44.866Z",
    "to": "2016-10-31T12:33:44.866Z",
    "raw": {
      "from": "now-6h",
      "to": "now"
    }
  },
  "rangeRaw": {
    "from": "now-6h",
    "to": "now"
  },
  "interval": "30s",
  "intervalMs": 30000,
  "targets": [
     { "target": "upper_50", "refId": "A", "type": "timeserie" },
     { "target": "upper_75", "refId": "B", "type": "timeserie" }
  ],
  "adhocFilters": {
    "key": "City",
    "operator": "=",
    "value": "Berlin"
  },
  "format": "json",
  "maxDataPoints": 550
})";

  auto query = li::mmm(
      s::panelId = int(),     

      s::range = li::mmm(s::from = std::string(), s::to = std::string(),
                         s::raw = li::mmm(s::from = std::string(), s::to = std::string())),
      s::rangeRaw = li::mmm(s::from = std::string(), s::to = std::string()),

      s::interval = std::string(),       
      s::intervalMs = int(), 

      s::targets = std::vector<decltype(
          li::mmm(s::target = std::string(), s::refId = std::string(), s::type = std::string()))>(),

      s::adhocFilters =
          std::unordered_map<std::string, std::string>(),

      s::format = std::string(), 
      
      s::maxDataPoints = int()

  );

  auto obj = li::json_object(
      s::panelId,
      s::range = li::json_object(s::from, s::to, s::raw = li::json_object(s::from, s::to)),
      s::rangeRaw = li::json_object(s::from, s::to), 
      s::interval, s::intervalMs,
      s::targets = li::json_vector(s::target, s::refId, s::type),
      s::adhocFilters = li::json_map<std::string>(), 
      s::format, s::maxDataPoints);

  auto err = obj.decode(json, query);
  std::cout << err.what << std::endl;
  std::cout << obj.encode(query) << std::endl;

  assert(query.panelId == 1);
  assert(query.range.from == "2016-10-31T06:33:44.866Z");
  assert(query.range.to == "2016-10-31T12:33:44.866Z");
  assert(query.targets.size() == 2);
  assert(query.interval == "30s");
  assert(query.format == "json");
  assert(query.maxDataPoints == 550);

}