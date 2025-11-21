//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de funções não seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"
#include <stdbool.h>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar memória para uma imagem
IVC* vc_image_new(int width, int height, int channels, int levels)
{
	IVC* image = (IVC*)malloc(sizeof(IVC));
	if (image == NULL) return NULL;
	if ((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;

	// bytesperline é a quantidade de bytes para uma linha, alinhada ao número de canais
	image->bytesperline = image->width * image->channels;

	// Aloca um bloco contínuo de memória para a imagem
	image->data = (unsigned char*)malloc(image->bytesperline * image->height * sizeof(unsigned char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}

// Libertar memória de uma imagem
IVC* vc_image_free(IVC* image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char* netpbm_get_token(FILE* file, char* tok, int len)
{
	char* t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)));
		if (c != '#') break;
		do c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF) break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#') ungetc(c, file);
	}

	*t = 0;

	return tok;
}


long int unsigned_char_to_bit(unsigned char* datauchar, unsigned char* databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char* p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char* databit, unsigned char* datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char* p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}


IVC* vc_read_image(char* filename)
{
	FILE* file = NULL;
	IVC* image = NULL;
	unsigned char* tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}


int vc_write_image(char* filename, IVC* image)
{
	FILE* file = NULL;
	unsigned char* tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL) return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: ESPAÇOS DE COR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//Função que calcula o negativo de uma imagem em tons de cinzento
int vc_gray_negative(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytheperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) { return 0; }
	if (channels != 1) { return 0; }

	//Inverter a imagem Gray
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytheperline + x * channels;
			//data[pos] = 255 - data[pos];
			data[pos] = srcdst->levels - data[pos];
		}
	}
	return 1;
}

//Gerar negativo da imagem RGB
int vc_rgb_negative(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytheperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) { return 0; }
	if (channels != 3) return 0;

	//Inverter a imagem RGB
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos = y * bytheperline + x * channels;
			data[pos] = 255 - data[pos];
			data[pos + 1] = 255 - data[pos + 1];
			data[pos + 2] = 255 - data[pos + 2];
		}
	}
	return 1;
}

int vc_rgb_to_gray(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}
	}
	return 1;
}

int vc_rgb_to_hsv(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf, hue, max, min;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 3)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			//H: dst->data[pos]
			//S: dst->data[pos+1]
			//V: dst->data[pos+2

			//-> max:
			max = MAX3(rf, gf, bf);
			//-> min:
			min = MIN3(rf, gf, bf);

			if (max == rf) {
				if (gf >= bf) {
					hue = 60 * ((gf - bf) / (max - min));
				}
				else if (bf > gf) {
					hue = 360 + 60 * ((gf - bf) / (max - min));
				}
			}
			else if (max == gf) {
				hue = 120 + 60 * ((bf - rf) / (max - min));
			}
			else if (max == bf) {
				hue = 240 + 60 * ((rf - gf) / (max - min));
			}

			dst->data[pos_dst] = (hue / 360) * 255;
			if (max != min)
				dst->data[pos_dst + 1] = (max - min) / max * 255;
			else
				dst->data[pos_dst + 1] = 0;
			dst->data[pos_dst + 2] = max;

		}
	}
	return 1;
}

int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float hs, ss, vs;
	float contador = 0;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;
	if (dst->levels != 255) return 0;
	if (!(hmin >= 0 && hmax <= 360)) return 0;
	if (!(smin >= 0 && smax <= 255)) return 0;
	if (!(vmin >= 0 && vmax <= 255)) return 0;

	hmin = ((float)hmin * 255) / 360;
	hmax = ((float)hmax * 255) / 360;
	smin = ((float)smin * 255) / 100;
	smax = ((float)smax * 255) / 100;
	vmin = ((float)vmin * 255) / 100;
	vmax = ((float)vmax * 255) / 100;

	//h/360*255
	//s/100*255
	//v/100*255

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			hs = datasrc[pos_src];
			ss = datasrc[pos_src + 1];
			vs = datasrc[pos_src + 2];

			/*if(hs >= hmin && hs <= hmax)
				datadst[pos_dst] = 255;
			else
				datadst[pos_dst] = 0;
			if(ss >= smin && ss <= smax)
				datadst[pos_dst + 1] = 255;
			else
				datadst[pos_dst + 1] = 0;
			if (vs >= vmin && vs <= vmax)
				datadst[pos_dst + 2] = 255;
			else
				datadst[pos_dst + 2] = 0;*/
			if (hs >= hmin && hs <= hmax && ss >= smin && ss <= smax && vs >= vmin && vs <= vmax)
			{
				datadst[pos_dst] = 255;
				contador++;
			}
			else {
				datadst[pos_dst] = 0;
			}
		}
	}
	return contador;
}

