/*
No tengo ni idea de como compilar esto correctamente de momento.
Lo que hago es:

g++ -I include -c main.cpp
g++ main.o -o main -L lib -lsfml-graphics -lsfml-window -lsfml-system

Objetivo principal:
    -Generar nivel a partir de .txt[LISTO]
    -Agregar texturas a las paredes[LISTO]
    -Agregar texturas al techo/suelo[Meh, me vale de momento]
    -Mover camara[LISTO]
    -Mover camara con mouse[LISTO]
    -Renderizar objetos a partir de clase
    -Implementar pathfinding Dijkstra o A* para dicho objeto
    -Cambiar de nivel

*/

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <SFML/Window/Mouse.hpp>

// tamaño ventana
const int sWidth = 1280;
const float sWHalf = 640.0f; // mitad de pantalla
const int screenHeight = 720;

const float cameraHeight = 0.45f; //0 --> floor 1 --> ceiling

const int texture_size = 512; 
const int texture_wall_size = 128; 

const float fps_refresh_time = 0.1f; 






///////////////////////////////////////////////////////////////////////////
// La unica clase (de momento) [EDIT: y la de Objeto2d], luego quitare del main y hare Player, Render
// Devuelta, no se como compilar varios modulos con SFML aun, cuando sepa
// le doy su modulo
///////////////////////////////////////////////////////////////////////////
class Map{
    public:

    //Guarda mapa .txt en un array
        void loadMap(const std::string &nombre){
            std::ifstream f1;
            std::string cad;
            f1.open(nombre);
            int temp = 0;
            if(!f1.good()){
                std::cout << "ERROR: Invalid map file" << std::endl;
                exit(1);
            }
            while(!f1.eof()){
                getline(f1, cad);
                for(int i = 0; i < cad.length(); i++){
                    worldMap[i+temp] = cad[i];
                }
                temp += cad.length();
            }

            f1.close();
        }
        
        // Todas las walls (paredes en ingles, me parece mas conveniente wall xd) posibles
        enum class WallTexture {
            Banner,
            Red,
            Mutant,
            Gray,
            Blue,
            Moss,
            Wood,
            Stone,
        };

        // devuelve caracter de la tile del mapa 
        char getTile(int x, int y) const{
            return worldMap[y*mapWidth+x];
        }

        //Comprueba que funcione y se haya cargado bien el mapa
        bool mapCheck(){
            int mSize = 24*24 - 1; // -1 idkw

            for (int i = 0; i < mapHeight; i++) {
                for (int j = 0; j < mapWidth; j++) {
                    char tile = getTile(j, i);

                    if (tile != '.' && wallTypes.find(tile) == wallTypes.end()) {
                        std::cout << "ERROR: Unknown tile" << std::endl;
                        return false;
                    }
                    
                    if ((i == 0 || j == 0 || i == mapHeight - 1 || j == mapWidth - 1) && tile == '.') {
                        std::cout << "WARNING: Player can access limbo" << std::endl;
                        return false;
                    }
                }
            }
            return true;
        }

        //Codigos de las walls
        const std::unordered_map<char, WallTexture> wallTypes {
            {'#', WallTexture::Blue},
            {'=', WallTexture::Wood},
            {'M', WallTexture::Moss},
            {'N', WallTexture::Mutant},
            {'A', WallTexture::Gray},
            {'!', WallTexture::Red},
            {'@', WallTexture::Banner},
            {'^', WallTexture::Stone},
        };
    
    private:
        //dimensiones del mapa
        static const int mapWidth = 24;
        static const int mapHeight = 24;
        //Array del mapa: 1 dimension resulta mas simple(de momento)
        typedef char MArray[mapWidth*mapHeight];
        MArray worldMap;
};
////////////////////////////////////////////////////////////////////////



//Objeto 2d: tiene coordenadas y grafico asignado
class Object{
    public:
        Object(float xPos, float yPos, const std::string &texturePath){
            texture.loadFromFile(texturePath);
            position = sf::Vector2f(xPos, yPos);
            textureWidth = texture.getSize().x;
        }

    private:
        sf::Vector2f position;
        sf::Texture texture;
        float textureWidth;
    
};


