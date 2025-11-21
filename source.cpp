#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <cmath>
#include <map>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include "coinTracker.h"
#include "coinDetector.h"
#include "visualization.h"

extern "C" {
#include "vc.h"
}

// Vari�veis globais
cv::Rect detectionArea;                       // �rea de dete��o central
std::map<std::string, int> coinCounter;       // Contador de moedas por tipo
std::vector<CoinInfo> trackedCoins;           // Vetor de moedas rastreadas
int nextCoinId = 1;                           // ID da pr�xima moeda detetada

int main(void) {
    // Inicializar o contador de moedas
    coinCounter["1c"] = 0;
    coinCounter["2c"] = 0;
    coinCounter["5c"] = 0;
    coinCounter["10c"] = 0;
    coinCounter["20c"] = 0;
    coinCounter["50c"] = 0;
    coinCounter["1E"] = 0;
    coinCounter["2E"] = 0;

    // Nome do ficheiro de v�deo
    char videofile[20] = "video2.mp4";
    cv::VideoCapture capture;

    struct {
        int width, height;
        int framesTotalNum;
        int fps;
        int frameNum;
    } video;

    std::string str;
    int key = 0;

    // Abrir ficheiro de v�deo
    capture.open(videofile);

    if (!capture.isOpened()) {
        std::cerr << "Erro ao abrir o ficheiro de v�deo!\n";
        return 1;
    }

    // Obter propriedades do v�deo
    video.framesTotalNum = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
    video.fps = (int)capture.get(cv::CAP_PROP_FPS);
    video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

    // Definir a �rea de dete��o central (100% da largura e 25% da altura)
    int areaWidth = video.width;
    int areaHeight = video.height * 0.25;
    int areaX = 0;
    int areaY = (video.height - areaHeight) / 2;
    detectionArea = cv::Rect(areaX, areaY, areaWidth, areaHeight);

    // Criar janelas de visualiza��o
    cv::namedWindow("Dete��o de Moedas", cv::WINDOW_AUTOSIZE);
    //cv::namedWindow("Frame RGB", cv::WINDOW_AUTOSIZE);
    //cv::namedWindow("Segmenta��o Pretas", cv::WINDOW_AUTOSIZE);
    //cv::namedWindow("Segmenta��o Douradas", cv::WINDOW_AUTOSIZE);

    // Iniciar cron�metro
    vcTimer();

    cv::Mat frame, frameRGB;
    while (key != 'q') {
        capture.read(frame);
        if (frame.empty()) break;

        video.frameNum = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

        // Converter de BGR para RGB
        cv::cvtColor(frame, frameRGB, cv::COLOR_BGR2RGB);
        //cv::imshow("Frame RGB", frameRGB);

        // Criar imagem IVC a partir do frame RGB
        IVC* src = vc_image_new(video.width, video.height, 3, 255);
        if (src == NULL) {
            std::cerr << "Erro ao criar imagem IVC!" << std::endl;
            continue;
        }
        memcpy(src->data, frameRGB.data, video.width * video.height * 3);

        // Criar imagens bin�rias para segmenta��o
        IVC* imageBronzeSEG = vc_image_new(src->width, src->height, 1, src->levels);
        IVC* imageGoldSEG = vc_image_new(src->width, src->height, 1, src->levels);
        IVC* imageSilverSEG = vc_image_new(src->width, src->height, 1, src->levels);

        // Segmentar moedas nos espa�os de cor apropriados
        coinsSegmentationHSV(src, imageBronzeSEG, imageGoldSEG, imageSilverSEG);

        std::vector<OVC> validBlobs;

    #pragma region Moedas Pretas
        // Processar moedas pretas
        cv::Mat binMatBronze(video.height, video.width, CV_8UC1);
        memcpy(binMatBronze.data, imageBronzeSEG->data, video.width * video.height);
        //cv::imshow("Segmenta��o Pretas", binMatBronze);

        IVC* bronzeLable = vc_image_new(video.width, video.height, 1, 255);
        if (bronzeLable != NULL) {
            int lablesNum = 0;
            OVC* bronzeBlobs = vc_binary_blob_labelling(imageBronzeSEG, bronzeLable, &lablesNum);

            if (bronzeBlobs != NULL) {
                vc_binary_blob_info(bronzeLable, bronzeBlobs, lablesNum, 1);

                for (int i = 0; i < lablesNum; i++) {
                    if (bronzeBlobs[i].area > 5000 && bronzeBlobs[i].area < 40000 && bronzeBlobs[i].type == 1) {
                        validBlobs.push_back(bronzeBlobs[i]);
                    }
                }

                delete[] bronzeBlobs;
            }

            vc_image_free(bronzeLable);
        }
        #pragma endregion
        #pragma region Moedas Douradas
        // Processar moedas douradas
        cv::Mat binMatGold(video.height, video.width, CV_8UC1);
        memcpy(binMatGold.data, imageGoldSEG->data, video.width * video.height);
        //cv::imshow("Segmenta��o Douradas", binMatGold);

        IVC* goldLable = vc_image_new(video.width, video.height, 1, 255);
        if (goldLable != NULL) {
            int nlabels = 0;
            OVC* goldBlobs = vc_binary_blob_labelling(imageGoldSEG, goldLable, &nlabels);

            if (goldBlobs != NULL) {
                vc_binary_blob_info(goldLable, goldBlobs, nlabels, 2);

                for (int i = 0; i < nlabels; i++) {
                    if (goldBlobs[i].area > 14000 && goldBlobs[i].area < 40000 && goldBlobs[i].type == 2) {
                        validBlobs.push_back(goldBlobs[i]);
                    }
                }

                delete[] goldBlobs;
            }

            vc_image_free(goldLable);
        }
        #pragma endregion
        #pragma region Moedas Silver
        // Processar moedas prateadas
        cv::Mat binMatSilver(video.height, video.width, CV_8UC1);
        memcpy(binMatSilver.data, imageSilverSEG->data, video.width * video.height);
        //cv::imshow("Segmenta��o Prateadas", binMatSilver);

        IVC* silverLable = vc_image_new(video.width, video.height, 1, 255);
        if (silverLable != NULL) {
            int nlabels = 0;
            OVC* silverBlobs = vc_binary_blob_labelling(imageSilverSEG, silverLable, &nlabels);

            if (silverBlobs != NULL) {
                vc_binary_blob_info(silverLable, silverBlobs, nlabels, 3);

                for (int i = 0; i < nlabels; i++) {
                    if (silverBlobs[i].area > 16000 && silverBlobs[i].area < 40000 && silverBlobs[i].type == 3) {
                        validBlobs.push_back(silverBlobs[i]);
                    }
                }

                delete[] silverBlobs;
            }

            vc_image_free(silverLable);
        }
        #pragma endregion
        // Atualizar o rastreamento das moedas
        updateCoinTracker(validBlobs);

        // Desenhar caixas e IDs das moedas rastreadas
        drawTrackedCoins(frame);

        // Mostrar estat�sticas de contagem
        showStats(frame);

        // Exibir informa��es do v�deo
        str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 5);
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);

        str = std::string("FRAME: ").append(std::to_string(video.frameNum)).append("/").append(std::to_string(video.framesTotalNum));
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 5);
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);

        str = std::string("MOEDAS: ").append(std::to_string(trackedCoins.size()));
        cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 5);
        cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);

        cv::imshow("Dete��o de Moedas", frame);

        // Libertar mem�ria
        vc_image_free(src);
        vc_image_free(imageBronzeSEG);
        vc_image_free(imageGoldSEG);

        key = cv::waitKey(1);
    }

    // Parar cron�metro e mostrar tempo total
    vcTimer();

    // Fechar janelas e v�deo
    cv::destroyAllWindows();
    capture.release();

    return 0;
}
