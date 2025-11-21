#include <cmath>
#include <algorithm>
#include <iostream>
#include "coinTracker.h"
#include "coinDetector.h"

// Calcula a circularidade de um blob
double calcCircularity(int area, int perimeter) {
    if (perimeter == 0) return 0;
    return (4.0 * M_PI * area) / (perimeter * perimeter);
}

// Atualiza o rastreamento das moedas detetadas
void updateCoinTracker(std::vector<OVC>& blobs) {
    // Marcar todas as moedas como inativas no início
    for (auto& coin : trackedCoins) {
        coin.inTracking = false;
    }

    for (size_t i = 0; i < blobs.size(); i++) {
        int centerX = blobs[i].xc;
        int centerY = blobs[i].yc;

        // Ignorar moedas fora da área de deteção
        if (!detectionArea.contains(cv::Point(centerX, centerY))) {
            continue;
        }

        int area = blobs[i].area;
        int perimeter = blobs[i].perimeter;
        double circularity = calcCircularity(area, perimeter);

        std::string coinType = getCoinType(area, blobs[i].type);
        bool wasFound = false;

        // Verifica se já existe uma moeda rastreada próxima
        for (auto& coin : trackedCoins) {
            int dx = centerX - coin.centerX;
            int dy = centerY - coin.centerY;
            double distancia = std::sqrt(dx * dx + dy * dy);

            if (distancia < 50) {
                coin.boundingBox = cv::Rect(blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].height);
                coin.centerX = centerX;
                coin.centerY = centerY;
                coin.inTracking = true;
                coin.frameDetectionGap = 0;
                coin.circularity = circularity;
                coin.area = area;
                coin.perimeter = perimeter;
                coin.blobType = blobs[i].type;
                wasFound = true;
                break;
            }
        }

        // Adiciona nova moeda se não for uma já existente
        if (!wasFound) {
            CoinInfo novaMoeda;
            novaMoeda.id = nextCoinId++;
            novaMoeda.type = coinType;
            novaMoeda.boundingBox = cv::Rect(blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].height);
            novaMoeda.centerX = centerX;
            novaMoeda.centerY = centerY;
            novaMoeda.inTracking = true;
            novaMoeda.frameDetectionGap = 0;
            novaMoeda.circularity = circularity;
            novaMoeda.area = area;
            novaMoeda.perimeter = perimeter;
            novaMoeda.blobType = blobs[i].type;

            trackedCoins.push_back(novaMoeda);

            coinCounter[coinType]++;
        }
    }

    // Atualiza o número de frames sem deteção
    for (auto& coin : trackedCoins) {
        if (!coin.inTracking) {
            coin.frameDetectionGap++;
        }
    }

    // Remove moedas que já não são detetadas há vários frames
    trackedCoins.erase(
        std::remove_if(
            trackedCoins.begin(),
            trackedCoins.end(),
            [](const CoinInfo& coin) { return coin.frameDetectionGap > 15; }
        ),
        trackedCoins.end()
    );
}

// Desenha as moedas rastreadas na imagem
void drawTrackedCoins(cv::Mat& frame) {
    cv::rectangle(frame, detectionArea, cv::Scalar(0, 0, 255), 2);

    for (const auto& coin : trackedCoins) {
        if (coin.inTracking) {
            cv::Scalar coinBoxColor = cv::Scalar(0, 255, 0);
            cv::Scalar centroidColor = cv::Scalar(255, 255, 255);
            cv::Scalar textColor = cv::Scalar(0, 255, 255);

            // Caixa delimitadora
            cv::rectangle(frame, coin.boundingBox, coinBoxColor, 2);

            // Centroide
            cv::circle(frame, cv::Point(coin.centerX, coin.centerY), 3, centroidColor, -1);

            std::string coinText = coin.type + " (ID:" + std::to_string(coin.id) + ")";
            cv::putText(frame, coinText, cv::Point(coin.boundingBox.x, std::max(coin.boundingBox.y - 10, 0)),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);

            // Informações adicionais
            int yOffset = coin.boundingBox.y;
            int xPos = coin.boundingBox.x + coin.boundingBox.width + 5;

            std::string areaText = "Area: " + std::to_string(coin.area);
            std::string boxText = "Box: " + std::to_string(coin.boundingBox.width) + "x" + std::to_string(coin.boundingBox.height);
            std::string circText = "Circularidade: " + std::to_string(std::round(coin.circularity * 100) / 100.0);
            std::string centroidText = "Centroide: (" + std::to_string(coin.centerX) + "," + std::to_string(coin.centerY) + ")";
            std::string perimeterText = "Perimetro: " + std::to_string(coin.perimeter);

            // Decide onde desenhar as informações (ao lado ou abaixo)
            if (xPos + 150 > frame.cols) {
                xPos = coin.boundingBox.x;
                yOffset = coin.boundingBox.y + coin.boundingBox.height + 15;

                cv::putText(frame, areaText, cv::Point(xPos, yOffset), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);
                cv::putText(frame, boxText, cv::Point(xPos, yOffset + 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);
                cv::putText(frame, circText, cv::Point(xPos, yOffset + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);
                cv::putText(frame, centroidText, cv::Point(xPos, yOffset + 45), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);
                cv::putText(frame, perimeterText, cv::Point(xPos, yOffset + 60), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);
            }
            else {
                cv::putText(frame, areaText, cv::Point(xPos, yOffset + 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);
                cv::putText(frame, boxText, cv::Point(xPos, yOffset + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);
                cv::putText(frame, circText, cv::Point(xPos, yOffset + 45), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);
                cv::putText(frame, centroidText, cv::Point(xPos, yOffset + 60), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);
                cv::putText(frame, perimeterText, cv::Point(xPos, yOffset + 75), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);
            }
        }
    }
}
