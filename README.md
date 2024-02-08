# Timberman

## Sobre
Timberman é um jogo onde o jogador assume o papel de um lenhador que precisa coletar madeiras e entregá-las a um NPC para prosseguir no jogo.

O jogo segue uma temática low-poly. Os modelos utilizados são gratuitos e foram obtidos em: [cgtrader](https://www.cgtrader.com/) e [sketchfab](https://sketchfab.com/)

Este projeto foi desenvolvido como trabalho final da disciplina de Fundamentos de Computação Gráfica da CIC/UFRGS.

![image](https://github.com/ThiagoSzz/Timberman-Game/assets/49589136/37fd4afa-8ee8-45d8-8bb5-7ae68bee6f27)

![image](https://github.com/ThiagoSzz/Timberman-Game/assets/49589136/cd868b83-dce6-4456-8d2c-b2acc954f477)


## Demonstração
[Clique aqui para assistir a demo](https://www.youtube.com/watch?v=KDSOwAVDU2k)

## Inspiração
Timberman foi inspirado por um jogo de celular também chamado [Timberman](https://www.youtube.com/watch?v=y-gX4VSFGk0), que possui uma jogabilidade diferente (é um clicker game).

![image](https://github.com/ThiagoSzz/Timberman-Game/assets/49589136/56b82df6-db46-4c21-8a9e-3c1ad479a53a)

## Como Jogar

Para jogar, utilize as seguintes teclas de movimentação:
- W: Avançar
- A: Mover para a esquerda
- S: Retroceder
- D: Mover para a direita

Pressionar a tecla de controle esquerda (CTRL) enquanto se move aumentará a velocidade do jogador, simulando uma ação de corrida.

Para cortar árvores, aproxime-se de uma árvore e mantenha o botão esquerdo do mouse pressionado por três segundos. A ação de "balançar" o machado só é possível quando o jogador está próximo de uma árvore.

As instruções da história do jogo e outras informações sobre como jogar são fornecidas através de monólogos textuais com o NPC cavaleiro.

## Instalação

Para compilar a aplicação, basta acessar o arquivo “Laboratorio_5.cbp” na raiz do diretório do jogo. Em seguida, deve-se alterar o Build Target do projeto para “Release (CBlocks 17.12 32-bit)”, fazer o build e, por fim, executar.

Alternativamente, pode-se acessar o arquivo “main.exe” dentro da pasta “\bin\Release” na raiz do diretório do jogo, que foi compilado com Target x86 [32 bit].

Caso o método descrito não funcione, delete as pastas “bin” e “obj” e efetue novamente a compilação.


## Funcionalidades
- Objetos virtuais representados através de malhas poligonais complexas
- Transformações geométricas de objetos virtuais
- Implementação de uma câmera look-at
- Implementação de uma câmera câmera livre
- Teste de intersecção (cubo-cubo, cubo-plano e ponto-esfera)
- Objetos com o modelo de iluminação difusa (Lambert) e Blinn-Phong
- Objetos com o modelo de iluminação de Phong
- Objeto virtual com movimentação definida através de uma curva de Bézier cúbica

## Tecnologias
![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![OpenGL](https://img.shields.io/badge/OpenGL-%23FFFFFF.svg?style=for-the-badge&logo=opengl)

## Créditos
- Thiago Haab
