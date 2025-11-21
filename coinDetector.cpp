#include "coinDetector.h"
#include <iostream>

// Classifica uma moeda com base na sua área e tipo de blob
std::string getCoinType(int area, int blobType) {
    if (blobType == 1) { // Moedas escuras (bronze)
        if (area >= 15000 && area < 19000) return "5c";
        else if (area >= 11500 && area < 14500) return "2c";
        else if (area >= 8000 && area < 11000) return "1c";
    }
    else if (blobType == 2) { // Moedas douradas
        if (area >= 22500 && area < 25000) return "50c";
        else if (area >= 18000 && area < 22000) return "20c";
        else if (area >= 14500 && area < 18000) return "10c";
        
    }
    else if (blobType == 3) {
        if (area >= 16000 && area < 22000) return "1E";
        else if (area >= 22000 && area < 29000) return "2E";
    }
}

// Segmenta moedas na imagem com base no espaço HSV
int coinsSegmentationHSV(IVC* srcImage, IVC* dstImageBronze, IVC* dstImageGold, IVC* dstImageSilver) {
    // Verificações básicas
    if ((srcImage == NULL) || (dstImageBronze == NULL) || (dstImageGold == NULL)|| (dstImageSilver == NULL)) return 0;
    if ((srcImage->width <= 0) || (srcImage->height <= 0) || (srcImage->data == NULL)) return 0;
    if ((dstImageBronze->width != srcImage->width) || (dstImageBronze->height != srcImage->height)|| (dstImageSilver->height != srcImage->height)) return 0;
    if ((dstImageGold->width != srcImage->width) || (dstImageGold->height != srcImage->height)|| (dstImageSilver->height != srcImage->height)) return 0;

    // Conversão de RGB para HSV
    IVC* hsvImage = vc_image_new(srcImage->width, srcImage->height, 3, 255);
    if (hsvImage == NULL) return 0;

    if (vc_rgb_to_hsv(srcImage, hsvImage) == 0) {
        vc_image_free(hsvImage);
        return 0;
    }

    IVC* tempBronze = vc_image_new(dstImageBronze->width, dstImageBronze->height, 1, 255);
    IVC* tempGold = vc_image_new(dstImageGold->width, dstImageGold->height, 1, 255);
    IVC* tempSilver = vc_image_new(dstImageSilver->width, dstImageSilver->height, 1, 255);

    // Segmentação por intervalos HSV
    vc_hsv_segmentation(hsvImage, tempBronze, 16, 38, 31, 82, 10, 47); // Moedas bronze
    vc_hsv_segmentation(hsvImage, tempGold, 38, 75, 28, 65, 17, 61);   // Moedas douradasd
    vc_hsv_segmentation(hsvImage, tempSilver, 9, 153, 0, 101, 15, 40);   // Moedas prateadas

    // Operações morfológicas para limpar a segmentação
    vc_binary_close(tempBronze, dstImageBronze, 7, 7);
    vc_image_free(tempBronze);

    vc_binary_close(tempGold, dstImageGold, 7, 7);
    vc_image_free(tempGold);
    
    vc_binary_close(tempSilver, dstImageSilver, 11, 7);
    vc_image_free(tempSilver);

    vc_image_free(hsvImage);
    return 1;
}
