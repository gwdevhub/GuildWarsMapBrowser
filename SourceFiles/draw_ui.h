#pragma once
#include "MapBrowser.h"

struct PickingInfo;
void draw_ui(std::map<int, std::unique_ptr<DATManager>>& dat_managers, int& dat_manager_to_show, MapRenderer* map_renderer, PickingInfo picking_info, std::vector<std::vector<std::string>>& csv_data, 
    int& FPS_target, DX::StepTimer& timer, int& pixels_per_tile_x, int& pixels_per_tile_y, bool& pixels_per_tile_changed);
