# Contagem Automática de Moedas em Vídeo (Visão por Computador – C/C++)

Projeto desenvolvido em C/C++ no âmbito da unidade curricular **Visão por Computador**.

O projeto analisa vídeos e faz a **detecção, classificação e quantificação automática de moedas**, apresentando estatísticas detalhadas sobre as mesmas.

---

## Objetivo

Dado um ficheiro de vídeo (`*.mp4`), o programa deve:

- identificar todas as moedas visíveis ao longo do vídeo;
- calcular o **somatório total de dinheiro**;
- apresentar estatísticas por tipo de moeda;
- mostrar informação geométrica relevante (área, perímetro, centroide, etc.).

---

## Funcionalidades Principais

Durante e/ou após o processamento de um vídeo, o sistema deve disponibilizar:

- **Número total de moedas** observadas ao longo do vídeo;
- **Número de moedas por tipo**:
  - 1, 2, 5, 10, 20, 50 cêntimos;
  - 1 e 2 euros;
- **Área e perímetro** (em píxeis) de cada moeda;
- **Desenho de informação sobre o vídeo**:
  - caixa delimitadora (bounding box) de cada moeda;
  - centro de massa (centroide);
  - indicação do tipo de moeda detetada (ex.: “10c”, “1€”).

Características dos vídeos fornecidos:

- Resolução: **1280x720**
- Frame rate: **30 fps**

---

## Conceitos de Visão por Computador

O sistema deve recorrer a:

- **Segmentação** por tonalidade e/ou brilho;
- **Melhoramento de imagem** (ex.: remoção de ruído);
- **Análise de imagem** para cálculo de:
  
  - área;
  - caixa delimitadora;
  - circularidade;
  - centroide;
  - outras métricas consideradas relevantes;
    
- **Algoritmos de distinção de moedas**, com base nas características extraídas.

---

## Tecnologias e Restrições

- Linguagem: **C ou C++**
- Biblioteca: **OpenCV**
- É fornecido código de exemplo para:
  - leitura/captura de vídeo;
  - exibição de frames.

Restrições importantes:

- Para além das funções OpenCV usadas no `CodigoExemplo.cpp`, só podem ser utilizadas **até mais 3 funções ou instâncias de classes OpenCV** adicionais.  
- Não é permitida a utilização de:
  - outras bibliotecas externas de processamento de imagem;
  - código copiado de repositórios públicos (GitHub, etc.);
  - qualquer código cuja autoria não seja dos elementos do grupo.

---

# Execução
./vc_moedas data/video1.mp4
