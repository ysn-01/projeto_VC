//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma once

#define VC_DEBUG
#define MAX3(a,b,c) ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))
#define MIN3(a,b,c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))
#define MAX(a,b) (a > b ? a : b)

#ifndef M_PI  
#define M_PI 3.14159265358979323846  
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char* data;
	int width, height;
	int channels;			// Binário/Cinzentos=1; RGB=3
	int levels;				// Binário=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UM BLOB (OBJECTO)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    int x, y, width, height;	// Caixa Delimitadora (Bounding Box)  
    int area;					// Área  
    int xc, yc;					// Centro-de-massa  
    int perimeter;				// Perímetro  
    int label;					// Etiqueta  
    int radius;					// Raio (adicionado para corrigir o erro E0135)
	int type;                   // 1= preta, 2= dourada
} OVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC* vc_read_image(char* filename);
int vc_write_image(char* filename, IVC* image);

// FUNÇÕES: ESPAÇOS DE COR
int vc_gray_negative(IVC* srcdst);
int vc_rgb_negative(IVC* srcdst);
int vc_rgb_to_gray(IVC* src, IVC* dst);
int vc_rgb_to_hsv(IVC* src, IVC* dst);
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_scale_gray_to_color_pallete(IVC* src, IVC* dst);
int vce_brain_percentage(IVC* src);
int vc_gray_to_binary(IVC* src, IVC* dst, int threshold);
int vc_gray_to_binary_global_mena(IVC* srcdst);
int vc_grade_to_binary_grade(IVC* src, IVC* dst, int neighbor);
int vc_gray_to_binary_niblack(IVC* src, IVC* dst, int kernel, float k);
int vc_binary_dilate(IVC* src, IVC* dst, int kernel);
int vc_binary_erode(IVC* src, IVC* dst, int kernel);
int vc_binary_open(IVC* src, IVC* dst, int kernel1, int kernel2);
int vc_binary_close(IVC* src, IVC* dst, int kernel1, int kernel2);
int vc_gray_to_binary_(IVC* src, IVC* dst, int threshold1, int threshold2);
int vc_write_image_binary_to_gray(IVC* src, IVC* bin, IVC* dst);
int vc_binary_blob_labelling_(IVC* src, IVC* dst);

OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels);
int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs, int type);

int vc_gray_edge_prewitt(IVC* src, IVC* dst, float th);
int vc_rgb_get_green(IVC* srcdst);
int vc_draw_circle(IVC* image, int x0, int y0, int radius, int value);

int vc_binary_invert(IVC* srcdst);
int vc_draw_rectangle(IVC* image, int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b, int thickness);
int vc_draw_char(IVC* image, int x, int y, char c, unsigned char r, unsigned char g, unsigned char b);
int vc_draw_text(IVC* image, int x, int y, const char* text, unsigned char r, unsigned char g, unsigned char b);
int vc_mask_all(IVC* src, IVC* dst);
int vc_binary_segmentation(IVC* src, IVC* dst, int h_min, int h_max, int s_min, int s_max, int v_min, int v_max);
int vc_binary_or2(IVC* src1, IVC* src2, IVC* dst);