int vc_scale_gray_to_color_palette(IVC* src, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 3)) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (datasrc[pos_src] <= 63)
			{
				rf = 0;
				gf = (float)datasrc[pos_src] * 4;
				bf = 255;
			}
			else if (datasrc[pos_src] >= 64 && datasrc[pos_src] <= 127)
			{
				rf = 0;
				gf = 255;
				bf = 255 - ((float)datasrc[pos_src] - 64) * 4;
			}
			else if (datasrc[pos_src] >= 128 && datasrc[pos_src] <= 191)
			{
				rf = ((float)datasrc[pos_src] - 128) * 4;
				gf = 255;
				bf = 0;
			}
			else
			{
				rf = 255;
				gf = 255 - ((float)datasrc[pos_src] - 192) * 4;
				bf = 0;
			}

			datadst[pos_dst] = rf;
			datadst[pos_dst + 1] = gf;
			datadst[pos_dst + 2] = bf;
		}
	}
	return 1;
}

int vce_brain_percentage(IVC* src) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src;
	float rf, gf, bf;
	float rp = 0, yp = 0, gp = 0, bp = 0, brp = 0;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->channels != 3)) return 0;

	IVC* imageDST = vc_image_new(src->width, src->height, 1, src->levels);

	if (imageDST == NULL) {
		printf("ERROR -> vc_image_new():\n\tFail to create file!\n");
		(void)getchar();
		return 0;
	}

	//red
	rp = rp + vc_hsv_segmentation(src, imageDST, 0, 45, 50, 100, 50, 100);
	vc_write_image("vc0008r1.ppm", imageDST);
	rp = rp + vc_hsv_segmentation(src, imageDST, 291, 360, 50, 100, 50, 100);
	vc_write_image("vc0008r2.ppm", imageDST);

	//yellow
	yp = yp + vc_hsv_segmentation(src, imageDST, 46, 70, 50, 100, 50, 100);
	vc_write_image("vc0008y.ppm", imageDST);

	//green
	gp = gp + vc_hsv_segmentation(src, imageDST, 71, 160, 50, 100, 14, 100);
	vc_write_image("vc0008g.ppm", imageDST);

	//blue 
	bp = bp + vc_hsv_segmentation(src, imageDST, 161, 290, 50, 100, 14, 100);
	vc_write_image("vc0008b.ppm", imageDST);

	//brain
	brp = brp + vc_hsv_segmentation(src, imageDST, 1, 359, 50, 100, 10, 100);
	vc_write_image("vc0008w.ppm", imageDST);

	printf("----- Total pixels -----\n");
	printf("Red: %.0f\n", rp);
	printf("Yellow: %.0f\n", yp);
	printf("Green: %.0f\n", gp);
	printf("Blue: %.0f\n", bp);
	printf("Brain: %.0f\n", brp);

	printf("----- Percentage -----\n");
	printf("- %.2f %% of brain with 76%% at 100%% brain activity\n", (rp / brp) * 100);
	printf("- %.2f %% of brain with 51%% at 75%% brain activity\n", (yp / brp) * 100);
	printf("- %.2f %% of brain with 26%% at 50%% brain activity\n", (gp / brp) * 100);
	printf("- %.2f %% of brain with 0%% at 25%% brain activity\n\n\n", (bp / brp) * 100);
	vc_image_free(imageDST);

	return 1;
}

int vc_gray_to_binary(IVC* src, IVC* dst, int threshold) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (datasrc[pos_src] > threshold)
				datadst[pos_dst] = 255;
			else
				datadst[pos_dst] = 0;
		}
	}
	return 1;
}

int vc_binary_invert(IVC* srcdst) {
	if (srcdst == NULL || srcdst->channels != 1) return 0;
	int size = srcdst->width * srcdst->height;
	for (int i = 0; i < size; i++) {
		srcdst->data[i] = 255 - srcdst->data[i];
	}
	return 1;
}

int vc_gray_to_binary_global_mena(IVC* srcdst) {
	unsigned char* datasrc = (unsigned char*)srcdst->data;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels_src = srcdst->channels;
	int width = srcdst->width;
	int height = srcdst->height;
	int x, y;
	long int pos_src;
	float threshold = 0;
	int count = 0;
	float brighttes = 0;

	//Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if ((srcdst->channels != 1)) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline + x * channels_src;
			count++;
			brighttes += datasrc[pos_src];
		}
	}

	float average = brighttes / count;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline + x * channels_src;
			if (datasrc[pos_src] > average)
				datasrc[pos_src] = 255;
			else
				datasrc[pos_src] = 0;
		}
	}
	return 1;
}

