
#define CHECK_EQUAL(name, exp, ref) \
try {\
auto res = exp;\
if (res != (ref)) {\
  std::cout << "Test " << name <<  " failed " << (res) << " != " << ref << std::endl;\
  assert(0); \
}\
else\
  std::cout << "Test " << name <<  " \tOK" << std::endl;\
} catch (std::exception e) { std::cout << "Test " << name <<  " \tKO" << e.what() << std::endl; assert(0);} \
catch (iod::http_error e) { std::cout << "Test " << name <<  " \tKO: " << e.what() << std::endl; assert(0); }

#define CHECK_THROW(name, exp) \
try {\
exp;\
std::cout << "Test " << name <<  " \t\tKO (did not throw)" << std::endl; throw std::runtime_error("Test KO."); \
} catch (std::runtime_error e) { std::cout << "Test " << name <<  " \tOK: " << e.what() << std::endl; } \
catch (iod::http_error e) { std::cout << "Test " << name <<  " \tOK: " << e.what() << std::endl;  }


#define CHECK(name, exp) \
try {\
exp;\
std::cout << "Test " << name <<  " \t\tOK" << std::endl; \
} catch (std::runtime_error e) { std::cout << "Test " << name <<  " \tKO: " << e.what() << std::endl; assert(0); } \
catch (iod::http_error e) { std::cout << "Test " << name <<  " \tKO: " << e.what() << std::endl; assert(0); }

