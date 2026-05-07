#ifndef ASSETUTILS_H
#define ASSETUTILS_H

#include <algorithm>
#include <vector>

template <typename T>
void SortSingleAssetList(std::vector<T>& asset_list) {
    std::sort(asset_list.begin(), asset_list.end(), [](const T& a, const T& b) {
        return a.file_path.parent_path() < b.file_path.parent_path();
    });
}

// variadic templates, gets all the parameter, no need to call it 3-5 times over and over.
template <typename... Args>
void SortAllAssets(Args&... asset_lists) {

    (SortSingleAssetList(asset_lists), ...); 
}
#endif