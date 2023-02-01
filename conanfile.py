from conans import ConanFile, CMake, tools
from conan.tools.cmake import CMakeToolchain, CMake

class SimpleDBSCANConan(ConanFile):
    name = "SimpleDBSCAN"
    version = "1.1"
    license = "MIT"
    author = "CallmeNezha"
    description = "Implementation of dbscan algorithms using own implemented kdtree instance. Deepsense have slightly modified kd tree parameters to gain better performance and results"
    topics = ("Library needs improvements to provide better performance")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    exports_sources = ["CMakeLists.txt", "src/*", "include/*", "FindSimpleDBSCAN.cmake"]

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

        # Explicit way:
        # self.run('cmake %s/hello %s'
        #          % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        cmake = CMake(self)
        cmake.install()

        self.copy("*.h")
        self.copy("*.dll", keep_path=True)
        self.copy("*.so", keep_path=True)
        self.copy("*.dylib", keep_path=True)
        self.copy("*.a", keep_path=True)


    def package_info(self):
        self.cpp_info.libs = ["dbscan"]

