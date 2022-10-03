//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   LABORATÓRIO 5
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <windows.h>
#include <math.h>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>
#include <stb_image.h>

// Headers locais, definidos na pasta "include/"
#include "utils.h"
#include "matrices.h"
#include "collisions.h"

// Definições
#define default_speed      5.50f // Velocidade padrão do jogador
#define sensitivity        0.50f // Sensibilidade do mouse
#define delay_cut_tree     3.0f
#define n_trees            200
#define tree_types         1
#define n_decoration       400
#define decoration_types   4
#define n_rocks            200
#define rock_types         7

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

#define M_PI     3.14159265358979323846

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando modelo \"%s\"... ", filename);

        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        printf("OK.\n");
    }
};


// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename, int mode_id=GL_CLAMP_TO_EDGE); // Função que carrega imagens de textura
void DrawVirtualObject(const char* object_name, int ind_type=0); // Desenha um objeto armazenado em g_VirtualScene
void getAllObjectsInFile(const char* filename);
glm::vec4 GetUserInput(GLFWwindow* window, float x, float y, float z);
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(ObjModel*); // Função para debugging
glm::vec3 getBezierCurve(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, float t);
glm::vec3 get3DBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t);

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowProjection(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = true;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = -12.63; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi =  0.06;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 2.0f; // Distância da câmera para a origem

float r, x, y, z;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint vertex_shader_id;
GLuint fragment_shader_id;
GLuint program_id = 0;
GLint model_uniform;
GLint view_uniform;
GLint projection_uniform;
GLint object_id_uniform;
GLint bbox_min_uniform;
GLint bbox_max_uniform;

// Número de texturas carregadas pela função LoadTextureImage()
GLuint g_NumLoadedTextures = 0;

float player_speed = default_speed;

float dt = 0;
float dt1 = 0;
float dt0 = 0;

float x1 = 3.77f;
float y1 = 2.50f;
float z1 = -26.03f;

float prev_x1 = x1;
float prev_y1 = y1;
float prev_z1 = z1;

float axe_angle = 0.0f;
float timer = 0.0f;
bool can_chop = false;
int choppable = 999;
int broken_trees = 0;
bool broke_tree[n_trees];
glm::vec2 trunk_pos[n_trees];
char delay_left[30] = "";

bool collided_with_npc = false;
bool accepted_quest = false;
bool start_game = false;

char obj_names[100][50]={};
int sizeObjModels = 0;

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    GLFWwindow* window = glfwCreateWindow(800, 600, "Timberman", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Referência: https://stackoverflow.com/questions/44321902/glfw-setwindowicon
    GLFWimage images[1];
    images[0].pixels = stbi_load("../../data/textures/icon.png", &images[0].width, &images[0].height, 0, 4); //rgba channels
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);

    // Definimos a função de callback que será chamada sempre que o usuário
    // movimentar o cursor do mouse...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    // Desabilita o ícone do cursor e mantém ele na janela caso saia
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    LoadShadersFromFiles();

    // Carrega a textura
    LoadTextureImage("../../data/textures/texture_gradient.png");
    LoadTextureImage("../../data/textures/low_poly_stones_color_palette.png");
    LoadTextureImage("../../data/textures/axe.png");
    LoadTextureImage("../../data/textures/bigtree.png");
    LoadTextureImage("../../data/textures/character.png");
    LoadTextureImage("../../data/textures/capa.png");

    const char* filename = "../../data/forest_nature_set_all_in.obj";

    // Carrega o arquivo dos objetos
    ObjModel scene(filename);
    ComputeNormals(&scene);
    BuildTrianglesAndAddToVirtualScene(&scene);

    const char* filename1[rock_types] = {"../../data/stone_1.obj",
                                         "../../data/stone_with_moss_2.obj",
                                         "../../data/stone_3.obj",
                                         "../../data/stone_4.obj",
                                         "../../data/stone_with_moss_5.obj",
                                         "../../data/stone_6.obj",
                                         "../../data/stone_7.obj"};

    for(int i=0; i<rock_types; i++){
        // Carrega o arquivo dos objetos
        ObjModel mountain(filename1[i]);
        ComputeNormals(&mountain);
        BuildTrianglesAndAddToVirtualScene(&mountain);

        getAllObjectsInFile(filename1[i]);
    }

    const char* filename2 = "../../data/character.obj";

    // Carrega o arquivo dos objetos
    ObjModel character(filename2);
    ComputeNormals(&character);
    BuildTrianglesAndAddToVirtualScene(&character);

    const char* filename3 = "../../data/axe.obj";

    // Carrega o arquivo dos objetos
    ObjModel axe(filename3);
    ComputeNormals(&axe);
    BuildTrianglesAndAddToVirtualScene(&axe);

    const char* filename4 = "../../data/bigtree.obj";

    // Carrega o arquivo dos objetos
    ObjModel bigtree(filename4);
    ComputeNormals(&bigtree);
    BuildTrianglesAndAddToVirtualScene(&bigtree);

    const char* filename5 = "../../data/littlechicks.obj";

    // Carrega o arquivo dos objetos
    ObjModel chicks(filename5);
    ComputeNormals(&chicks);
    BuildTrianglesAndAddToVirtualScene(&chicks);

    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    const char* tree_names[tree_types] = {"Tree_Spruce_small_01_Cylinder.016"};

    const char* decoration_names[decoration_types] = {"Grass_bush_high_01_Plane.002", "Grass_bush_low_01_Plane.005",
                                                      "Flower_bush_white_Plane.023",  "Flower_bush_red_Plane.031"};

    glm::vec3 tree_position[n_trees];
    glm::vec3 decoration_position[n_decoration];
    glm::vec3 rock_position[n_rocks];
    glm::vec3 log_position[n_rocks];

    float tree_scale[n_trees];
    float rock_scale[n_rocks];

    float random_x, random_z;

    // Randomiza posições e rotações das árvores
    for(int i=0; i<n_trees; i++){
        do{
            random_x = rand() % 500 - 100;
            random_z = rand() % 500 - 100;
        }while((pow(random_x,2) + pow(random_z,2) <= pow(50,2)) ||
               (pow(random_x,2) + pow(random_z,2) >= pow(150,2)));

        tree_position[i].x = random_x;
        tree_position[i].z = random_z;

        tree_scale[i] = (rand() % 80)/100.0 + 0.5;

        broke_tree[i] = false;
    }

    // Randomiza posições e rotações das árvores
    for(int i=0; i<n_decoration; i++){
        do{
            random_x = rand() % 500 - 250;
            random_z = rand() % 500 - 250;
        }while((pow(random_x,2) + pow(random_z,2) <= pow(40,2)) ||
               (pow(random_x,2) + pow(random_z,2) >= pow(150,2)));

        decoration_position[i].x = random_x;
        decoration_position[i].z = random_z;
    }

    for(int i=0; i<n_rocks; i++){
        do{
            random_x = rand() % 500 - 250;
            random_z = rand() % 500 - 250;
        }while((pow(random_x,2) + pow(random_z,2) <= pow(240,2)) ||
               (pow(random_x,2) + pow(random_z,2) >= pow(280,2)));

        rock_position[i].x = random_x;
        rock_position[i].z = random_z;

        rock_scale[i] = ((rand() % 50)/100.0 + 0.8)*40.0;
    }

    for(int i=0; i<10; i++){
        do{
            random_x = rand() % 500 - 250;
            random_z = rand() % 500 - 250;
        }while((pow(random_x,2) + pow(random_z,2) <= pow(50,2)) ||
               (pow(random_x,2) + pow(random_z,2) >= pow(150,2)));

        log_position[i].x = random_x;
        log_position[i].z = random_z;
    }

    // Inicializando os valores da posição da câmera e do up_vector para a câmera look-at
    glm::vec4 camera_position_c =  glm::vec4(62.26f, 15.0f, -49.71f, 1.0f);
    glm::vec4 camera_view_vector = glm::vec4(x1, 2.5f, z1, 1.0f) - glm::vec4(62.26f, 15.0f, -49.71f, 1.0f);
    glm::vec4 camera_up_vector =   glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    glm::vec2 p0, p1, p2, p3;
    p0 = glm::vec2(-22.66f, -18.40f);
    p1 = glm::vec2(-6.60f, -23.68f);
    p2 = glm::vec2(8.76f, -17.90f);
    p3 = glm::vec2(16.43f, -9.21f);

    glm::vec3 bezier_obj, bezier_camera;

    float t = 0.0;

    int n_spheres = 10;

    glm::vec3 tree_spheres[n_spheres] = {glm::vec3(-9.10, 2.50, -7.91),
                                         glm::vec3(-3.59, 2.50, -7.60),
                                         glm::vec3(1.02, 2.50, -5.68),
                                         glm::vec3(3.75, 2.50, -5.08),
                                         glm::vec3(2.76, 2.50, -1.09),
                                         glm::vec3(5.68, 2.50, 3.07),
                                         glm::vec3(1.81, 2.50, 3.25),
                                         glm::vec3(-5.05, 2.50, 5.33),
                                         glm::vec3(-9.95, 2.50, 6.65),
                                         glm::vec3(-5.37, 2.50, -1.88)};

    bool turn_1 = false;
    int dir = 0;

    int level = 0;
    char const* dialog_text[15] = {"               Ola caro lenhador, o inverno esta se aproximando                ",
                                   "e os moradores de uma vila proxima daqui precisam de madeira para se aquecerem.",
                                   "              Caso deseje nos ajudar a coleta-las, pressione [Y].              ",

                                   "                             Certo, muito obrigado!                            ",
                                   "              Voce pode comecar coletando a madeira de 3 arvores.              ",
                                   "                           Estarei aqui te esperando!                          ",

                                   "                   Nos ficamos muito gratos pela sua ajuda!                    ",
                                   "                Porem, precisaremos de mais um pouco de madeira.               ",
                                   "                  Precisaremos da madeira de mais 5 arvores.                   ",

                                   "               Temos alguns moradores novos na nossa vila, entao               ",
                                   "                   precisaremos de mais madeira. Poderia nos                   ",
                                   "                 ajudar coletando a madeira de mais 10 arvores?                ",

                                   "                        Muito obrigado pela sua ajuda!                         ",
                                   "                  Com certeza as madeiras coletadas por voce                   ",
                                   "                  ajudarao a nos aquecer no proximo inverno.                   "};

    bool camera_type = false;
    char broken[20] = "0";

    // Ficamos em loop, renderizando, até que o usuário feche a janela
    while ((!glfwWindowShouldClose(window))||(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS))
    {
        if((turn_1) && (t >= 1.0)){
            p0 = glm::vec2(-22.66f, -18.40f);
            p1 = glm::vec2(-6.60f, -23.68f);
            p2 = glm::vec2(8.76f, -17.90f);
            p3 = glm::vec2(16.43f, -9.21f);

            if((bezier_obj.x >= 16.43f) && (bezier_obj.z >= -9.21f)){
                turn_1 = false;
                t = 0;
                dir = 0;
            }
        }
        else if((!turn_1) && (t >= 1.0)){
            p0 = glm::vec2(16.43f, -9.21f);
            p1 = glm::vec2(8.76f, -17.90f);
            p2 = glm::vec2(-6.60f, -23.68f);
            p3 = glm::vec2(-22.66f, -18.40f);

            if((bezier_obj.x <= -22.66f) && (bezier_obj.z >= -18.40f)){
                turn_1 = true;
                t = 0;
                dir = 1;
            }
        }

        bezier_obj = getBezierCurve(p0, p1, p2, p3, t);

        glClearColor(0.433, 0.773, 0.984, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_id);

        prev_x1 = x1;
        prev_z1 = z1;

        r = g_CameraDistance;
        y = r*sin(g_CameraPhi);
        z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

        dt1 = glfwGetTime();
        dt = dt1 - dt0;

        //printf("glm::vec2(%.2ff, %.2ff);\n", camera_position_c.x, camera_position_c.z);
        //printf("%.2f %.2f\n", g_CameraTheta, g_CameraPhi);

        if(start_game){
            t += dt*0.1;

            if(!camera_type){
                // Curva de Bezier para a câmera
                bezier_camera = get3DBezierCurve(glm::vec3(62.26f, 15.0f, -49.71f),
                                                 glm::vec3(37.03f, 7.0f,  -40.22f),
                                                 glm::vec3(12.83f, 5.0f,  -34.03f),
                                                 glm::vec3(x1,     2.5f,     z1-1), t);

                // Câmera look-at "cut-scene"
                camera_position_c = glm::vec4(bezier_camera.x, bezier_camera.y, bezier_camera.z, 1.0f);
                camera_view_vector = glm::vec4(x1, 2.5f, z1, 1.0f) - glm::vec4(bezier_camera.x, bezier_camera.y, bezier_camera.z, 1.0f);

                if(t >= 1.0){
                    camera_type = true;

                    g_CameraTheta = -12.63;
                    g_CameraPhi =  0.06;
                }
            }
            else{
                // Câmera livre
                camera_position_c = GetUserInput(window, x, y, z);
                camera_view_vector = glm::vec4(x, -y, z, 0.0f);
            }
        }
        else{
            GetUserInput(window, x, y, z);
        }

        glm::mat4 view = Matrix_Camera_View(camera_position_c,
                                            camera_view_vector,
                                            camera_up_vector);

        glm::mat4 projection;

        float nearplane = -0.1f;  // Posição do "near plane"
        float farplane  = -400.0f; // Posição do "far plane"

        // Projeção Perspectiva.
        float field_of_view = 3.141592 / 3.0f;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);

        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

        // Desenhamos o plano do chão
        model = Matrix_Translate(0.0f,0.0f,0.0f)
              * Matrix_Scale(8.0f,1.0f,8.0f);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, TERRAIN);
        DrawVirtualObject("SimpleGround_Plane.024");

        int current_i = 0;
        int i;
        int amount = int(n_trees/tree_types);

        for(int j=0; j<tree_types; j++){
            for(i=current_i; i<(current_i)+amount; i++){
                if(!broke_tree[i]){
                    model = Matrix_Translate(tree_position[i].x, -0.1f, tree_position[i].z)
                          * Matrix_Rotate_X(sin(2*dt1)*0.005)
                          * Matrix_Scale(tree_scale[i], tree_scale[i], tree_scale[i]);
                    glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(object_id_uniform, TREES);
                    DrawVirtualObject(tree_names[j]);

                    if(pointCubeCollision(camera_position_c,
                                          g_VirtualScene[tree_names[j]],
                                          tree_position[i],
                                          tree_scale[i],
                                          3.5f)){

                        x1 = prev_x1;
                        z1 = prev_z1;
                    }

                    if(pointCubeCollision(camera_position_c,
                                          g_VirtualScene[tree_names[j]],
                                          tree_position[i],
                                          tree_scale[i],
                                          3.2f)){

                        can_chop = true;
                        choppable = i;
                    }
                }
                else{
                    model = Matrix_Translate(trunk_pos[i].x+(21.0f*tree_scale[i]), -0.1f, trunk_pos[i].y)
                          * Matrix_Scale(tree_scale[i], tree_scale[i], tree_scale[i]);
                    glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(object_id_uniform, TREES);
                    DrawVirtualObject("Stump_average_low_Cube.014");
                }
            }

            current_i = i;
        }

        current_i = 0;
        amount = int(n_decoration/decoration_types);

        for(int j=0; j<decoration_types; j++){
            for(i=current_i; i<(current_i)+amount; i++){
                // Desenhamos os objetos
                model = Matrix_Translate(decoration_position[i].x, 0.0f, decoration_position[i].z);

                glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(object_id_uniform, TREES);
                DrawVirtualObject(decoration_names[j]);
            }

            current_i = i;
        }

        current_i = 0;
        amount = int(n_rocks/rock_types);

        for(int j=0; j<sizeObjModels; j++){
            for(i=current_i; i<(current_i)+amount; i++){
                // Desenhamos os objetos
                model = Matrix_Translate(rock_position[i].x, 0.0f, rock_position[i].z)
                      * Matrix_Scale(rock_scale[i], rock_scale[i], rock_scale[i]);
                glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));

                glUniform1i(object_id_uniform, MOUNTAINS);
                DrawVirtualObject(obj_names[j]);

                if(pointSphereCollision(camera_position_c,
                                    glm::vec3(rock_position[i].x, 0.0f, rock_position[i].z),
                                    rock_scale[i]+2.0f)){

                    x1 = prev_x1;
                    z1 = prev_z1;
                }
            }

            current_i = i;
        }

        for(i=0; i<10; i++){
            // Desenhamos os objetos
            model = Matrix_Translate(log_position[i].x, -0.1f, log_position[i].z)
                  * Matrix_Scale(0.8f, 0.8f, 0.8f);

            if(cubeCubeCollision(camera_position_c,
                                 g_VirtualScene["Log_big_regular_Cylinder.015"],
                                 log_position[i],
                                 0.8f)){

                x1 = prev_x1;
                z1 = prev_z1;
            }

            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, TREES);
            DrawVirtualObject("Log_big_regular_Cylinder.015");
        }

        // Desenhamos os objetos
        model = Matrix_Translate(0.0f, 0.0f, 0.0f)
              * Matrix_Scale(2.0f, 2.0f, 2.0f);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, BIGTREE);
        DrawVirtualObject("fattree_Mesh.003");

        for(int i=0; i<n_spheres; i++){
            if(pointSphereCollision(camera_position_c,
                                    tree_spheres[i],
                                    5.0f)){

                x1 = prev_x1;
                z1 = prev_z1;
            }
        }

        // Desenhamos os objetos
        model = Matrix_Translate(bezier_obj.x, 0.1f, bezier_obj.z)
              * Matrix_Rotate_X(sin(8*dt1)*0.05)
              * Matrix_Rotate_Y((dir*180+180)*M_PI/180.0)
              * Matrix_Scale(0.03f, 0.03f, 0.03f);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, CHICKEN_LEG);
        DrawVirtualObject("laranja1");
        glUniform1i(object_id_uniform, CHICKEN_BODY);
        DrawVirtualObject("branco1");
        glUniform1i(object_id_uniform, CHICKEN_EYE);
        DrawVirtualObject("preto1");
        DrawVirtualObject("preto1_1");
        glUniform1i(object_id_uniform, CHICKEN_COMB);
        DrawVirtualObject("vermelho1");

        // Desenhamos os objetos
        model = Matrix_Translate(bezier_obj.x + 2.0f + sin(0.5*dt1)*3, 0.1f, bezier_obj.z + 2.0f + sin(0.5*dt1)*3)
              * Matrix_Rotate_X(sin(8*dt1)*0.05)
              * Matrix_Rotate_Y((dir*180+180)*M_PI/180.0)
              * Matrix_Scale(0.03f, 0.03f, 0.03f);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, CHICKEN_LEG);
        DrawVirtualObject("laranja2");
        glUniform1i(object_id_uniform, CHICKEN_BODY);
        DrawVirtualObject("branco2");
        glUniform1i(object_id_uniform, CHICKEN_EYE);
        DrawVirtualObject("preto2");
        DrawVirtualObject("preto2_1");
        glUniform1i(object_id_uniform, CHICKEN_COMB);
        DrawVirtualObject("vermelho2");

        // Desenhamos os objetos
        model = Matrix_Translate(bezier_obj.x + 0.5f + sin(0.5*dt1)*3, 0.1f, bezier_obj.z + 0.5f + sin(0.5*dt1)*3)
              * Matrix_Rotate_X(sin(8*dt1)*0.05)
              * Matrix_Rotate_Y((dir*180+180)*M_PI/180.0)
              * Matrix_Scale(0.03f, 0.03f, 0.03f);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, CHICKEN_LEG);
        DrawVirtualObject("laranja3");
        glUniform1i(object_id_uniform, CHICKEN_BODY);
        DrawVirtualObject("branco3");
        glUniform1i(object_id_uniform, CHICKEN_EYE);
        DrawVirtualObject("preto3");
        DrawVirtualObject("preto3_1");
        glUniform1i(object_id_uniform, CHICKEN_COMB);
        DrawVirtualObject("vermelho3");

        if(camera_type){
            // Desenhamos os objetos
            model = Matrix_Translate(x1+x*0.1f, y1-0.65f, z1+z*0.1f)
                  * Matrix_Rotate_Y(g_CameraTheta + 90*M_PI/180.0)
                  * Matrix_Rotate_X(-20*M_PI/180.0)
                  * Matrix_Rotate_Z(axe_angle*M_PI/180.0)
                  * Matrix_Scale(0.002f, 0.002f, 0.002f);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, AXE);
            DrawVirtualObject("Cube");
            DrawVirtualObject("Plane");
            DrawVirtualObject("Cube.001");
        }

        // Desenhamos os objetos
        model = Matrix_Translate(-4.0f, 0.0f, -10.0f)
              * Matrix_Rotate_Y(180*M_PI/180.0)
              * Matrix_Scale(6.8f, 6.8f, 6.8f);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, CHARACTER);
        DrawVirtualObject("knight");
        model = Matrix_Translate(-4.0f, 0.0f, -10.02f)
              * Matrix_Rotate_Y(180*M_PI/180.0)
              * Matrix_Rotate_X(sin(2*dt1)*0.005)
              * Matrix_Scale(6.8f, 6.8f, 6.8f);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, CHARACTER_CAPA);
        DrawVirtualObject("capa");

        if(pointSphereCollision(camera_position_c,
                                glm::vec3(3.04f, 2.5f, -10.26f),
                                2.0f)){

            x1 = prev_x1;
            z1 = prev_z1;
        }

        if(pointSphereCollision(camera_position_c,
                                glm::vec3(3.04f, 2.5f, -10.26f),
                                7.7f)){

            TextRendering_PrintString(window, dialog_text[0+(level*3)], -0.565f, -0.65f, 1.2f);
            TextRendering_PrintString(window, dialog_text[1+(level*3)], -0.565f, -0.72f, 1.2f);
            TextRendering_PrintString(window, dialog_text[2+(level*3)], -0.565f, -0.79f, 1.2f);

            if(level == 0 && accepted_quest){
                level = 1;
                broken_trees = 0;
            }
            else if(level == 1 && broken_trees >= 3){
                accepted_quest = false;
                level = 2;
                broken_trees = 0;
            }
            else if(level == 2 && broken_trees >= 5){
                accepted_quest = false;
                level = 3;
                broken_trees = 0;
            }
            else if(level == 3 && broken_trees >= 10){
                accepted_quest = false;
                level = 4;
                broken_trees = 0;
            }
        }

        sprintf(broken,"%d", broken_trees);
        TextRendering_PrintString(window, "Arvores cortadas: ", -0.99f, 0.95f, 1.0f);
        TextRendering_PrintString(window, broken, -0.78f, 0.95f, 1.0f);

        TextRendering_PrintString(window, delay_left, -0.05f, 0.0f, 1.0f);

        if(!camera_type && !start_game){
            TextRendering_PrintString(window, "         Timberman          ", -0.25f, 0.035f, 1.5f);
            TextRendering_PrintString(window, "Pressione [ENTER] para jogar", -0.25f, -0.035f, 1.5f);
        }

        TextRendering_ShowFramesPerSecond(window);
        glfwSwapBuffers(window);
        glfwPollEvents();

        dt0 = dt1;
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