int vc_grade_to_binary_grade(IVC* src, IVC* dst, int neighbor) {
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline = src->width * src->channels;
	int width = src->width;
	int height = src->height;
	int offset = (neighbor - 1) / 2;
	int x, y, nx, ny;
	long int pos_src, pos_src_for, pos_dst;
	int Vmax, Vmin, threshold;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	offset = (neighbor - 1) / 2;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline + x;
			pos_dst = pos_src;
			Vmax = 0;
			Vmin = 255;

			for (ny = y - offset; ny <= y + offset; ny++) {
				for (nx = x - offset; nx <= x + offset; nx++) {
					if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
						pos_src_for = ny * bytesperline + nx;
						int pixel_value = datasrc[pos_src_for];

						if (pixel_value > Vmax) Vmax = pixel_value;
						if (pixel_value < Vmin) Vmin = pixel_value;
					}
				}
			}

			threshold = (Vmax + Vmin) / 2;
			if (datasrc[pos_src] > threshold)
				datadst[pos_dst] = 255;
			else
				datadst[pos_dst] = 0;
		}
	}
	return 1;
}

int vc_gray_to_binary_niblack(IVC* src, IVC* dst, int kernel, float k) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int x, y, aux_x, aux_y;
	float avg, std_dev, sum_aux;
	long int pos, pos_aux;
	int pixel_value_cont, threshold, N;

	int neighbors_to_count = (int)kernel / 2;


	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) != (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos = y * bytesperline_src + x * channels_src;

			pixel_value_cont = 0;
			avg = 0.0f;
			sum_aux = 0;
			N = 0;

			for (aux_y = y - neighbors_to_count; aux_y <= y + neighbors_to_count; aux_y++) {
				for (aux_x = x - neighbors_to_count; aux_x <= x + neighbors_to_count; aux_x++) {
					if (aux_x >= 0 && aux_x < width && aux_y >= 0 && aux_y < height) {
						pos_aux = aux_y * bytesperline_src + aux_x * channels_src;
						pixel_value_cont += datasrc[pos_aux];
						N++;
					}
				}
			}

			avg = ((float)pixel_value_cont) / N;


			for (aux_y = y - neighbors_to_count; aux_y <= y + neighbors_to_count; aux_y++) {
				for (aux_x = x - neighbors_to_count; aux_x <= x + neighbors_to_count; aux_x++) {
					if (aux_x >= 0 && aux_x < width && aux_y >= 0 && aux_y < height) {
						pos_aux = aux_y * bytesperline_src + aux_x * channels_src;
						int temp = (datasrc[pos_aux] - avg);
						sum_aux += temp * temp;
					}
				}
			}


			std_dev = sqrt((((float)sum_aux) / N));
			threshold = avg + k * std_dev;

			datadst[pos] = (datasrc[pos] > threshold) ? 255 : 0;
		}
	}

	return 0;
}

int vc_binary_dilate(IVC* src, IVC* dst, int kernel) {
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline = src->width * src->channels;
	int width = src->width;
	int height = src->height;
	int offset = (kernel - 1) / 2;
	int x, y, nx, ny;
	long int pos_src, pos_src_for, pos_dst;
	bool flag = false;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline + x;
			pos_dst = pos_src;
			flag = false;

			for (ny = y - offset; ny <= y + offset; ny++) {
				for (nx = x - offset; nx <= x + offset; nx++) {
					if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
						pos_src_for = ny * bytesperline + nx;
						int pixel_value = datasrc[pos_src_for];
						if (pixel_value == 255) {
							flag = true;
							break;
						}
					}
				}
			}
			datadst[pos_dst] = (flag) ? 255 : 0;
		}
	}
	return 1;
}

int vc_binary_erode(IVC* src, IVC* dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	int bytesperline = width * channels;
	int offset = kernel / 2;
	int x, y, nx, ny;
	long int pos_src, pos_dst, pos_for;
	bool found_black;

	// Verificações
	if (!src || !dst || !src->data || !dst->data) return 0;
	if (src->width != dst->width || src->height != dst->height) return 0;
	if (src->channels != 1 || dst->channels != 1) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			found_black = false;

			for (ny = -offset; ny <= offset && !found_black; ny++)
			{
				for (nx = -offset; nx <= offset; nx++)
				{
					int px = x + nx;
					int py = y + ny;

					if (px >= 0 && px < width && py >= 0 && py < height)
					{
						pos_for = py * bytesperline + px;
						if (datasrc[pos_for] == 0) // se algum vizinho for preto, erosão aplica-se
						{
							found_black = true;
							break;
						}
					}
				}
			}

			pos_dst = y * bytesperline + x;
			datadst[pos_dst] = (found_black) ? 0 : 255;
		}
	}

	return 1;
}


