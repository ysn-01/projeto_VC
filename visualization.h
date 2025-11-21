#pragma once

#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <opencv2/opencv.hpp>
#include "coinTracker.h"

// Desenha as caixas delimitadoras e IDs das moedas no frame
void drawTrackedCoins(cv::Mat& frame);
void vcTimer(void);
// Mostra estatísticas das moedas no frame
void showStats(cv::Mat& frame);

#endif // VISUALIZATION_H