glm::vec3 getBezierCurve(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, float t){

    float x = pow(1-t,3)*p0.x + 3*t*pow(1-t,2)*p1.x + 3*pow(t,2)*(1-t)*p2.x + pow(t,3)*p3.x;
    float z = pow(1-t,3)*p0.y + 3*t*pow(1-t,2)*p1.y + 3*pow(t,2)*(1-t)*p2.y + pow(t,3)*p3.y;

    return glm::vec3(x, 0.0f, z);
}

glm::vec3 get3DBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t){

    float x = pow(1-t,3)*p0.x + 3*t*pow(1-t,2)*p1.x + 3*pow(t,2)*(1-t)*p2.x + pow(t,3)*p3.x;
    float y = pow(1-t,3)*p0.y + 3*t*pow(1-t,2)*p1.y + 3*pow(t,2)*(1-t)*p2.y + pow(t,3)*p3.y;
    float z = pow(1-t,3)*p0.z + 3*t*pow(1-t,2)*p1.z + 3*pow(t,2)*(1-t)*p2.z + pow(t,3)*p3.z;

    return glm::vec3(x, y, z);
}


glm::vec4 GetUserInput(GLFWwindow* window, float x, float y, float z){

    float mov = 0;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        x1 = x1 - (dt*player_speed) * x;
        z1 = z1 - (dt*player_speed) * z;
        mov = 6;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        x1 = x1 + (dt*player_speed) * x;
        z1 = z1 + (dt*player_speed) * z;
        mov = 6;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        x1 = x1 - (dt*player_speed) * z;
        z1 = z1 + (dt*player_speed) * x;
        mov = 6;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        x1 = x1 + (dt*player_speed) * z;
        z1 = z1 - (dt*player_speed) * x;
        mov = 6;
    }

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS){
        start_game = true;
    }

    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS){
        accepted_quest = true;
    }
    else{
        accepted_quest = false;
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }

    sprintf(delay_left, " ");

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && can_chop){
        axe_angle = sin(8*timer)*20;
        timer += dt;

        sprintf(delay_left, "%.1fs", (delay_cut_tree - timer));

        if(timer >= delay_cut_tree){
            broke_tree[choppable] = true;
            trunk_pos[choppable] = glm::vec2(x1+x, z1+z);
            broken_trees++;
            can_chop = false;
        }
    }
    else{
        if(int(axe_angle) != 0){
            axe_angle = sin(8*timer)*20;
            timer += dt;
        }
        else{
            axe_angle = 0;
            timer = 0;
            can_chop = false;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
            player_speed = 7.5f;
            mov = 8;
        }
    }
    else{
        player_speed = default_speed;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    y1 = abs(sin(mov*dt1)*0.2)+2.5;

    return glm::vec4(x1, y1, z1, 1.0f);
}

