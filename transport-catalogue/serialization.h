#pragma once

#include "request_handler.h"

#include <filesystem>
#include <fstream>

class Serializator
{
public:
    Serializator(RequestHandler& handler, const std::string& path) : handler(handler), path(path) { }
    void Save();
    void Load();

private:
    std::filesystem::path path;
    RequestHandler& handler;
};