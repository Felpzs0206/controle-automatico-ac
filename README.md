# Sistema de Controle Automático de Ar Condicionado

Este projeto simula um sistema de controle automático de ar condicionado utilizando um microcontrolador Raspberry Pi Pico. O sistema monitora o ambiente para ligar o ar condicionado somente quando necessário, garantindo conforto térmico e economia de energia. O projeto utiliza sensores simulados (botões e joystick), uma matriz de LEDs e um display OLED para fornecer feedback visual.

## Funcionalidades

- **Monitoramento de Presença**: Simulado por um botão, indica se há alguém no ambiente.
- **Monitoramento de Janela**: Simulado por outro botão, detecta se a janela está aberta ou fechada.
- **Monitoramento de Temperatura**: Simulado pelo eixo Y de um joystick, retorna a temperatura do ambiente.
- **Feedback Visual**: Utiliza uma matriz de LEDs e um display OLED para exibir o estado dos sensores e mensagens de alerta.

## Componentes

- **Raspberry Pi Pico**: Microcontrolador principal.
- **Botões**: Simulam os sensores de presença e janela.
- **Joystick**: Simula o sensor de temperatura.
- **Matriz de LEDs**: Fornece feedback visual sobre o estado dos sensores.
- **Display OLED**: Exibe a temperatura atual e mensagens de alerta.

## Lógica de Funcionamento

O sistema toma decisões com base nas seguintes condições:

- **Presença**: Se há alguém no ambiente.
- **Janela**: Se a janela está aberta ou fechada.
- **Temperatura**: Se a temperatura está alta ou baixa.

Com base nessas condições, o sistema decide se o ar condicionado deve ser ligado ou não, evitando desperdício de energia.

### Exemplos de Cenários

- **Temperatura Alta e Janela Fechada**: O display sugere "Feche a janela".
- **Temperatura Baixa e Janela Aberta**: O display sugere "Abra a janela".
- **Temperatura Alta, Janela Fechada e Sem Presença**: O LED azul pisca como alerta, indicando que o ar condicionado não deve ser ligado.

## Como Utilizar

1. **Conectar os Componentes**: Conecte os botões, joystick, matriz de LEDs e display OLED ao Raspberry Pi Pico conforme o esquema de ligação.
2. **Carregar o Código**: Carregue o código fornecido no Raspberry Pi Pico.
3. **Interagir com o Sistema**: Pressione os botões para simular a presença e o estado da janela, e gire o joystick para simular a temperatura.

## Estrutura do Código

O código está organizado da seguinte forma:

- **Inicialização**: Configuração dos pinos, inicialização do ADC, I2C, matriz de LEDs e display OLED.
- **Interrupções**: Funções de interrupção para os botões.
- **Atualização da Matriz de LEDs**: Função para atualizar a matriz de LEDs com base no estado dos sensores.
- **Loop Principal**: Leitura contínua dos sensores e atualização do display e matriz de LEDs.