int vc_binary_open(IVC* src, IVC* dst, int kernel1, int kernel2)
{
	IVC* tmp = vc_image_new(src->width, src->height, src->channels, src->levels);
	if (tmp == NULL) return 0;
	vc_binary_erode(src, tmp, kernel1);
	vc_binary_dilate(tmp, dst, kernel2);
	vc_image_free(tmp);
	return 1;
}

int vc_binary_close(IVC* src, IVC* dst, int kernel1, int kernel2) {
	IVC* tmp = vc_image_new(src->width, src->height, src->channels, src->levels);
	if (tmp == NULL) return 0;
	vc_binary_dilate(src, tmp, kernel1);
	vc_binary_erode(tmp, dst, kernel2);
	vc_image_free(tmp);
	return 1;
}

int vc_gray_to_binary_(IVC* src, IVC* dst, int threshold1, int threshold2) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (datasrc[pos_src] > threshold1 && datasrc[pos_src] < threshold2)
				datadst[pos_dst] = 255;
			else
				datadst[pos_dst] = 0;
		}
	}
	return 1;
}

int vc_write_image_binary_to_gray(IVC* src, IVC* bin, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* databin = (unsigned char*)bin->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (databin[pos_src] == 255)
				datadst[pos_dst] = datasrc[pos_src];
			else
				datadst[pos_dst] = 0;
		}
	}
	return 1;
}

int vc_copy_image_binary(IVC* src, IVC* dst) {
	if (src == NULL || src->data == NULL) return 0;
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y, label = 1, low_label = 1;
	bool flag = false;
	long int pos_src, pos_dst, pos_src_left, pos_src_right_top, pos_src_top, pos_src_bottom, pos_src_left_top;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (int y = 0; y < src->height; y++)
		for (int x = 0; x < src->width; x++)
		{
			pos_src = y * bytesperline + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			datadst[pos_dst] = datasrc[pos_src];
		}
	return 1;
}

int vc_binary_blob_labelling_(IVC* src, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y, label = 100, low_label = 100;
	bool flag = false;
	long int pos_src, pos_dst, pos_src_left, pos_src_right_top, pos_src_top, pos_src_bottom, pos_src_left_top;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	vc_copy_image_binary(src, dst);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;
			pos_src_left = y * bytesperline + (x - 1) * channels_src;
			pos_src_top = (y - 1) * bytesperline + x * channels_src;
			pos_src_right_top = (y - 1) * bytesperline + (x + 1) * channels_src;
			pos_src_left_top = (y - 1) * bytesperline + (x - 1) * channels_src;

			low_label = label;
			flag = false;

			if (datadst[pos_src] == 255) {
				if (pos_src_left >= 0 /* && pos_src_left < width*/)
				{
					if (datadst[pos_src_left] == 0) {

					}
					else {
						flag = true;
						if (datadst[pos_src_left] < low_label && datadst[pos_src_left] != 0)
							low_label = datadst[pos_src_left];
					}
				}
				if (pos_src_right_top >= 0 /* && pos_src_right_top < width*/)
				{
					if (datadst[pos_src_right_top] == 0) {

					}
					else {
						flag = true;
						if (datadst[pos_src_right_top] < low_label && datadst[pos_src_right_top] != 0)
							low_label = datadst[pos_src_right_top];
					}
				}
				if (pos_src_left_top >= 0 /* && pos_src_left_top < width */)
				{
					if (datadst[pos_src_left_top] == 0) {

					}
					else {
						flag = true;
						if (datadst[pos_src_left_top] < low_label && datadst[pos_src_left_top] != 0)
							low_label = datadst[pos_src_left_top];
					}
				}
				if (pos_src_top >= 0 /* && pos_src_top < width*/)
				{
					if (datadst[pos_src_top] == 0) {

					}
					else {
						flag = true;
						if (datadst[pos_src_top] < low_label && datadst[pos_src_top] != 0)
							low_label = datadst[pos_src_top];
					}
				}
				if (flag)
					if (low_label == 0)
						datadst[pos_src] = 0;
					else
						datadst[pos_src] = low_label;
				else {
					datadst[pos_src] = label;
					label++;
				}

			}

		}
	}
	return 1;
}

