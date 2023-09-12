add_an_tool(BoujourDiscover discover.cpp)

target_link_libraries(BoujourDiscover PRIVATE Core Utilities)

add_an_tool(BoujourRegister register.cpp)

target_link_libraries(BoujourRegister PRIVATE Core Utilities)