// Função que carrega uma imagem para ser utilizada como textura
void LoadTextureImage(const char* filename, int mode_id)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, mode_id);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, mode_id);

    // Parâmetros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;
}

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name, int ind_type)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Setamos as variáveis "bbox_min" e "bbox_max" do fragment shader
    // com os parâmetros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(ind_type);
}

void getAllObjectsInFile(const char* filename){

    FILE *f = fopen(filename, "r");

    char aux[30];

    while(fscanf(f, "%s ", aux) != EOF){
        if(strcmp(aux, "o") == 0){
            fscanf(f, "%[^\n]", obj_names[sizeObjModels]);
            //printf("%s\n", obj_names[sizeObjModels]);
            sizeObjModels++;
        }
    }

    fclose(f);

    //printf("Carregou %d objetos.", sizeObjModels-1);
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( program_id != 0 )
        glDeleteProgram(program_id);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    program_id = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    model_uniform           = glGetUniformLocation(program_id, "model"); // Variável da matriz "model"
    view_uniform            = glGetUniformLocation(program_id, "view"); // Variável da matriz "view" em shader_vertex.glsl
    projection_uniform      = glGetUniformLocation(program_id, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    object_id_uniform       = glGetUniformLocation(program_id, "object_id"); // Variável "object_id" em shader_fragment.glsl
    bbox_min_uniform        = glGetUniformLocation(program_id, "bbox_min");
    bbox_max_uniform        = glGetUniformLocation(program_id, "bbox_max");


    // Variáveis em "shader_fragment.glsl" para acesso das imagens de textura
    glUseProgram(program_id);
    glUniform1i(glGetUniformLocation(program_id, "TextureImage0"), 0);
    glUniform1i(glGetUniformLocation(program_id, "TextureImage1"), 1);
    glUniform1i(glGetUniformLocation(program_id, "TextureImage2"), 2);
    glUniform1i(glGetUniformLocation(program_id, "TextureImage3"), 3);
    glUniform1i(glGetUniformLocation(program_id, "TextureImage4"), 4);
    glUniform1i(glGetUniformLocation(program_id, "TextureImage5"), 5);
    glUseProgram(0);
}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice.

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4  vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
            }

            const glm::vec4  a = vertices[0];
            const glm::vec4  b = vertices[1];
            const glm::vec4  c = vertices[2];

            const glm::vec4  n = crossproduct(b-a,c-a);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize( 3*num_vertices );

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3*i + 0] = n.x;
        model->attrib.normals[3*i + 1] = n.y;
        model->attrib.normals[3*i + 2] = n.z;
    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para deleção após serem linkados
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        //Nada por enquanto...
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.

    if (g_LeftMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        // Atualizamos parâmetros da câmera com os deslocamentos
        g_CameraTheta -= 0.01f*dx;
        g_CameraPhi   += 0.01f*dy;

        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f/2;
        float phimin = -phimax;

        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;

        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_RightMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        //float dx = xpos - g_LastCursorPosX;
        //float dy = ypos - g_LastCursorPosY;

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_MiddleMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        //float dx = xpos - g_LastCursorPosX;
        //float dy = ypos - g_LastCursorPosY;

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    g_CameraDistance -= 0.1f*yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
        g_CameraDistance = verysmallnumber;
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // ================
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    // ================

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Se o usuário apertar a tecla P, utilizamos projeção perspectiva.
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = true;
    }

    // Se o usuário apertar a tecla O, utilizamos projeção ortográfica.
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = false;
    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Esta função recebe um vértice com coordenadas de modelo p_model e passa o
// mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
// view, e projection; e escreve na tela as matrizes e pontos resultantes
// dessas transformações.
void TextRendering_ShowModelViewProjection(
    GLFWwindow* window,
    glm::mat4 projection,
    glm::mat4 view,
    glm::mat4 model,
    glm::vec4 p_model
)
{
    if ( !g_ShowInfoText )
        return;

    glm::vec4 p_world = model*p_model;
    glm::vec4 p_camera = view*p_world;
    glm::vec4 p_clip = projection*p_camera;
    glm::vec4 p_ndc = p_clip / p_clip.w;

    float pad = TextRendering_LineHeight(window);

    TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f-pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f-2*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-6*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-7*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-8*pad, 1.0f);

    TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f-9*pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f-10*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-14*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-15*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-16*pad, 1.0f);

    TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f-17*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f-18*pad, 1.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glm::vec2 a = glm::vec2(-1, -1);
    glm::vec2 b = glm::vec2(+1, +1);
    glm::vec2 p = glm::vec2( 0,  0);
    glm::vec2 q = glm::vec2(width, height);

    glm::mat4 viewport_mapping = Matrix(
        (q.x - p.x)/(b.x-a.x), 0.0f, 0.0f, (b.x*p.x - a.x*q.x)/(b.x-a.x),
        0.0f, (q.y - p.y)/(b.y-a.y), 0.0f, (b.y*p.y - a.y*q.y)/(b.y-a.y),
        0.0f , 0.0f , 1.0f , 0.0f ,
        0.0f , 0.0f , 0.0f , 1.0f
    );

    TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f-22*pad, 1.0f);
    TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f-23*pad, 1.0f);
    TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f-24*pad, 1.0f);

    TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f-25*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f-26*pad, 1.0f);
}