// Etiquetagem de blobs
// src		: Imagem binária de entrada
// dst		: Imagem grayscale (irá conter as etiquetas)
// nlabels	: Endereço de memória de uma variável, onde será armazenado o número de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. É necessário libertar posteriormente esta memória.
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[1000] = { 0 };
	int labelarea[1000] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC* blobs; // Apontador para array de blobs (objectos) que será retornado desta função.

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binária para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixéis de plano de fundo devem obrigatóriamente ter valor 0
	// Todos os pixéis de primeiro plano devem obrigatóriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1,254]
	// Este algoritmo está assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A está marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B está marcado, e é menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C está marcado, e é menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D está marcado, e é menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do número de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que não hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se não há blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}


int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs, int type)
{
	int x, y, n;
	long int pos;
	unsigned char* data = (unsigned char*)src->data;

	// Verificação de parâmetros
	if (data == NULL || blobs == NULL || nblobs <= 0 || src->channels != 1)
		return 0;

	// Processamento de cada blob
	for (n = 0; n < nblobs; n++)
	{
		int label = blobs[n].label;
		int area = 0;
		int sumX = 0, sumY = 0;
		int perimeter = 0;
		int xmin = src->width, xmax = 0, ymin = src->height, ymax = 0;

		for (y = 0; y < src->height; y++)
		{
			for (x = 0; x < src->width; x++)
			{
				// Usar bytesperline para acessar corretamente os pixels
				pos = y * src->bytesperline + x;

				if (data[pos] == label)
				{
					// Atualizar área
					area++;

					// Acumular para centro de massa
					sumX += x;
					sumY += y;

					// Atualizar bounding box
					if (x < xmin) xmin = x;
					if (x > xmax) xmax = x;
					if (y < ymin) ymin = y;
					if (y > ymax) ymax = y;

					// Verificar contorno (perímetro) com verificações seguras
					bool isBorder = false;

					// Verificação do pixel à esquerda
					if (x == 0 || (x > 0 && data[pos - 1] != label))
						isBorder = true;
					// Verificação do pixel à direita
					else if (x == src->width - 1 || (x < src->width - 1 && data[pos + 1] != label))
						isBorder = true;
					// Verificação do pixel acima
					else if (y == 0 || (y > 0 && data[pos - src->bytesperline] != label))
						isBorder = true;
					// Verificação do pixel abaixo
					else if (y == src->height - 1 || (y < src->height - 1 && data[pos + src->bytesperline] != label))
						isBorder = true;

					if (isBorder)
						perimeter++;
				}
			}
		}

		// Preencher dados do blob
		blobs[n].area = area;

		// Verificar se o blob é válido antes de calcular outras propriedades
		if (area > 0)
		{
			blobs[n].x = xmin;
			blobs[n].y = ymin;
			blobs[n].width = xmax - xmin + 1;
			blobs[n].height = ymax - ymin + 1;
			blobs[n].xc = sumX / area;
			blobs[n].yc = sumY / area;
			blobs[n].perimeter = perimeter;
			blobs[n].type = type;
			blobs[n].radius = (int)sqrt((double)area / M_PI); // Raio aproximado
		}
		else
		{
			// Inicializar valores para um blob inválido
			blobs[n].x = 0;
			blobs[n].y = 0;
			blobs[n].width = 0;
			blobs[n].height = 0;
			blobs[n].xc = 0;
			blobs[n].yc = 0;
			blobs[n].perimeter = 0;
			blobs[n].radius = 0;
		}
	}

	return 1;
}



