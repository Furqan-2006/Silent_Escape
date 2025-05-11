#pragma once

#include <vector>
#include <string>
#include "gameObj.hpp"
#include "textureManager.hpp"

std::vector<GameObject> loadMap(const std::string &fileName, TextureManager &textureManager, float tileSize = 40.f);
std::vector<std::vector<int>> loadGridMap(const std::string &fileName);
