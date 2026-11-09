# Open-Meteo `best_match` sem calibração

O Painel usa a temperatura do Open-Meteo `best_match` sem offset, mesmo com diferenças de 2 a 3 °C observadas em relação ao app de tempo da Apple. Os modelos disponíveis para a cidade configurada divergem cerca de 3 °C entre si, não há evidência de um erro sempre para o mesmo lado que justifique offset, e WeatherKit exigiria conta Apple Developer paga e token JWT no ESP32. A Comparação com ontem usa a mesma fonte nos dois dias, então o erro absoluto pesa menos nela.
