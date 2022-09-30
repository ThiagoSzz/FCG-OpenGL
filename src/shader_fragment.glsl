#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define TERRAIN 0
#define TREES 1
#define MOUNTAINS 2
#define CHARACTER 3
#define CHARACTER_CAPA 4
#define AXE 5
#define BIGTREE 6
#define CHICKEN_LEG 7
#define CHICKEN_BODY 8
#define CHICKEN_EYE 9
#define CHICKEN_COMB 10

uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;
uniform sampler2D TextureImage3;
uniform sampler2D TextureImage4;
uniform sampler2D TextureImage5;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Vetor que define o sentido da reflexão especular ideal.
    vec4 r = -l + 2.0*n*(dot(n,l));

    // Parâmetros que definem as propriedades espectrais da superfície
    vec3 Kd; // Refletância difusa
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

    float px = position_model.x;
    float py = position_model.y;
    float pz = position_model.z;

    if ( object_id == TERRAIN )
    {
        U = texcoords.x;
        V = texcoords.y;

        color.rgb = texture(TextureImage0, vec2(U,V)).rgb;
        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);

        return;
    }
    else if ( object_id == TREES )
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ
        U = texcoords.x;
        V = texcoords.y;

        // Propriedades espectrais
        Kd = texture(TextureImage0, vec2(U,V)).rgb;
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2.0;
        q  = 1.0;
    }
    else if ( object_id == MOUNTAINS )
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ
        U = texcoords.x;
        V = texcoords.y;

        // Propriedades espectrais
        Kd = texture(TextureImage1, vec2(U,V)).rgb;
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2.0;
        q  = 1.0;
    }
    else if ( object_id == CHARACTER )
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ
        U = texcoords.x;
        V = texcoords.y;

        // Propriedades espectrais
        Kd = texture(TextureImage4, vec2(U,V)).rgb;
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2.0;
        q  = 1.0;
    }
    else if ( object_id == AXE )
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ
        float minx = bbox_min.x;
        float maxx = bbox_max.x;

        float miny = bbox_min.y;
        float maxy = bbox_max.y;

        float minz = bbox_min.z;
        float maxz = bbox_max.z;

        U = (px-minx)/(maxx-minx);
        V = (pz-minx)/(maxy-miny);

        Kd = texture(TextureImage2, vec2(U,V)).rgb;
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/3.0;
        q  = 1.0;
    }
    else if ( object_id == BIGTREE )
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ
        U = texcoords.x;
        V = texcoords.y;

        // Propriedades espectrais
        Kd = texture(TextureImage3, vec2(U,V)).rgb;
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2.0;
        q  = 1.0;
    }
    else if ( object_id == CHARACTER_CAPA )
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ
        U = texcoords.x;
        V = texcoords.y;

        // Propriedades espectrais
        Kd = texture(TextureImage5, vec2(U,V)).rgb;
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2.0;
        q  = 1.0;
    }
    else if ( object_id == CHICKEN_LEG )
    {
        U = texcoords.x;
        V = texcoords.y;

        Kd = vec3(0.996,0.402,0.0);
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2.0;
        q  = 1.0;
    }
    else if ( object_id == CHICKEN_BODY )
    {
        U = texcoords.x;
        V = texcoords.y;

        Kd = vec3(0.976, 0.972, 0.960);
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/5.0;
        q  = 1.0;
    }
    else if ( object_id == CHICKEN_EYE )
    {
        U = texcoords.x;
        V = texcoords.y;

        Kd = vec3(0.0, 0.0, 0.0);
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2.0;
        q  = 1.0;
    }
    else if ( object_id == CHICKEN_COMB )
    {
        U = texcoords.x;
        V = texcoords.y;

        Kd = vec3(0.996, 0.113, 0.093);
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2.0;
        q  = 1.0;
    }
    else
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ
        U = texcoords.x;
        V = texcoords.y;

        // Propriedades espectrais
        Kd = vec3(0.0,0.0,0.0);
        Ks = vec3(0.0,0.0,0.0);
        Ka = vec3(0.0,0.0,0.0);
        q  = 1.0;
    }

    // Espectro da fonte de iluminação
    vec3 I = vec3(1.0,1.0,1.0);

    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2,0.2,0.2);

    // Termo difuso utilizando a lei dos cossenos de Lambert
    vec3 lambert_diffuse_term = Kd*I*max(0.0,dot(n,l));

    // Termo ambiente
    vec3 ambient_term = Ka*Ia;

    // Termo especular utilizando o modelo de iluminação de Phong
    vec3 phong_specular_term  = Ks*I*pow(max(0.0,dot(r,v)), q);

    // NOTE: Se você quiser fazer o rendering de objetos transparentes, é
    // necessário:
    // 1) Habilitar a operação de "blending" de OpenGL logo antes de realizar o
    //    desenho dos objetos transparentes, com os comandos abaixo no código C++:
    //      glEnable(GL_BLEND);
    //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 2) Realizar o desenho de todos objetos transparentes *após* ter desenhado
    //    todos os objetos opacos; e
    // 3) Realizar o desenho de objetos transparentes ordenados de acordo com
    //    suas distâncias para a câmera (desenhando primeiro objetos
    //    transparentes que estão mais longe da câmera).
    // Alpha default = 1 = 100% opaco = 0% transparente
    color.a = 1;

    // Cor final do fragmento calculada com uma combinação dos termos difuso,
    // especular, e ambiente. Veja slide 129 do documento Aula_17_e_18_Modelos_de_Iluminacao.pdf.
    color.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
}