// Escrevemos na tela qual matriz de projeção está sendo utilizada.
void TextRendering_ShowProjection(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    if ( g_UsePerspectiveProjection )
        TextRendering_PrintString(window, "Perspective", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
    else
        TextRendering_PrintString(window, "Orthographic", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// Função para debugging: imprime no terminal todas informações de um modelo
// geométrico carregado de um arquivo ".obj".
// Veja: https://github.com/syoyo/tinyobjloader/blob/22883def8db9ef1f3ffb9b404318e7dd25fdbb51/loader_example.cc#L98
void PrintObjModelInfo(ObjModel* model)
{
  const tinyobj::attrib_t                & attrib    = model->attrib;
  const std::vector<tinyobj::shape_t>    & shapes    = model->shapes;
  const std::vector<tinyobj::material_t> & materials = model->materials;

  printf("# of vertices  : %d\n", (int)(attrib.vertices.size() / 3));
  printf("# of normals   : %d\n", (int)(attrib.normals.size() / 3));
  printf("# of texcoords : %d\n", (int)(attrib.texcoords.size() / 2));
  printf("# of shapes    : %d\n", (int)shapes.size());
  printf("# of materials : %d\n", (int)materials.size());

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", materials[i].roughness);
    printf("  material.Pm     = %f\n", materials[i].metallic);
    printf("  material.Ps     = %f\n", materials[i].sheen);
    printf("  material.Pc     = %f\n", materials[i].clearcoat_thickness);
    printf("  material.Pcr    = %f\n", materials[i].clearcoat_thickness);
    printf("  material.aniso  = %f\n", materials[i].anisotropy);
    printf("  material.anisor = %f\n", materials[i].anisotropy_rotation);
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :
