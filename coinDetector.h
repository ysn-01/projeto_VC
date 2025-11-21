#pragma once
#ifndef COIN_DETECTOR_H
#define COIN_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <vector>

extern "C" {
#include "vc.h"
}

// Estrutura para classificação de moedas baseada em área
std::string getCoinType(int area, int type);

// Função para segmentar moedas no espaço de cores HSV
int coinsSegmentationHSV(IVC* src, IVC* imageSEG1, IVC* imageSEGD, IVC* imageSEGS);

#endif // COIN_DETECTOR_H