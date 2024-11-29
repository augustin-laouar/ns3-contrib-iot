# Use
## Examples
### Include Nlohmann JSON lib
- Create folder scratch/includes
- Download json.hpp from github repo : https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp
- Add this line at the top of scratch/CMakeLists.txt : 
  ```C
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)
  ```
- Include it in script file : 
  ```cpp
  #include "json.hpp"
  using json = nlohmann::json;
  ```