int vc_gray_edge_prewitt(IVC* src, IVC* dst, float th) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	int mask_x[3][3] = { { -1, 0, 1 }, { -1, 0, 1 }, { -1, 0, 1 } };
	int mask_y[3][3] = { { -1, -1, -1 }, { 0, 0, 0 }, { 1, 1, 1 } };
	float sumX = 0, sumY = 0, mag;
	int posA, posB, posC, posD, posE, posF, posG, posH, posX;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 1; y < height - 1; y++) {
		for (x = 1; x < width - 1; x++) {

			// POSA POSB POSC
			// POSD POSX POSE
			// POSF POSG POSH

			posA = (y - 1) * bytesperline + (x - 1) * channels_src; // A
			posB = (y - 1) * bytesperline + x * channels_src; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels_src; // C
			posD = y * bytesperline + (x - 1) * channels_src; // D
			posX = y * bytesperline + x * channels_src; // X
			posE = y * bytesperline + (x + 1) * channels_src; // E
			posF = (y + 1) * bytesperline + (x - 1) * channels_src; // F
			posG = (y + 1) * bytesperline + x * channels_src; // G
			posH = (y + 1) * bytesperline + (x + 1) * channels_src; // H

			// PosA*(-1) + PosB*(0) + PosC*(1)
			// PosD*(-1) + PosX*(0) + PosE*(1)
			// PosF*(-1) + PosG*(0) + PosH*(1)

			sumX = datasrc[posA] * mask_x[0][0];
			sumX += datasrc[posB] * mask_x[0][1];
			sumX += datasrc[posC] * mask_x[0][2];

			sumX += datasrc[posD] * mask_x[1][0];
			sumX += datasrc[posX] * mask_x[1][1];
			sumX += datasrc[posE] * mask_x[1][2];

			sumX += datasrc[posF] * mask_x[2][0];
			sumX += datasrc[posG] * mask_x[2][1];
			sumX += datasrc[posH] * mask_x[2][2];

			sumX = sumX / 3; //3 = 1+1+1

			//PosA*(-1) + PosB*(-1) + PosC*(-1)
			//PosD*(0) + PosX*(0) + PosE*(0)
			//PosF*(1) + PosG*(1) + PosH*(1)

			sumY = datasrc[posA] * mask_y[0][0];
			sumY += datasrc[posB] * mask_y[0][1];
			sumY += datasrc[posC] * mask_y[0][2];

			sumY += datasrc[posD] * mask_y[1][0];
			sumY += datasrc[posX] * mask_y[1][1];
			sumY += datasrc[posE] * mask_y[1][2];

			sumY += datasrc[posF] * mask_y[2][0];
			sumY += datasrc[posG] * mask_y[2][1];
			sumY += datasrc[posH] * mask_y[2][2];

			sumY = sumY / 3; //3 = 1+1+1

			//datadst[posX] ) (unsigned char)sqrt((sumX * sumX) + (sumY * sumY));
			mag = (unsigned char)(sqrt((double)(sumX * sumX + sumY * sumY)) / sqrt(2.0));

			if (mag > th)
			{
				datadst[posX] = 255;
			}
			else
			{
				datadst[posX] = 0;
			}
		}
	}
	return 1;
}

int vc_rgb_get_green(IVC* srcdst) {
	if (srcdst == NULL || srcdst->channels != 3) return 0;

	int bytesperline = srcdst->bytesperline;
	int width = srcdst->width;
	int height = srcdst->height;
	unsigned char* data = srcdst->data;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int pos = y * bytesperline + x * srcdst->channels;

			// Set red and blue channels to 0, keep green channel
			data[pos] = 0;       // Red
			data[pos + 2] = 0;   // Blue
		}
	}

	return 1;
}

int vc_draw_circle(IVC* image, int x0, int y0, int radius, int value)
{
	unsigned char* data = (unsigned char*)image->data;
	int width = image->width;
	int height = image->height;
	int bytesperline = image->bytesperline;
	int channels = image->channels;
	int x, y;
	long int pos;

	if ((x0 < 0) || (y0 < 0) || (x0 >= width) || (y0 >= height)) return 0;
	if ((image->data == NULL) || (channels != 1 && channels != 3)) return 0;

	for (int angle = 0; angle < 360; angle++)
	{
		int dx = (int)(x0 + radius * cos(angle * M_PI / 180.0));
		int dy = (int)(y0 + radius * sin(angle * M_PI / 180.0));

		if (dx >= 0 && dx < width && dy >= 0 && dy < height)
		{
			pos = dy * bytesperline + dx * channels;

			if (channels == 1)
			{
				data[pos] = value;
			}
			else if (channels == 3)
			{
				// Desenhar em vermelho puro
				data[pos + 0] = 0;     // Blue
				data[pos + 1] = 0;     // Green
				data[pos + 2] = 255;   // Red
			}
		}
	}

	return 1;
}

