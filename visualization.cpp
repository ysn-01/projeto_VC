#pragma once
#include <iostream>
#include <chrono>
#include <cmath>
#include "coinTracker.h"
#include "coinDetector.h"

// Função auxiliar que mede o tempo decorrido entre duas chamadas
void vcTimer(void) {
    static bool running = false; // Indica se o temporizador já foi iniciado
    static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now(); // Guarda o instante da chamada anterior

    if (!running) {
        // Primeira chamada apenas inicia o temporizador
        running = true;
    }
    else {
        // Segunda chamada calcula e exibe o tempo decorrido
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

        // Converte o tempo para segundos
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
        double nseconds = time_span.count();

        std::cout << "Tempo decorrido: " << nseconds << " segundos" << std::endl;
        std::cout << "Pressione qualquer tecla para continuar...\n";
        std::cin.get(); // Aguarda uma tecla do utilizador
    }
}

// Função para exibir as estatísticas (número e valor total) das moedas detetadas
void showStats(cv::Mat& frame) {
    // Coordenadas iniciais para desenhar o texto no canto superior direito
    int x = frame.cols - 200;
    int y = 30;
    int total = 0;         // Total de moedas
    double valor = 0.0;    // Valor total em euros

    // Itera sobre o mapa coinCounter para mostrar o número de moedas por tipo
    for (const auto& par : coinCounter) {
        std::string str = par.first + ": " + std::to_string(par.second) + " Moeda(s)";

        // Desenha o texto com sombra preta (para melhor visibilidade)
        cv::putText(frame, str, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 5);
        // Texto branco por cima
        cv::putText(frame, str, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
        y += 20;

        // Determina o valor da moeda correspondente
        double v = 0.0;
        if (par.first == "1c") v = 0.01;
        else if (par.first == "2c") v = 0.02;
        else if (par.first == "5c") v = 0.05;
        else if (par.first == "10c") v = 0.10;
        else if (par.first == "20c") v = 0.20;
        else if (par.first == "50c") v = 0.50;
        else if (par.first == "1E") v = 1.0;
        else if (par.first == "2E") v = 2.0;

        // Soma o valor multiplicado pelo número de moedas desse tipo
        valor += v * par.second;
        total += par.second;
    }

    // Espaço antes do total
    y += 10;

    // Mostra o total de moedas
    std::string str = "Total: " + std::to_string(total) + " Moeda(s)";
    cv::putText(frame, str, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 5);
    cv::putText(frame, str, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);

    // Mostra o valor total acumulado com duas casas decimais
    y += 20;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << valor;
    str = "Valor: " + ss.str() + " E";
    cv::putText(frame, str, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 5);
    cv::putText(frame, str, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
}
