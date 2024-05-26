//
//  mesh2.h
//  40_AnimLoading
//
//  Created by iklee on 5/26/24.
//

#ifndef mesh2_h
#define mesh2_h

#include <vector>
#include <string>

// 메쉬 구조체 정의
struct Vertex {
    glm::vec3 Position;
    glm::vec2 TexCoords;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;
    unsigned int textureID;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, unsigned int textureID) {
        this->vertices = vertices;
        this->indices = indices;
        this->textureID = textureID;
        setupMesh();
    }

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // vertex positions
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        // vertex texture coords
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void Draw(unsigned int shaderProgram) {
        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};



#endif /* mesh2_h */
