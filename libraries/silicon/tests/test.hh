
#define CHECK_EQUAL(name, exp, ref) \
try {\
auto res = exp;\
if (res != (ref)) {\
  std::cout << "Test " << name <<  " failed " << (res) << " != " << ref << std::endl;\
}\
else\
  std::cout << "Test " << name <<  " \tOK" << std::endl;\
} catch (std::exception e) { std::cout << "Test " << name <<  " failed with exception" << e.what() << std::endl;} \
catch (iod::http_error e) { std::cout << "Test " << name <<  " \tOK: " << e.what() << std::endl;}

#define CHECK_THROW(name, exp) \
try {\
exp;\
std::cout << "Test " << name <<  " \t\tKO (did not throw)" << std::endl; \
} catch (std::runtime_error e) { std::cout << "Test " << name <<  " \tOK: " << e.what() << std::endl;} \
catch (iod::http_error e) { std::cout << "Test " << name <<  " \tOK: " << e.what() << std::endl;}


#define CHECK(name, exp) \
try {\
exp;\
std::cout << "Test " << name <<  " \t\tOK" << std::endl; \
} catch (std::runtime_error e) { std::cout << "Test " << name <<  " \tKO: " << e.what() << std::endl;} \
catch (iod::http_error e) { std::cout << "Test " << name <<  " \tKO: " << e.what() << std::endl;}

