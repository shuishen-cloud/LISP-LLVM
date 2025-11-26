/*
 * Filename:  stl_test.cpp
 * Project:   lwy
 * Author:    lwy
 * ***
 * Created:   2025/11/20 Thursday 20:21:49
 * Modified:  2025/11/20 Thursday 20:22:02
 * ***
 * Description: 用来学习 STL 的用法
 */

#include <algorithm>
#include <iostream>
#include <vector>

void displayVector(std::vector<int> vec) {
    for (auto v : vec) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
}

int main(void) {
    // vector 初始化
    std::vector<int> vec{1, 2, 3};

    displayVector(vec);

    // vector 增加元素
    auto vecBeginValue = vec.back();

    for (int i = vecBeginValue + 1; i < vecBeginValue + 5; i++) {
        vec.push_back(i);
    }

    displayVector(vec);

    // vector 删除元素，
    // * 什么都要使用一下 irator ，实在是不是很方便

    auto it = find(vec.begin(), vec.end(), 3);
    return 0;
}
