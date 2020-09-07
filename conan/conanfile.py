from conans import ConanFile, CMake, tools


class LithiumConan(ConanFile):
    name = "lithium"
    version = "0.01"
    license = "MIT"
    author = "Matthieu Garrigues matthieu.garrigues@gmail.com"
    url = "https://github.com/matt-42/lithium"
    description = "C++17 easy to use and high performance web framework"
    topics = ("webframework", "sql", "serialization", "high performance")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake"
    requires = [ "sqlite3/3.32.3"]
    exports_sources = ["cmake/FindMYSQL.cmake"]

    def source(self):
        self.run("git clone https://github.com/matt-42/lithium")
        # This small hack might be useful to guarantee proper /MT /MD linkage
        # in MSVC if the packaged project doesn't have variables to set it
        # properly
        tools.replace_in_file("lithium/CMakeLists.txt", "project(li)",
                              '''project(lithium)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()''')

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="lithium")
        cmake.build(["-t", "li_symbol_generator"])

        # Explicit way:
        # self.run('cmake %s/lithium %s'
                #   % (self.source_folder, cmake.command_line))
        # self.run("cmake --build %s li_symbol_generator" % cmake.build_config)

    def package(self):
        self.copy("*.hh", dst="include", src="lithium/single_headers")
        self.copy("li_symbol_generator", dst="bin", keep_path=False)

    def package_info(self):
        self.copy("cmake/FindMYSQL.cmake", ".", ".")
        #self.cpp_info.libs = ["hello"]

