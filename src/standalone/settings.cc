#include <iostream>
#include <fstream>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include "host.h"
#include "settings.h"

bool LoadSettings() {
    std::ifstream file("settings.json");

    if (!file.is_open())
        return false;
    
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    builder["collectComments"] = false;
    if (!Json::parseFromStream(builder, file, &root, &errs)) {
        file.close();

        std::cerr << "Failed to parse settings: " << errs;
        return false;
    }

    Json::Value cfg_overlays = root["overlays"];
    overlays.clear();

    for (int i = 0; i < cfg_overlays.size(); ++i) {
        Json::Value &item = cfg_overlays[i];
        overlay_info &overlay = overlays[item["name"].asString()];
        
        overlay.name = item["name"].asString();
        overlay.url = item["url"].asString();
        overlay.x = item["x"].asInt();
        overlay.y = item["y"].asInt();
        overlay.width = item["width"].asInt();
        overlay.height = item["height"].asInt();
        overlay.visible = item.get("visible", false).asBool();
        overlay.locked = item.get("locked", false).asBool();
    }

    file.close();
    return true;
}

bool SaveSettings() {
    Json::Value root(Json::objectValue), cfg_overlays(Json::arrayValue);

    for (auto &pair : overlays) {
        Json::Value item(Json::objectValue);
        item["name"] = pair.second.name;
        item["url"] = pair.second.url;
        item["x"] = pair.second.x;
        item["y"] = pair.second.y;
        item["width"] = pair.second.width;
        item["height"] = pair.second.height;
        item["visible"] = pair.second.visible;
        item["locked"] = pair.second.locked;

        cfg_overlays.append(item);
    }

    root["overlays"] = cfg_overlays;
    
    std::ofstream file("settings.json");
    if (!file.is_open())
        return false;

    file << root;
    file.close();

    return true;
}
