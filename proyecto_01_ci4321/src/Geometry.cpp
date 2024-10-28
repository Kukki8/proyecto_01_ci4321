#include "Geometry.h"
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

Sphere::Sphere(float radius, int sectorCount, int stackCount, bool full)
{
    this->radius = radius;
    this->sectorCount = sectorCount;
    this->stackCount = stackCount;

    position = glm::vec3(0.0, 0.0, 0.0);
    rotation = glm::vec3(0.0, 0.0, 0.0);

    // Inicializacion de variables a utilizar para calcular posicon, normales y coordenadas de la textura
    float x, y, z, xy;                    
    float nx, ny, nz, lengthInv = 1.0f / radius;
    float s, t;

    // Inicializacion de distancias entre longitudes y latitudes
    float sphereRange = 2 * PI;

    if (!full) {
        sphereRange = PI;
    }

    float sectorStep = sphereRange / sectorCount; // <- Poner sectorStep = PI/sectorCount para hacer media circunferencia (Top/Separado/despues)
    float stackStep = PI / stackCount;

    // Variables para los angulos a utilizar
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        // Se determina el angulo de apertura vetical de la esfera (desde pi/2 a -pi/2)
        stackAngle = PI / 2 - i * stackStep; 

        // Vector del plano XY -> r * cos(u)
        xy = radius * cosf(stackAngle);

        // Vector de la componente Z -> r * sin(u)
        z = radius * sinf(stackAngle);              

        // Ciclo que recorre el numero de sectores de la esfera. 
        // El primer y ultimo vertice tendran la misma posicion y las mismas normales, pero las coordenadas de sus texturas varia
        for (int j = 0; j <= sectorCount; ++j)
        {
            // Recorrido horizontal de la esfera (de 0 a 2pi)
            sectorAngle = j * sectorStep;           

            // Posicion del vertice (x,y,z)

            // Componente X -> r * cos(u) * cos(v)
            x = xy * cosf(sectorAngle);             

            // Componente Y -> r * cos(u) * sin(v)
            y = xy * sinf(sectorAngle);  

            // Agregamos los vertices a la lista
            attributes.push_back(x);
            attributes.push_back(y);
            attributes.push_back(z);

            // Agregando normales (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            attributes.push_back(nx);
            attributes.push_back(ny);
            attributes.push_back(nz);

            // Calculos de las coordenadas de los vertices en la textura, dentro del rango [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            attributes.push_back(s);
            attributes.push_back(t);
        }
    }

    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        // Inicializacion de la esfera por sectores. Se usara el termino "aro" como analogia, al referirse a cada nivel de la esfera
        // Comienzo desde el "aro" actual
        k1 = i * (sectorCount + 1);     

        // Comienzo desde el "aro" actual
        k2 = k1 + sectorCount + 1;      

        for (int j = 0; j < sectorCount; ++j)
        {
            // Hay 2 triangulos por sector, excluyendo el aro superior y el inferior
            // k1 => k2 => k1+1
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }

            ++k1;
            ++k2;
        }
    }
}

void Sphere::Draw(const Shader& shader)
{
    // Creacion de transformaciones
    glm::mat4 model = glm::mat4(1.0f);
    
    model = glm::translate(model, position);

    model = glm::rotate(model, rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0, 0.0, 1.0));
    
    // Recuperacion de las ubicaciones de los uniforms
    unsigned int modelLoc = glGetUniformLocation(shader.ID, "model");
   
    // Pase de ubicaciones a los shaders
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, (void*)0);

}

void Sphere::SetupGL()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //Datos de vertices
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);           
    glBufferData(GL_ARRAY_BUFFER, (unsigned int)attributes.size() * sizeof(float), attributes.data(), GL_STATIC_DRAW);
    
    // Datos de indices
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);   
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (unsigned int)indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // Setear los atributos en el orden indicado (posicion, normales, coord Text) 
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)(sizeof(float) * 3));
    glVertexAttribPointer(2, 2, GL_FLOAT, false, 8 * sizeof(float), (void*)(sizeof(float) * 6));

    // Unbind de los buffers para limpiar futuras figuras
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Sphere::CleanGL()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
}

void Sphere::moveForward() {
    glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.01f);
    position += translation;
}

void Sphere::moveBackwards() {
    glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.01f);
    position -= translation;
}


