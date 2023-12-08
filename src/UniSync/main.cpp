#include "UniSync/SyncApp.h"

// #include "Engone/Engone.h"
// #include "Engone/Utilities/Utilities.h"
// #include "Engone/Utilities/Tracker.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#undef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#include <shellapi.h>

bool streq(const char* a, const char* b, bool compare_end = false, int len = -1) {
    int al = strlen(a);
    int bl = strlen(b);
    if(compare_end) {
        len = bl;
    }
    if(len == -1 && al != bl)
        return false;
    if(len == -1)
        len = al;
    
    if(!compare_end) {
        int i = 0;
        while(i < len) {
            if(a[i] != b[i]) {
                return false;
            }
            i++;
        }
    } else {
        int i = 0;
        while(i < len) {
            if(a[al - 1 - i] != b[bl - 1 - i]) {
                return false;
            }
            i++;
        }
    }
    return true;
}
void PrintHelp();
void runApp(int argc, const char** argv) {
	using namespace engone;

    // Default options
    unisync::AppOptions options{};
    options.cachePath = DEFAULT_FILE;
    
    bool print_args = false;
    // Options from command line
	for (int i = 0; i < argc;i++) {
        const char* arg = argv[i];
        // log::out << i<<": "<<arg<<"\n";
        
        if(i == 0) continue; // skip first arg, it's usually the exe
        
        int len = strlen(arg);
        if(len == 0)
            continue;
        
        // TODO: --console CreateConsole()
         if(streq(arg, "--print-args")) {
            print_args = true;
        } else if(streq(arg, "--file") || streq(arg, "-f")) {
            if(i + 1 < argc) {
                options.cachePath = argv[i + 1];
                i++;
            } else {
                log::out << log::RED << "Missing flag for '"<<arg<<"'\n";
            }
        } else if(streq(arg, "--help") || streq(arg, "-help") || streq(arg, "-?") || streq(arg, "-h")) {
            PrintHelp();
        // } else if(streq(arg, ".exe", true)) {
            // ignore
        } else {
            log::out << "Unknown flag/argument '"<<arg<<"' (see --help)\n";
        }
    }
       
    if(print_args) {
        log::out << "Arguments:\n";
        for (int i = 0; i < argc;i++) {
            const char* arg = argv[i];
            log::out << " "<<i<<": "<<arg<<"\n";
        }
    }
    
    unisync::StartApp(options);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
	using namespace engone;
	int argc;
	char** argv;
	ConvertArguments(argc,argv);
	runApp(argc,(const char**)argv);
	FreeArguments(argc, argv);
	return 0;
}
int main(int argc, const char** argv) {
	runApp(argc,(const char**)argv);
	return 0;
}

void PrintHelp() {
    using namespace engone;
    log::out << log::BLUE << "##   Help   ##\n";
    log::out << log::GRAY << "More information can be found here:\n"
        "https://github.com/Emarioo/UniSync/tree/master/docs\n";
    #define PRINT_USAGE(X) log::out << log::YELLOW << X ": "<<log::SILVER;
    #define PRINT_DESC(X) log::out << X;
    #define PRINT_EXAMPLES log::out << log::LIME << " Examples:\n";
    #define PRINT_EXAMPLE(X) log::out << X;
    
    PRINT_USAGE("exe --file <path>")
    PRINT_USAGE("exe -f <path>")
    PRINT_DESC("The path specifies which file UniSync will load connections and units from.\n")
    PRINT_EXAMPLES
    PRINT_EXAMPLE("  exe -f cache.dat\n")
    log::out << "\n";
    
    PRINT_USAGE("exe --print-args")
    PRINT_DESC("Prints the passed arguments.\n")
    log::out << "\n";
    
    // PRINT_USAGE("exe [file0 ...] -out [file0 ...]")
    // PRINT_DESC("With the "<<log::WHITE<<"out"<<log::WHITE<<" flag, files to the left will be compiled into "
    //          "bytecode and written to the files on the right of the out flag. "
    //          "The amount of files on the left and right must match.\n")
    // PRINT_EXAMPLES
    // PRINT_EXAMPLE("  compiler.exe main.btb -out program.btbc\n")
    // log::out << "\n";
    // PRINT_USAGE("compiler.exe -run [file0 ...]")
    // PRINT_DESC("Runs bytecode files generated with the out flag.\n")
    // PRINT_EXAMPLES
    // PRINT_EXAMPLE("  compiler.exe -run program.btbc\n")
    // log::out << "\n";
    // PRINT_USAGE("compiler.exe -target <target-platform>")
    // PRINT_DESC("Compiles source code to the specified target whether that is bytecode, Windows, Linux, x64, object file, or an executable. "
    //         "All of those in different combinations may not be supported yet.\n")
    // PRINT_EXAMPLES
    // PRINT_EXAMPLE("  compiler.exe main.btb -out main.exe -target win-x64\n")
    // PRINT_EXAMPLE("  compiler.exe main.btb -out main -target linux-x64\n")
    // PRINT_EXAMPLE("  compiler.exe main.btb -out main.btbc -target bytecode\n")
    // log::out << "\n";
    // PRINT_USAGE("compiler.exe -user-args [arg0 ...]")
    // PRINT_DESC("Only works if you run a program. Any arguments after this flag will be passed to the program that is specified to execute\n")
    // PRINT_EXAMPLES
    // PRINT_EXAMPLE("  compiler.exe main.btb -user-args hello there\n")
    // PRINT_EXAMPLE("  compiler.exe -run main.btbc -user-args -some-flag \"TO PASS\"\n")
    // log::out << "\n";
    #undef PRINT_USAGE
    #undef PRINT_DESC
    #undef PRINT_EXAMPLES
    #undef PRINT_EXAMPLE
}