#pragma once

#include <vector>
#include <string>
#include "gameObj.hpp"

std::vector<GameObject> loadMap(const std::string &fileName, float tileSize = 50.f);