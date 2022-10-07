//      Universidade Federal do Rio Grande do Sul
//              Instituto de Informática
//        Departamento de Informática Aplicada
//
//     INF01047 Fundamentos de Computação Gráfica
//                Prof. Eduardo Gastal
//
// TRABALHO FINAL - FUNDAMENTOS DE COMPUTAÇÃO GRÁFICA

// Bibliotecas e Headers
#include <cmath>
#include <cstdio>
#include <cstdlib>

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

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tiny_obj_loader.h>
#include <stb_image.h>

#include "utils.h"
#include "matrices.h"
#include "collisions.h"

// Definições
#define default_speed      5.50f // Velocidade padrão do jogador
#define sensitivity        0.50f // Sensibilidade do mouse
#define delay_cut_tree     3.0f  // Delay para cortar a árvore
#define n_trees            250   // Número de árvores geradas no mapa
#define tree_types         1     // Tipos de árvore (objetos lidos)
#define n_decoration       400   // Número de decorações (plantas) geradas no mapa
#define decoration_types   4     // Tipos de decorações (objetos lidos)
#define n_rocks            200   // Número de pedras para compor a montanha
#define rock_types         7     // Tipos de pedras (objetos lidos)

// IDs das Texturas
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

// Outras definições
#define M_PI 3.14159265358979323846

// Estrutura que representa um modelo geométrico carregado a partir de um arquivo .obj
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

// Definição das funções
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename, int mode_id=GL_CLAMP_TO_EDGE); // Função que carrega imagens de textura
void DrawVirtualObject(const char* object_name, int ind_type=0); // Desenha um objeto armazenado em g_VirtualScene
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU

glm::vec4 getUserInput(GLFWwindow* window, float x, float y, float z);
void getAllObjectsInFile(const char* filename);
glm::vec3 get2DBezierCurve(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, float t);
glm::vec3 get3DBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t);

void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);

// Variáveis globais
std::map<std::string, SceneObject> g_VirtualScene; // Cena virtual (dicionário).
std::stack<glm::mat4>  g_MatrixStack; // Pilha que guardará as matrizes de modelagem.

float g_ScreenRatio = 1.0f; // Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().

bool g_LeftMouseButtonPressed = true;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas
float g_CameraTheta = -12.63;  // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi =  0.06;     // Ângulo em relação ao eixo Y
float g_CameraDistance = 2.0f; // Distância da câmera para a origem
float r, x, y, z;

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

GLuint g_NumLoadedTextures = 0; // Número de texturas carregadas pela função LoadTextureImage()

float player_speed = default_speed;

// Animação baseada no tempo
float dt = 0;
float dt1 = 0;
float dt0 = 0;

// Posição do jogador
float x1 = 3.77f;
float y1 = 2.50f;
float z1 = -26.03f;

// Posição do jogador em caso de colisão
float prev_x1 = x1;
float prev_y1 = y1;
float prev_z1 = z1;

// Árvores e machado
float axe_angle = 0.0f;       // Rotação do machado de acordo com a animação
float timer = 0.0f;
bool can_chop = false;        // Variável alterada quando há colisão com uma árvore
int choppable = 999;          // Índice da árvore sendo cortada
int broken_trees = 0;
bool broke_tree[n_trees];     // Estado da árvore (se cortada ou não)
glm::vec2 trunk_pos[n_trees]; // Posição do tronco da árvore cortada
char delay_left[30] = "";

// NPC e início do jogo
bool collided_with_npc = false;
bool accepted_quest = false;
bool start_game = false;