Cube::Cube(float width, float height, float depth)
{
    this->width = width;
    this->height = height;
    this->depth = depth;

    position = glm::vec3(0.0, 0.0, 0.0);
    rotation = glm::vec3(0.0, 0.0, 0.0);

    // Inicializacion del cambio de las caras con respecto al cubo unitario
    float w = width/2;
    float h = height/2;
    float d = depth/2;

    // Lista de atributos del cubo (Posicion / Tex Coords)
    attributes = {
        // Cara trasera
        // Triangulo inferior
        -w, -h, -d,  0.0f, 0.0f,
         w, -h, -d,  1.0f, 0.0f,
         w,  h, -d,  1.0f, 1.0f,
        //Triangulo superior
         w,  h, -d,  1.0f, 1.0f,
        -w,  h, -d,  0.0f, 1.0f,
        -w, -h, -d,  0.0f, 0.0f,

        // Cara frontal
        // Triangulo inferior
        -w, -h,  d,  0.0f, 0.0f,
         w, -h,  d,  1.0f, 0.0f,
         w,  h,  d,  1.0f, 1.0f,
        // Triangulo superior
         w,  h,  d,  1.0f, 1.0f,
        -w,  h,  d,  0.0f, 1.0f,
        -w, -h,  d,  0.0f, 0.0f,

        // Cara lateral izquierda
        // Triangulo superior
        -w,  h,  d,  1.0f, 0.0f,
        -w,  h, -d,  1.0f, 1.0f,
        -w, -h, -d,  0.0f, 1.0f,
        // Triangulo inferior
        -w, -h, -d,  0.0f, 1.0f,
        -w, -h,  d,  0.0f, 0.0f,
        -w,  h,  d,  1.0f, 0.0f,

        // Cara lateral derecha
        // Triangulo superior
         w,  h,  d,  1.0f, 0.0f,
         w,  h, -d,  1.0f, 1.0f,
         w, -h, -d,  0.0f, 1.0f,
        // Triangulo inferior
         w, -h, -d,  0.0f, 1.0f,
         w, -h,  d,  0.0f, 0.0f,
         w,  h,  d,  1.0f, 0.0f,

        // Cara inferior
        // Triangulo superior
        -w, -h, -d,  0.0f, 1.0f,
         w, -h, -d,  1.0f, 1.0f,
         w, -h,  d,  1.0f, 0.0f,
        // Triangulo inferior
         w, -h,  d,  1.0f, 0.0f,
        -w, -h,  d,  0.0f, 0.0f,
        -w, -h, -d,  0.0f, 1.0f,

        // Cara superior
        // Triangulo superior
        -w,  h, -d,  0.0f, 1.0f,
         w,  h, -d,  1.0f, 1.0f,
         w,  h,  d,  1.0f, 0.0f,
        // Triangulo inferior
         w,  h,  d,  1.0f, 0.0f,
        -w,  h,  d,  0.0f, 0.0f,
        -w,  h, -d,  0.0f, 1.0f
    };

};

void Cube::Draw(const Shader& shader)
{
    // Creacion de transformaciones
    glm::mat4 model = glm::mat4(1.0f); 
   
    model = glm::translate(model, position);

    model = glm::rotate(model, rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0, 0.0, 1.0));

    // Recuperacion de las ubicaciones de los uniforms
    unsigned int modelLoc = glGetUniformLocation(shader.ID, "model");

    // Pase de ubicaciones a los shaders
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);

    // Dibujamos el cubo
    glDrawArrays(GL_TRIANGLES, 0, 36);

}

void Cube::SetupGL()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Iniciamos el proceso de binding/vinculacion
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (unsigned int)attributes.size() * sizeof(float), attributes.data(), GL_STATIC_DRAW);

    // Atributos de posicion
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atributos de coordenadas de texturas
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind de los buffers para limpiar futuras figuras
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Cube::CleanGL()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Cube::moveForward() {
    glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.01f);
    position += translation;
}

void Cube::moveBackwards() {
    glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.01f);
    position -= translation;
}

void Cube::moveRight() {
    glm::vec3 translation = glm::vec3(0.01f, 0.0f, 0.0f);
    position += translation;
}

void Cube::moveLeft() {
    glm::vec3 translation = glm::vec3(0.01f, 0.0f, 0.0f);
    position -= translation;
}