int vc_draw_rectangle(IVC* image, int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b, int thickness)
{
	if (!image || image->channels < 3) return;

	int img_w = image->width;
	int img_h = image->height;
	unsigned char* data = image->data;

	// Desenha 4 linhas (bordas do retângulo)

	// Top border
	for (int ty = y; ty < y + thickness; ty++) {
		if (ty < 0 || ty >= img_h) continue;
		for (int tx = x; tx < x + w; tx++) {
			if (tx < 0 || tx >= img_w) continue;
			int idx = (ty * img_w + tx) * 3;
			data[idx] = r;
			data[idx + 1] = g;
			data[idx + 2] = b;
		}
	}

	// Bottom border
	for (int ty = y + h - thickness; ty < y + h; ty++) {
		if (ty < 0 || ty >= img_h) continue;
		for (int tx = x; tx < x + w; tx++) {
			if (tx < 0 || tx >= img_w) continue;
			int idx = (ty * img_w + tx) * 3;
			data[idx] = r;
			data[idx + 1] = g;
			data[idx + 2] = b;
		}
	}

	// Left border
	for (int tx = x; tx < x + thickness; tx++) {
		if (tx < 0 || tx >= img_w) continue;
		for (int ty = y; ty < y + h; ty++) {
			if (ty < 0 || ty >= img_h) continue;
			int idx = (ty * img_w + tx) * 3;
			data[idx] = r;
			data[idx + 1] = g;
			data[idx + 2] = b;
		}
	}

	// Right border
	for (int tx = x + w - thickness; tx < x + w; tx++) {
		if (tx < 0 || tx >= img_w) continue;
		for (int ty = y; ty < y + h; ty++) {
			if (ty < 0 || ty >= img_h) continue;
			int idx = (ty * img_w + tx) * 3;
			data[idx] = r;
			data[idx + 1] = g;
			data[idx + 2] = b;
		}
	}

	return 1;
}

// Mapear caractere para índice da fonte
int char_to_font_index(char c) {
	if (c >= '0' && c <= '2') return c - '0';
	if (c == '5') return 3;
	if (c == 'c') return 4;
	if (c == 'e') return 5;
	if (c == 's') return 6;
	if (c == 't') return 7;
	if (c == 'i') return 8;
	if (c == ' ') return 9;
	return 9; // espaço para desconhecidos
}

//// Função para desenhar um caractere na imagem (5x7 pixels)
int vc_draw_char(IVC* image, int x, int y, char c, unsigned char r, unsigned char g, unsigned char b)
{
//	int idx = char_to_font_index(c);
//	if (idx < 0) return;
//
//	int img_w = image->width;
//	int img_h = image->height;
//	unsigned char* data = image->data;
//
//	for (int col = 0; col < 5; col++) {
//		unsigned char line = font_5x7[idx][col];
//		for (int row = 0; row < 7; row++) {
//			if (line & (1 << row)) {
//				int px = x + col;
//				int py = y + row;
//				if (px >= 0 && px < img_w && py >= 0 && py < img_h) {
//					int pos = (py * img_w + px) * 3;
//					data[pos] = r;
//					data[pos + 1] = g;
//					data[pos + 2] = b;
//				}
//			}
//		}
//	}
return 1;
}

// Função para desenhar uma string na imagem
int vc_draw_text(IVC* image, int x, int y, const char* text, unsigned char r, unsigned char g, unsigned char b)
{
	int cursor_x = x;
	while (*text) {
		vc_draw_char(image, cursor_x, y, *text, r, g, b);
		cursor_x += 6; // 5 pixels largura + 1 pixel espaço
		text++;
	}
	return 1;
}

