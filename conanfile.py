import platform
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
    generators = "cmake_find_package", "cmake_paths"

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.h")
        self.copy("*.hpp")
        self.copy("*.dll", keep_path=True)
        self.copy("*.so", keep_path=True)
        self.copy("*.dylib", keep_path=True)
        self.copy("*.a", keep_path=True)
        self.copy("*.lib", keep_path=True)

    def package_info(self):
        if platform.system() == "Windows":
            if self.settings.build_type == "Debug":
                self.cpp_info.libdirs = ['lib/Debug']
            else:
                self.cpp_info.libdirs = ['lib/Release']
        self.cpp_info.libs = ["dbscan"]
