#include "filesystem.hpp"

#ifndef __EMSCRIPTEN__
#include <fstream>
#include "tinyfiledialogs.h"

bool saveToFile(std::string name, std::vector<uint8_t> data) {
    const char* filename = tinyfd_saveFileDialog("Save", name.c_str(), 0, nullptr, nullptr);
    if (!filename) return false;

    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

bool loadFromFile(std::string type, std::vector<uint8_t>* outData) {
    const char* filterPatterns[] = { ("*" + type).c_str() };
    const char* filename = tinyfd_openFileDialog("Open", nullptr, 1, filterPatterns, nullptr, 0);
    if (!filename) return false;

    std::ifstream file(filename);
    if (!file) return false;
    outData->assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return true;
}
#else
#include <emscripten.h>

bool saveToFile(std::string name, std::vector<uint8_t> data) {
    EM_ASM({
        const ptr = $0;
        const len = $1;
        const name = UTF8ToString($2);
        const array = new Uint8Array(HEAPU8.slice(ptr, ptr + len));
        const blob = new Blob([array], { type: 'application/octet-stream' });
        const a = document.createElement('a');
        a.href = URL.createObjectURL(blob);
        a.download = name;
        a.click();
    }, data.data(), data.size(), name.c_str());
    return true;
}

bool fileLoaded = false;
bool fileError = false;
std::vector<uint8_t> fileData;
EM_JS(void, loadFileFromJS, (const char* filetype), {
    var input = document.createElement("input");
    input.type = "file";
    input.accept = UTF8ToString(filetype);

    input.addEventListener("click", function () {
        window.onfocus = function () {
            setTimeout(function () {
                if (input.files.length === 0) {
                    Module.ccall("onFileError", null, [], []);
                }
            
                window.onfocus = null;
            }, 500);
        };
    });
    input.addEventListener("change", function () {
        var file = input.files[0];
        if (file) {
            var reader = new FileReader();
            reader.onload = function (event) {
                const arrayBuffer = event.target.result;
                const uint8Array = new Uint8Array(arrayBuffer);

                // Allocate C++ buffer
                const ptr = Module._allocFileBuffer(uint8Array.length);
                const heapBytes = new Uint8Array(Module.HEAPU8.buffer, ptr, uint8Array.length);
                heapBytes.set(uint8Array);

                // Notify C++ that the file is ready
                Module.ccall("onFileLoaded", null, ["number", "number"], [ptr, uint8Array.length]);
            };
            reader.readAsArrayBuffer(file);
        } else {
            Module.ccall("onFileError", null, [], []);
        }
    });

    input.click();
});
extern "C" {
    uint8_t* allocFileBuffer(int size) {
        return (uint8_t*)malloc(size);
    }

    void onFileLoaded(uint8_t* data, int length) {
        fileData = std::vector<uint8_t>(data, data + length);
        fileLoaded = true;
        free(data);
    }

    void onFileError() {
        fileError = true;
    }
}
bool loadFromFile(std::string type, std::vector<uint8_t>* outData) {
    fileLoaded = false;
    loadFileFromJS(type.c_str());
    while (!fileLoaded && !fileError) {
        emscripten_sleep(100);
    }
    if (fileError) return false;
    outData->assign(fileData.begin(), fileData.end());
    return true;
}
#endif

bool saveToFile(std::string filename, std::string data) {
    return saveToFile(filename, std::vector<uint8_t>(data.begin(), data.end()));
}

bool loadFromFile(std::string filetype, std::string* outData) {
    std::vector<uint8_t> data;
    bool success = loadFromFile(filetype, &data);
    if (success) {
        *outData = std::string(data.begin(), data.end());
    }
    return success;
}