// Leitura dos objetos de um .obj
char obj_names[100][50]={};
int sizeObjModels = 0;

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW
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

    // Muda o ícone da janela
    // FONTE: https://stackoverflow.com/questions/44321902/glfw-setwindowicon
    GLFWimage images[1];
    images[0].pixels = stbi_load("../../data/textures/icon.png", &images[0].width, &images[0].height, 0, 4); //rgba channels
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);

    // Definimos a função de callback que será chamada sempre que o usuário
    // movimentar o cursor do mouse...
    glfwSetCursorPosCallback(window, CursorPosCallback);

    // Desabilita o ícone do cursor e mantém ele na janela caso saia
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for redimensionada
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    LoadShadersFromFiles();

    // Carrega as texturas
    LoadTextureImage("../../data/textures/texture_gradient.png");
    LoadTextureImage("../../data/textures/low_poly_stones_color_palette.png");
    LoadTextureImage("../../data/textures/axe.png"); // Textura criada pelo grupo
    LoadTextureImage("../../data/textures/bigtree.png");
    LoadTextureImage("../../data/textures/character.png"); // Textura criada pelo grupo
    LoadTextureImage("../../data/textures/capa.png"); // Textura criada pelo grupo

    // Carrega modelo com o set das árvores, troncos e plano do chão
    ObjModel scene("../../data/forest_nature_set_all_in.obj");
    ComputeNormals(&scene);
    BuildTrianglesAndAddToVirtualScene(&scene);

    // Carrega os modelos das pedras da montanha
    const char* filename[rock_types] = {"../../data/stone_1.obj",
                                         "../../data/stone_with_moss_2.obj",
                                         "../../data/stone_3.obj",
                                         "../../data/stone_4.obj",
                                         "../../data/stone_with_moss_5.obj",
                                         "../../data/stone_6.obj",
                                         "../../data/stone_7.obj"};
    for(int i=0; i<rock_types; i++){
        ObjModel mountain(filename[i]);
        ComputeNormals(&mountain);
        BuildTrianglesAndAddToVirtualScene(&mountain);

        getAllObjectsInFile(filename[i]);
    }

    // Carrega o modelo do NPC (cavaleiro)
    ObjModel character("../../data/character.obj");
    ComputeNormals(&character);
    BuildTrianglesAndAddToVirtualScene(&character);

    // Carrega o modelo do machado
    ObjModel axe("../../data/axe.obj");
    ComputeNormals(&axe);
    BuildTrianglesAndAddToVirtualScene(&axe);

    // Carrega o modelo da árvore gigante
    ObjModel bigtree("../../data/bigtree.obj");
    ComputeNormals(&bigtree);
    BuildTrianglesAndAddToVirtualScene(&bigtree);

    // Carrega o modelo das galinhas
    ObjModel chicks("../../data/littlechicks.obj");
    ComputeNormals(&chicks);
    BuildTrianglesAndAddToVirtualScene(&chicks);

    TextRendering_Init(); // Inicializamos o código para renderização de texto.

    glEnable(GL_DEPTH_TEST); // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.

    // Habilitamos o Backface Culling. Veja slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Nomes dos objetos das árvores
    const char* tree_names[tree_types] = {"Tree_Spruce_small_01_Cylinder.016"};

    // Nomes dos objetos das decorações
    const char* decoration_names[decoration_types] = {"Grass_bush_high_01_Plane.002", "Grass_bush_low_01_Plane.005",
                                                      "Flower_bush_white_Plane.023",  "Flower_bush_red_Plane.031"};

    // Vetores de posição das árvores, decorações, pedras e troncos
    glm::vec3 tree_position[n_trees];
    glm::vec3 decoration_position[n_decoration];
    glm::vec3 rock_position[n_rocks];
    glm::vec3 log_position[n_rocks];

    // Vetores de escaladas árvores e pedras
    float tree_scale[n_trees];
    float rock_scale[n_rocks];

    // Randomização
    float random_x, random_z;

    // Randomiza posições e escala das árvores
    // Obs.: não verifica se estão muito perto ou dentro de outras
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

    // Randomiza posições das decorações
    // Obs.: não verifica se estão muito perto ou dentro de outras
    for(int i=0; i<n_decoration; i++){
        do{
            random_x = rand() % 500 - 250;
            random_z = rand() % 500 - 250;
        }while((pow(random_x,2) + pow(random_z,2) <= pow(40,2)) ||
               (pow(random_x,2) + pow(random_z,2) >= pow(150,2)));

        decoration_position[i].x = random_x;
        decoration_position[i].z = random_z;
    }

    // Randomiza posições e escala das pedras
    // Obs.: não verifica se estão muito perto ou dentro de outras
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

    // Randomiza posições dos troncos
    // Obs.: não verifica se estão muito perto ou dentro de outros
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
    glm::vec4 camera_position_c =  glm::vec4(62.26f, 15.0f, -49.71f, 1.0f); // Início da curva de bezier da câmera look-at
    glm::vec4 camera_view_vector = glm::vec4(x1, 2.5f, z1, 1.0f) - glm::vec4(62.26f, 15.0f, -49.71f, 1.0f);
    glm::vec4 camera_up_vector =   glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    // Definição das esferas da colisão com a árvore
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

    // Curvas de Bezier
    glm::vec2 p0, p1, p2, p3; // Pontos de controle da curva das galinhas (4 pontos = cúbica)
    p0 = glm::vec2(-22.66f, -18.40f);
    p1 = glm::vec2(-6.60f, -23.68f);
    p2 = glm::vec2(8.76f, -17.90f);
    p3 = glm::vec2(16.43f, -9.21f);
    glm::vec3 bezier_obj, bezier_camera; // Posição na Curva de Bezier das galinhas e da câmera
    float t = 0.0; // Parâmetro "t" do cálculo da curva de bezier
    bool turn_1 = false;
    int dir = 0;

    int level = 0; // Nível do jogo (ao total são 3)

    // Texto do monólogo com o NPC
    char const* monolog_text[15] = {"Ola caro lenhador, o inverno esta se aproximando",
                                    "e os moradores de uma vila proxima daqui precisam de madeira para se aquecerem.",
                                    "Caso deseje nos ajudar a coleta-las, pressione [Y].",

                                    "Certo, muito obrigado!",
                                    "Voce pode comecar coletando a madeira de 3 arvores.",
                                    "Estarei aqui te esperando!",

                                    "Nos ficamos muito gratos pela sua ajuda!",
                                    "Porem, precisaremos de mais um pouco de madeira.",
                                    "Precisaremos da madeira de mais 5 arvores.",

                                    "Temos alguns moradores novos na nossa vila, entao",
                                    "precisaremos de mais madeira. Poderia nos",
                                    "ajudar coletando a madeira de mais 10 arvores?",

                                    "Muito obrigado pela sua ajuda!",
                                    "Com certeza as madeiras coletadas por voce",
                                    "ajudarao a nos aquecer no proximo inverno."};

    bool camera_type = false; // Altera câmera entre look-at e livre

    char broken[20] = "0"; // Display do contador de árvores quebradas

    // Ficamos em loop, renderizando, até que o usuário feche a janela
    while ((!glfwWindowShouldClose(window))||(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS))
    {
        float lineheight = TextRendering_LineHeight(window);
        float charwidth = TextRendering_CharWidth(window);

        // Curva de Bezier das galinhas
        // Quando atingir o final da curva, altera os pontos de controle para fazer o caminho contrário
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

        bezier_obj = get2DBezierCurve(p0, p1, p2, p3, t);

        glClearColor(0.433, 0.773, 0.984, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_id);

        // Atualiza o "passo" anterior do jogador, para caso haja colisão
        prev_x1 = x1;
        prev_z1 = z1;

        // Câmera
        r = g_CameraDistance;
        y = r*sin(g_CameraPhi);
        z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

        // Animação baseada no tempo
        dt1 = glfwGetTime();
        dt = dt1 - dt0;

        // Aguarda o usuário iniciar o jogo pressionando ENTER
        if(start_game){
            t += dt*0.1;

            if(!camera_type){
                // Câmera look-at da "cut-scene"
                // Curva de Bezier para a câmera
                bezier_camera = get3DBezierCurve(glm::vec3(62.26f, 15.0f, -49.71f),
                                                 glm::vec3(37.03f, 7.0f,  -40.22f),
                                                 glm::vec3(12.83f, 5.0f,  -34.03f),
                                                 glm::vec3(x1,     2.5f,     z1-1), t);

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
                camera_position_c = getUserInput(window, x, y, z);
                camera_view_vector = glm::vec4(x, -y, z, 0.0f);
            }
        }
        else{
            getUserInput(window, x, y, z);
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

        // Desenha o plano do chão
        model = Matrix_Translate(0.0f,0.0f,0.0f)
              * Matrix_Scale(8.0f,1.0f,8.0f);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, TERRAIN);
        DrawVirtualObject("SimpleGround_Plane.024");

        // Desenha as árvores de acordo com os vetores de posição e escala randomizados
        int current_i = 0;
        int i;
        int amount = int(n_trees/tree_types);

        for(int j=0; j<tree_types; j++){
            for(i=current_i; i<(current_i)+amount; i++){
                if(!broke_tree[i]){
                    // Se a árvore não foi cortada ainda
                    // Desenha a árvore
                    model = Matrix_Translate(tree_position[i].x, -0.1f, tree_position[i].z)
                          * Matrix_Rotate_X(sin(2*dt1)*0.005) // Simula vento batendo nas árvores
                          * Matrix_Scale(tree_scale[i], tree_scale[i], tree_scale[i]);
                    glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(object_id_uniform, TREES);
                    DrawVirtualObject(tree_names[j]);

                    // Colisão ponto-cubo entre câmera (jogador) e árvores
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
                    // Se a árvore já foi cortada
                    // Desenha o tronco cortado
                    model = Matrix_Translate(trunk_pos[i].x+(21.0f*tree_scale[i]), -0.1f, trunk_pos[i].y)
                          * Matrix_Scale(tree_scale[i], tree_scale[i], tree_scale[i]);
                    glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(object_id_uniform, TREES);
                    DrawVirtualObject("Stump_average_low_Cube.014");
                }
            }

            current_i = i;
        }

        // Desenha as decorações de acordo com os vetores de posição e escala randomizados
        current_i = 0;
        amount = int(n_decoration/decoration_types);

        for(int j=0; j<decoration_types; j++){
            for(i=current_i; i<(current_i)+amount; i++){
                model = Matrix_Translate(decoration_position[i].x, 0.0f, decoration_position[i].z);

                glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(object_id_uniform, TREES);
                DrawVirtualObject(decoration_names[j]);
            }

            current_i = i;
        }

        // Desenha as pedras de acordo com os vetores de posição e escala randomizados
        current_i = 0;
        amount = int(n_rocks/rock_types);

        for(int j=0; j<sizeObjModels; j++){
            for(i=current_i; i<(current_i)+amount; i++){
                model = Matrix_Translate(rock_position[i].x, 0.0f, rock_position[i].z)
                      * Matrix_Scale(rock_scale[i], rock_scale[i], rock_scale[i]);
                glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));

                glUniform1i(object_id_uniform, MOUNTAINS);
                DrawVirtualObject(obj_names[j]);

                // Colisão ponto-esfera entre câmera (jogador) e pedras da montanha
                if(pointSphereCollision(camera_position_c,
                                    glm::vec3(rock_position[i].x, 0.0f, rock_position[i].z),
                                    rock_scale[i]+2.0f)){

                    x1 = prev_x1;
                    z1 = prev_z1;
                }
            }

            current_i = i;
        }

        // Desenha os troncos de acordo com os vetores de posição e escala randomizados
        for(i=0; i<10; i++){
            model = Matrix_Translate(log_position[i].x, -0.1f, log_position[i].z)
                  * Matrix_Scale(0.8f, 0.8f, 0.8f);

            // Colisão cubo-cubo entre câmera (jogador) e tronco
            // O cubo do jogador é definido a partir da soma/subtração de uma constante
            // em relação ao ponto da câmera
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

        // Desenha a árvore gigante do meio do mapa
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

        // Desenha a galinha maior
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

        // Desenha uma galinha menor
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

        // Desenha a outra galinha menor
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

        // Desenha o machado apenas caso o jogo já tenha começado
        // Ou seja, quando está na câmera livre
        if(camera_type){
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

        // Desenha o NPC (cavaleiro)
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

        // Colisão ponto-esfera entre câmera (jogador) e NPC
        if(pointSphereCollision(camera_position_c,
                                glm::vec3(3.04f, 2.5f, -10.26f),
                                2.0f)){

            x1 = prev_x1;
            z1 = prev_z1;
        }

        // Outra colisão para verificar se o jogador está próximo o suficiente
        // para mostrar as mensagens relativas a Quest
        if(pointSphereCollision(camera_position_c,
                                glm::vec3(3.04f, 2.5f, -10.26f),
                                7.7f)){

            // Mensagem da Quest
            TextRendering_PrintString(window, monolog_text[0+(level*3)], 0.0f-strlen(monolog_text[0+(level*3)])*charwidth*1.2f/2, -1.0f+0.05f+0.24f-lineheight*1.2f, 1.2f);
            TextRendering_PrintString(window, monolog_text[1+(level*3)], 0.0f-strlen(monolog_text[1+(level*3)])*charwidth*1.2f/2, -1.0f+0.05f+0.16f-lineheight*1.2f, 1.2f);
            TextRendering_PrintString(window, monolog_text[2+(level*3)], 0.0f-strlen(monolog_text[2+(level*3)])*charwidth*1.2f/2, -1.0f+0.05f+0.08f-lineheight*1.2f, 1.2f);

            // Progressão de nível
            if(level == 0 && accepted_quest){
                level = 1;
                broken_trees = 0;
            }
            else if(level == 1 && broken_trees >= 3){
                level = 2;
                broken_trees = 0;
            }
            else if(level == 2 && broken_trees >= 5){
                level = 3;
                broken_trees = 0;
            }
            else if(level == 3 && broken_trees >= 10){
                level = 4;
                broken_trees = 0;
            }
        }

        // Quest
        if(level == 1){
            TextRendering_PrintString(window, "Quest: cortar 3 arvores", -0.99f, 1.0f-lineheight-0.06f, 1.0f);
        }
        else if(level == 2){
            TextRendering_PrintString(window, "Quest: cortar 5 arvores", -0.99f, 1.0f-lineheight-0.06f, 1.0f);
        }
        else if(level == 3){
            TextRendering_PrintString(window, "Quest: cortar 10 arvores", -0.99f, 1.0f-lineheight-0.06f, 1.0f);
        }

        // Número de árvores cortadas durante a Quest
        sprintf(broken,"%d", broken_trees);
        TextRendering_PrintString(window, "Arvores cortadas: ", -0.99f, 1.0f-lineheight, 1.0f);
        TextRendering_PrintString(window, broken, -0.99f+strlen("Arvores cortadas: ")*charwidth*1.0f, 1.0f-lineheight-0.0025f, 1.0f);

        // Delay de quebrar a árvore
        TextRendering_PrintString(window, delay_left, strlen(delay_left)*charwidth*1.0f/2, 0.0f, 1.0f);

        // Enquanto o jogo não começar, mostra uma tela inicial
        if(!camera_type && !start_game){
            TextRendering_PrintString(window, "Timberman", 0.0f-strlen("Timberman")*charwidth*1.5f*(abs(sin(dt1))+0.3f)/2, 0.0f+0.01f+lineheight, 1.5f*(abs(sin(dt1))+0.3f));
            TextRendering_PrintString(window, "Pressione [ENTER] para jogar", 0.0f-strlen("Pressione [ENTER] para jogar")*charwidth*1.5f*(abs(sin(dt1))+0.3f)/2, 0.0f-0.01f-lineheight, 1.5f*(abs(sin(dt1))+0.3f));
        }

        // FPS
        TextRendering_ShowFramesPerSecond(window);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // Atualiza o dt0 com o novo "passo" do glfwGetTime()
        dt0 = dt1;
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

// Cálculo da Curva de Bezier 2D
glm::vec3 get2DBezierCurve(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, float t){

    float x = pow(1-t,3)*p0.x + 3*t*pow(1-t,2)*p1.x + 3*pow(t,2)*(1-t)*p2.x + pow(t,3)*p3.x;
    float z = pow(1-t,3)*p0.y + 3*t*pow(1-t,2)*p1.y + 3*pow(t,2)*(1-t)*p2.y + pow(t,3)*p3.y;

    return glm::vec3(x, 0.0f, z);
}

// Cálculo da Curva de Bezier 3D
glm::vec3 get3DBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t){

    float x = pow(1-t,3)*p0.x + 3*t*pow(1-t,2)*p1.x + 3*pow(t,2)*(1-t)*p2.x + pow(t,3)*p3.x;
    float y = pow(1-t,3)*p0.y + 3*t*pow(1-t,2)*p1.y + 3*pow(t,2)*(1-t)*p2.y + pow(t,3)*p3.y;
    float z = pow(1-t,3)*p0.z + 3*t*pow(1-t,2)*p1.z + 3*pow(t,2)*(1-t)*p2.z + pow(t,3)*p3.z;

    return glm::vec3(x, y, z);
}

// Input do usuário e rotações da câmera e de alguns objetos
glm::vec4 getUserInput(GLFWwindow* window, float x, float y, float z){

    // "Velocidade" da função seno
    // Usada para calcular o efeito de "Bobbing" da câmera
    // Enquanto o jogador encontra-se parado, é zero
    float mov = 0;

    // Movimento para trás
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        x1 = x1 - (dt*player_speed) * x;
        z1 = z1 - (dt*player_speed) * z;
        mov = 6;
    }

    // Movimento para frente
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        x1 = x1 + (dt*player_speed) * x;
        z1 = z1 + (dt*player_speed) * z;
        mov = 6;
    }

    // Movimento para direita
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        x1 = x1 - (dt*player_speed) * z;
        z1 = z1 + (dt*player_speed) * x;
        mov = 6;
    }

    // Movimento para esquerda
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        x1 = x1 + (dt*player_speed) * z;
        z1 = z1 - (dt*player_speed) * x;
        mov = 6;
    }

    // Início do jogo
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS){
        start_game = true;
    }

    // Aceitar Quest
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS){
        accepted_quest = true;
    }

    // Recarregar os shaders em tempo de execução
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }

    // Animação de cortar a árvore se estiver com o botão esquerdo pressionado
    // e se estiver colidindo com a árvore
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
        // Animação de volta do machado para a posição inicial
        // Para o machado transicionar suavemente entre a posição "usando" e "parado"
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

    // Movimentação de corrida (aumenta a velocidade)
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
            player_speed = 7.5f;
            mov = 8;
        }
    }
    else{
        player_speed = default_speed;
    }

    // Fechar a tela
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Bobbing da câmera (jogador)
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

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{

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

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :
