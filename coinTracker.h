#ifndef COIN_TRACKER_H
#define COIN_TRACKER_H

#include <vector>
#include <string>
#include <map>
#include <opencv2/opencv.hpp>

extern "C" {
#include "vc.h"
}

// Estrutura que armazena informações sobre cada moeda rastreada
typedef struct {
    int id;                        // Identificador único da moeda
    std::string type;              // Tipo da moeda (ex: 1c, 2c, 5c)
    cv::Rect boundingBox;          // Caixa delimitadora da moeda
    int centerX, centerY;          // Coordenadas do centro (centroide)
    bool inTracking;               // Indica se está a ser rastreada no frame atual
    int frameDetectionGap;         // Nº de frames desde a última deteção
    double circularity;            // Circularidade (4*π*área/perímetro²)
    int area;                      // Área do blob
    int perimeter;                 // Perímetro do blob
    int blobType;                  // Tipo do blob: 1 = preto, 2 = dourado
} CoinInfo;

// Variáveis globais
extern cv::Rect detectionArea;                  // Área onde as moedas são consideradas válidas
extern std::map<std::string, int> coinCounter;  // Contador acumulativo por tipo de moeda
extern std::vector<CoinInfo> trackedCoins;      // Lista das moedas atualmente rastreadas
extern int nextCoinId;                          // Próximo ID a atribuir a uma nova moeda

// Funções principais do rastreamento
double calcCircularity(int area, int perimetro);
void updateCoinTracker(std::vector<OVC>& blobs);
void drawTrackedCoins(cv::Mat& frame);

#endif // COIN_TRACKER_H