Cylinder::Cylinder(float radius, float height, int sectorCount) {

    // Crea un circulo en el plano XY
    this->radius = radius;
    this->height = height;
    this->sectorCount = sectorCount;

    position = glm::vec3(0.0, 0.0, 0.0);
    rotation = glm::vec3(0.0, 0.0, 0.0);
        
    // Variables para generar las cosas
    float sectorStep = 2 * PI / sectorCount;
    float sectorAngle; // Angulo en Radianes

    for (int i = 0; i <= sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        unitCircleVertices.push_back(cos(sectorAngle)); // x
        unitCircleVertices.push_back(sin(sectorAngle)); // y 
        unitCircleVertices.push_back(0);                // z
    }
    
    std::vector<float> unitVertices = unitCircleVertices;

    attributes.clear();

    int verticesCount = 0;

    for (int i = 0; i < 2; ++i) {
        float h = -height / 2.0f + i * height;
        float t = 1.0f - i;
        for (int j = 0, k = 0; j <= sectorCount; ++j, k += 3) {

            float ux = unitVertices[k];
            float uy = unitVertices[k + 1];
            float uz = unitVertices[k + 2];

            // Vectores de posicion
            // se añaden a la lista de vertices
            attributes.push_back(ux * radius);
            attributes.push_back(uy * radius);
            attributes.push_back(h);

            verticesCount += 3;

            // Se añaden los vectores normales
            attributes.push_back(ux);
            attributes.push_back(uy);
            attributes.push_back(uz);
                
            // Coord de la textura;
            // Calculos de las coordenadas de los vertices en la textura, dentro del rango [0, 1]
            float s = (float)j / sectorCount;

            attributes.push_back(s);
            attributes.push_back(t);
        }
    }
    
    // Se ubica el punto central del cilindro
    int baseCenterIndex = verticesCount / 3;
    int topCenterIndex = baseCenterIndex + sectorCount + 1;

    for ( int i = 0; i < 2; ++i) {

        float h = -height / 2.0f + i * height;
        float nz = -1 + i * 2;
        
        attributes.push_back(0);
        attributes.push_back(0);
        attributes.push_back(h);
        attributes.push_back(0);
        attributes.push_back(0);
        attributes.push_back(nz);
        attributes.push_back(0.5f);
        attributes.push_back(0.5f);
        
        for ( int j = 0, k = 0; j < sectorCount; ++j, k += 3) {
            float ux = unitVertices[k];
            float uy = unitVertices[k + 1];

            attributes.push_back(ux * radius);
            attributes.push_back(uy * radius);
            attributes.push_back(h);

            attributes.push_back(0);
            attributes.push_back(0);
            attributes.push_back(nz);
            

            attributes.push_back(-ux * 0.5f + 0.5f);
            attributes.push_back(-uy * 0.5f + 0.5f);
        }
    }


    int k1 = 0;
    int k2 = sectorCount + 1;
    indices.clear();

    for (int i = 0; i < sectorCount; ++i, ++k1, ++k2) {
        // Se dibujan dos triangulos por sector
        indices.push_back(k1);
        indices.push_back(k1 + 1);
        indices.push_back(k2);

        indices.push_back(k2);
        indices.push_back(k1 + 1);
        indices.push_back(k2 + 1);
    }

    // Indices para la superficie de abajo

    int k = baseCenterIndex + 1;
    for ( int i = 0; i < sectorCount; ++i) {
        if (i < sectorCount - 1) {
            indices.push_back(baseCenterIndex);
            indices.push_back(k + 1);
            indices.push_back(k);
        }

        else {
            indices.push_back(baseCenterIndex);
            indices.push_back(baseCenterIndex + 1);
            indices.push_back(k);
        }
        ++k;
    }
    k = topCenterIndex + 1;
    // indices para la parte de arriba
    for (int i = 0; i < sectorCount; ++i)
    {
        if (i < sectorCount - 1)
        {
            indices.push_back(topCenterIndex);
            indices.push_back(k);
            indices.push_back(k + 1);
        }
        else // Ultimo triangulo
        {
            indices.push_back(topCenterIndex);
            indices.push_back(k);
            indices.push_back(topCenterIndex + 1);
        }
        ++k;
    }
}

void Cylinder::SetupGL() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);


    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        (unsigned int)attributes.size() * sizeof(float),
        attributes.data(),
        GL_STATIC_DRAW);

    // Datos de indices
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
        (unsigned int)indices.size() * sizeof(unsigned int), 
        indices.data(), 
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // Setear los atributos en el orden indicado (posicion, normales, coord Text) 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float) * 3));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float) * 6));

    // Unbind de los buffers para limpiar futuras figuras
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Cylinder::CleanGL()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
}

void Cylinder::Draw(const Shader& shader)
{
    // Creacion de transformaciones
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, position);

    model = glm::rotate(model, rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0, 0.0, 1.0));

    // Recuperacion de las ubicaciones de los uniforms
    unsigned int modelLoc = glGetUniformLocation(shader.ID, "model");

    // Pase de ubicaciones a los shaders
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, (void*)0);

}

void Cylinder::DrawCanon(const Shader& shader)
{
    // Creacion de transformaciones
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, position);
    glm::vec3 pivot = glm::vec3(0.0f, 0.0f, -1.0f);
    model = glm::translate(model, pivot);
    model = glm::rotate(model, rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0, 0.0, 1.0));
    
    model = glm::translate(model, -pivot);

    // Recuperacion de las ubicaciones de los uniforms
    unsigned int modelLoc = glGetUniformLocation(shader.ID, "model");

    // Pase de ubicaciones a los shaders
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, (void*)0);

}

void Cylinder::moveForward() {
    glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.01f);
    position += translation;
}

void Cylinder::moveBackwards() {
    glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.01f);
    position -= translation;
}