int vc_mask_all(IVC* src, IVC* dst) {
	
	// 1, 2 e 5 c
	int h_min1 = 16, s_min1 = 31, v_min1 = 10;
	int h_max1 = 38, s_max1 = 82, v_max1 = 36;

	// 10, 20 e 50 c
	int h_min2 = 43, s_min2 = 21, v_min2 = 7;
	int h_max2 = 72, s_max2 = 68, v_max2 = 64;

	// 1, 2€
	int h_min3 = 13, s_min3 = 0, v_min3 = 0;
	int h_max3 = 90, s_max3 = 35, v_max3 = 36;

	IVC* imageHSV, * image1, * image2, * image3;
	IVC* image11, * image12;
	IVC* image21, * image22;
	IVC* image31, * image32;

	imageHSV = vc_image_new(src->width, src->height, src->channels, src->levels);
	image1 = vc_image_new(src->width, src->height, 1, src->levels);
	image2 = vc_image_new(src->width, src->height, 1, src->levels);
	image3 = vc_image_new(src->width, src->height, 1, src->levels);
	image11 = vc_image_new(src->width, src->height, 1, src->levels);
	image12 = vc_image_new(src->width, src->height, 1, src->levels);
	image21 = vc_image_new(src->width, src->height, 1, src->levels);
	image22 = vc_image_new(src->width, src->height, 1, src->levels);
	image31 = vc_image_new(src->width, src->height, 1, src->levels);
	image32 = vc_image_new(src->width, src->height, 1, src->levels);

	vc_rgb_to_hsv(src, imageHSV); // HSV
	vc_write_image((char*)"Teste.ppm", imageHSV);
	//system("FilterGear Teste.ppm");


	//Mascara para 1, 2 e 5c
	vc_hsv_segmentation(imageHSV, image1, h_min1, h_max1, s_min1, s_max1, v_min1, v_max1);
	vc_binary_close(image1, image11, 5, 5);
	vc_binary_open(image11, image12, 5, 5);
	//vc_write_image((char*)"Teste1.ppm", image12);
	//system("FilterGear Teste1.ppm");	

	//Mascara para 10, 20 e 50c
	vc_hsv_segmentation(imageHSV, image2, h_min2, h_max2, s_min2, s_max2, v_min2, v_max2);
	vc_binary_close(image2, image21, 5, 5);
	vc_binary_open(image21, image22, 5, 5);
	//vc_write_image((char*)"Teste2.ppm", image22);
	//system("FilterGear Teste2.ppm");

	//Mascara para 1, 2€
	vc_hsv_segmentation(imageHSV, image3, h_min3, h_max3, s_min3, s_max3, v_min3, v_max3);
	vc_binary_close(image3, image31, 7, 3);
	vc_binary_open(image31, image32, 3, 7);
	//vc_write_image((char*)"Teste3.ppm", image32);
	//system("FilterGear Teste3.ppm");

	//combinar tudo
	vc_binary_or(image12, image22, image32, dst);

	// Liberação de memória
	vc_image_free(imageHSV);
	vc_image_free(image1); vc_image_free(image2); vc_image_free(image3);
	vc_image_free(image11); vc_image_free(image12);
	vc_image_free(image21); vc_image_free(image22);
	vc_image_free(image31); vc_image_free(image32);

	return 1;
}

int vc_binary_or(IVC* src1, IVC* src2, IVC* src3, IVC* dst)
{
	if (!src1 || !src2 || !src3 || !dst) return 0;
	if (src1->width != src2->width || src1->width != src3->width || src1->width != dst->width) return 0;
	if (src1->height != src2->height || src1->height != src3->height || src1->height != dst->height) return 0;
	if (src1->channels != 1 || src2->channels != 1 || src3->channels != 1 || dst->channels != 1) return 0;

	int size = src1->width * src1->height;
	unsigned char* data1 = src1->data;
	unsigned char* data2 = src2->data;
	unsigned char* data3 = src3->data;
	unsigned char* datadst = dst->data;

	for (int i = 0; i < size; i++)
	{
		if (data1[i] == 255 || data2[i] == 255 || data3[i] == 255)
			datadst[i] = 255;
		else
			datadst[i] = 0;
	}
	return 1;
}

int vc_binary_segmentation(IVC* src, IVC* dst, int h_min, int h_max, int s_min, int s_max, int v_min, int v_max)
{
	IVC* imageHSV = vc_image_new(src->width, src->height, 3, src->levels);
	IVC* image1 = vc_image_new(src->width, src->height, 1, src->levels);
	IVC* image2 = vc_image_new(src->width, src->height, 1, src->levels);
	IVC* image3 = vc_image_new(src->width, src->height, 1, src->levels);
	vc_rgb_to_hsv(src, imageHSV); // HSV
	vc_write_image((char*)"METEOHSV.ppm", imageHSV);
	/*system("FilterGear Teste.ppm");*/
	vc_hsv_segmentation(imageHSV, image1, h_min, h_max, s_min, s_max, v_min, v_max);
	vc_binary_close(image1, image2, 13, 7);
	vc_binary_open(image2, image3, 7, 5);
	vc_copy_image_binary(image3, dst);
	// Liberação de memória
	vc_image_free(imageHSV);
	vc_image_free(image1);
	vc_image_free(image2);
	vc_image_free(image3);
	return 1;
}

int vc_binary_or2(IVC* src1, IVC* src2, IVC* dst)
{
	if (!src1 || !src2 || !dst) return 0;
	if (src1->width != src2->width || src1->width != dst->width) return 0;
	if (src1->height != src2->height || src1->height != dst->height) return 0;
	if (src1->channels != 1 || src2->channels != 1 || dst->channels != 1) return 0;

	int size = src1->width * src1->height;
	unsigned char* data1 = src1->data;
	unsigned char* data2 = src2->data;
	unsigned char* datadst = dst->data;

	for (int i = 0; i < size; i++)
	{
		if (data1[i] == 255 || data2[i] == 255)
			datadst[i] = 255;
		else
			datadst[i] = 0;
	}
	return 1;
}