// esta funcion pasara a ser de la clase Player, comprueba si se puede mover o si hay una wall bloqueando
bool canMove(sf::Vector2f position, sf::Vector2f size, const Map &map) {
    // esquinas del rectangulo de colision del jugador
    sf::Vector2i esqA(position - size / 2.0f);
    sf::Vector2i esqB(position + size / 2.0f);
    
    // Comprueba en todos los tiles que ocupa el jugador
    for (int y = esqA.y; y <= esqB.y; ++y) {
        for (int x = esqA.x; x <= esqB.x; ++x) {
            if (map.getTile(x, y) != '.') {
                return false;
            }
        }
    }
    return true;
}


//Esto tambien estaria en class Player, y se usa 2 veces para el vector plane y rotation cuando gira el jugador
sf::Vector2f rotateVec(sf::Vector2f vec, float valor) {
    return sf::Vector2f(
            vec.x * std::cos(valor) - vec.y * std::sin(valor),
            vec.x * std::sin(valor) + vec.y * std::cos(valor)
    );
}



//El main tocho, luego lo aliviamos con modulos
int main() {

    //cargamos el nivel
    Map map;
    map.loadMap("level1.txt");

    //comprobamos que todo funcione
    if (!map.mapCheck()) {
        std::cout << "ERROR: MAP NOT VALID" << std::endl;
        char x;
       std::cin >> x;
       exit(0);
    }
    sf::Font font;
    if (!font.loadFromFile("data/font/opensans.ttf")) {
        std::cout << "ERROR: FONT NOT VALID" << std::endl;
        char x;
       std::cin >> x;
       exit(0);
    }
    sf::Texture texture; // textura de walls
    if (!texture.loadFromFile("data/texture/walls.png")) {
        std::cout << "ERROR: TEXTURE NOT VALID" << std::endl;
        char x;
       std::cin >> x;
       exit(0);
    }


    // render state de la textura 
    sf::RenderStates state(&texture);

    //Todo este cacho iria dentro de Player
    sf::Vector2f position(2.5f, 2.0f); 
    sf::Vector2f direction(0.0f, 1.0f);
    sf::Vector2f plane(-0.66f, 0.0f);
    float fSize = 0.375f; 
    float moveSpeed = 5.0f; 
    float rotateSpeed = 2.0f;
    sf::Vector2f size(fSize, fSize);
    sf::Vector2i mPos(0.0f, 0.0f);
    sf::Vector2i mPos2(sWHalf, 0.0f);
    

    // Creamos la ventana
    sf::RenderWindow window(sf::VideoMode(sWidth + 1, screenHeight), "Copia china del Wolfenstein 3D");
    window.setSize(sf::Vector2u(sWidth, screenHeight)); 

    //Esto esconde el mouse
    window.setMouseCursorVisible(false);

    //Limite de FPS --> 60
    window.setFramerateLimit(60);
    //Focus es cuando esta activa la ventana o de fondo
    bool hasFocus = true;

    /*Ok esto es lo jodido: el rendering
    por ahi hay un enlace al metodo que use(esta en SDL y no SFML)
    Esto vamos haciendolo con lineas verticales, mirar el enlace*/
    sf::VertexArray lineas(sf::Lines, 18 * sWidth);

    sf::Text contFPS("", font, 50); // Un contador de FPS para ver si la cosa va bien
    sf::Clock clock; // timer
    char frameInfoString[sizeof("FPS: *****.*, Frame time: ******")];

    float dt_counter = 0.0f; // delta time
    int frame_counter = 0; 
    int64_t frame_time_micro = 0; // esto es el tiempo que emplea para dibujar cada frame

    ///Esto inicializa un objeto, pero no hace nada de momento
    sf::Texture objTexture;
    objTexture.loadFromFile("data/texture/barrel.png");
    sf::Sprite sprite;
    sprite.setTexture(objTexture, true);
    sprite.setPosition(640.0f, 700.0f);
    Object npc(2.0f, 2.5f, "data/texture/barrel.png");

    


    //Este es el bucle principal
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // Actualiza el FPS
        if (dt_counter >= fps_refresh_time) {
            float fps = (float)frame_counter / dt_counter;
            frame_time_micro /= frame_counter;
            snprintf(frameInfoString, sizeof(frameInfoString), "FPS: %3.1f, Frame time: %6ld", fps, frame_time_micro);
            contFPS.setString(frameInfoString);
            dt_counter = 0.0f;
            frame_counter = 0;
            frame_time_micro = 0;
        }
        dt_counter += dt;
        ++frame_counter;

        //SFML events(de ventana)
        sf::Event event;
        while (window.pollEvent(event)) {
            switch(event.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::LostFocus:
                hasFocus = false;
                break;
            case sf::Event::GainedFocus:
                hasFocus = true;
                break;
            default:
                break;
            }
        }
        
        //Input, esto ira en class Player
        if (hasFocus) {
            using kb = sf::Keyboard;

            float moveForward = 0.0f;
            if (kb::isKeyPressed(kb::W)) {
                moveForward = 1.0f;
            } else if (kb::isKeyPressed(kb::S)) {
                moveForward = -1.0f;
            }
            if (moveForward != 0.0f) {
                sf::Vector2f moveVec = direction * moveSpeed * moveForward * dt;

                if (canMove(sf::Vector2f(position.x + moveVec.x, position.y), size, map)) {
                    position.x += moveVec.x;
                }
                if (canMove(sf::Vector2f(position.x, position.y + moveVec.y), size, map)) {
                    position.y += moveVec.y;
                }
            }
            float strafe = 0.0f;

            if (kb::isKeyPressed(kb::D)) {
                strafe = 1.0f;
            } else if (kb::isKeyPressed(kb::A)) {
                strafe = -1.0f;
            }
            if (strafe != 0.0f) {
                sf::Vector2f moveVec = sf::Vector2f(direction.x * std::cos(90*strafe) - direction.y * std::sin(90*strafe), direction.x * std::sin(90*strafe) + direction.y * std::cos(90*strafe)) * moveSpeed * dt;
                if (canMove(sf::Vector2f(position.x + moveVec.x, position.y), size, map)) {
                    position.x += moveVec.x;
                }
                if (canMove(sf::Vector2f(position.x, position.y + moveVec.y), size, map)) {
                    position.y += moveVec.y;
                }
            }
            float rotateDirection = 0.0f;

            if(sf::Mouse::getPosition(window).x > mPos.x){
                rotateDirection = 1.0f;
            } else if(sf::Mouse::getPosition(window).x < mPos.x){
                rotateDirection = -1.0f;
            }
            mPos = sf::Mouse::getPosition(window);

            if(mPos.x >= (float)sWidth || mPos.x <= (float)0 || mPos.y >= (float)screenHeight || mPos.y <= (float)0){
                sf::Mouse::setPosition(mPos2, window);
                mPos = sf::Mouse::getPosition(window);
            }

            // rotacion, esto con la clase player es una linea
            if (rotateDirection != 0.0f) {
                float rotation = rotateSpeed * rotateDirection * dt;
                direction = rotateVec(direction, rotation);
                plane = rotateVec(plane, rotation);
            }

            //Cierra el programa con ESC
            if (kb::isKeyPressed(kb::Escape)) {
                exit(0);
            }
        }









        //La renderizacion: Lo mas jodido xd 

        lineas.resize(0);

        // Esto pasa por cada linea vertical de la ventana
        for (int x = 0; x < sWidth; ++x) {

            // emite un ray
            float cameraX = 2 * x / (float)sWidth - 1.0f; // posicion x relativo a camera
            sf::Vector2f rayPos = position;
            sf::Vector2f rayDir = direction + plane * cameraX; //direccion del ray con angulo

            //https://lodev.org/cgtutor/raycasting.html#Introduction

            //Concepto que ni idea xd, pero lo copie y funciona: 
            // NOTE: with floats, division by zero gives you the "infinity" value. This code depends on this.

            // distancia entre cada tile
            sf::Vector2f deltaDist(
                    sqrt(1.0f + (rayDir.y * rayDir.y) / (rayDir.x * rayDir.x)),
                    sqrt(1.0f + (rayDir.x * rayDir.x) / (rayDir.y * rayDir.y))
            );

            sf::Vector2i mapPos(rayPos); //posicion en mapa

            sf::Vector2i step; // direccion del "step"
            sf::Vector2f sideDist; // distancia hasta el proximo tile

            // calcula step y sideDist
            if (rayDir.x < 0.0f) {
                step.x = -1;
                sideDist.x = (rayPos.x - mapPos.x) * deltaDist.x;
            } else {
                step.x = 1;
                sideDist.x = (mapPos.x + 1.0f - rayPos.x) * deltaDist.x;
            }
            if (rayDir.y < 0.0f) {
                step.y = -1;
                sideDist.y = (rayPos.y - mapPos.y) * deltaDist.y;
            } else {
                step.y = 1;
                sideDist.y = (mapPos.y + 1.0f - rayPos.y) * deltaDist.y;
            }

            char tile = '.';
            bool horizontal;

            float perpWallDist = 0.0f; // distancia de la wall
            int wallHeight; 
            int ceilingPixel = 0; // posicion del ceiling(techo) en la pantalla
            int groundPixel = screenHeight; // posicion del suelo en la pantalla

            // inicializamos colores para techo y suelo
            sf::Color color1 = sf::Color::Blue;
            sf::Color color2 = sf::Color::White;
            sf::Color color3 = sf::Color::White;

            // esto hace efecto ajedrez, va cambiando de color
            sf::Color color = ((mapPos.x % 2 == 0 && mapPos.y % 2 == 0) ||
                               (mapPos.x % 2 == 1 && mapPos.y % 2 == 1)) ? color1 : color2;

            // hasta que toque wall, dibujamos suelo y techo
            while (tile == '.') {
                if (sideDist.x < sideDist.y) {
                    sideDist.x += deltaDist.x;
                    mapPos.x += step.x;
                    horizontal = true;
                    perpWallDist = (mapPos.x - rayPos.x + (1 - step.x) / 2) / rayDir.x;
                } else {
                    sideDist.y += deltaDist.y;
                    mapPos.y += step.y;
                    horizontal = false;
                    perpWallDist = (mapPos.y - rayPos.y + (1 - step.y) / 2) / rayDir.y;
                }

                wallHeight = screenHeight / perpWallDist;

                // suelo

                lineas.append(sf::Vertex(sf::Vector2f((float)x, (float)groundPixel), color, sf::Vector2f(385.0f, 129.0f)));
                groundPixel = int(wallHeight * cameraHeight + screenHeight * 0.5f);
                lineas.append(sf::Vertex(sf::Vector2f((float)x, (float)groundPixel), color, sf::Vector2f(385.0f, 129.0f)));

                // techo

                sf::Color color_c = color;
                color_c.r /= 2;
                color_c.g /= 2;
                color_c.b /= 2;

                color3.r = 173;
                color3.g = 129;
                color3.b = 26;

                lineas.append(sf::Vertex(sf::Vector2f((float)x, (float)ceilingPixel), color3, sf::Vector2f(385.0f, 129.0f)));
                ceilingPixel = int(-wallHeight * (1.0f - cameraHeight) + screenHeight * 0.5f);
                lineas.append(sf::Vertex(sf::Vector2f((float)x, (float)ceilingPixel), sf::Color::Black, sf::Vector2f(385.0f, 129.0f)));

                // cambiamos de color

                color = (color == color1) ? color2 : color1;

                tile = map.getTile(mapPos.x, mapPos.y);
            }

            // posicion mas alta y baja de linea
            int drawStart = ceilingPixel;
            int drawEnd = groundPixel;

            //posicion de textura en el tileset
            int wallTextureNum = (int)map.wallTypes.find(tile)->second;
            sf::Vector2i texture_coords(
                    wallTextureNum * texture_wall_size % texture_size,
                    wallTextureNum * texture_wall_size / texture_size * texture_wall_size
            );

            // calcula parte de wall
            float wall_x;
            if (horizontal) {
                wall_x = rayPos.y + perpWallDist * rayDir.y;
            } else {
                wall_x = rayPos.x + perpWallDist * rayDir.x;
            }
            wall_x -= floor(wall_x);

            // coordenada x de la textura
            int tex_x = int(wall_x * float(texture_wall_size));

            // si esta del otro lado, le damos la vuelta para que no quede raro
            if ((horizontal && rayDir.x <= 0) || (!horizontal && rayDir.y >= 0)) {
                tex_x = texture_wall_size - tex_x - 1;
            }

            texture_coords.x += tex_x;

            // Truco piola: oscurecemos 2 lados de cada wall (horizontal)
            color = sf::Color::White;
            if (horizontal) {
                color.r /= 2;
                color.g /= 2;
                color.b /= 2;
            }

            // añadimos las lineas al buffer
            lineas.append(sf::Vertex(
                        sf::Vector2f((float)x, (float)drawStart),
                        color,
                        sf::Vector2f((float)texture_coords.x, (float)texture_coords.y + 1)
            ));
            lineas.append(sf::Vertex(
                        sf::Vector2f((float)x, (float)drawEnd),
                        color,
                        sf::Vector2f((float)texture_coords.x, (float)(texture_coords.y + texture_wall_size - 1))
            ));
        }

        //dibujamos las lineas y todo lo demas
        window.clear();
        window.draw(lineas, state);
        window.draw(contFPS);
        window.draw(sprite);
        frame_time_micro += clock.getElapsedTime().asMicroseconds();
        window.display();
    }

    return 0;
}
