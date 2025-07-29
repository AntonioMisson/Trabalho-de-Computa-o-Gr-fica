#include <GL/glut.h>
#include <vector>
#include <ctime>
#include <memory>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

constexpr float VELOCIDADE_INICIAL = 0.04f;
constexpr float DIST_INICIAL = -30.0f;
constexpr float DIST_REPOS = 10.0f;
constexpr int   NUM_LADOS = 12;
constexpr float RAIO_TUNEL = 0.5f;

struct Vec3 { float x, y, z; Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {} };

struct Tiro {
    Vec3 pos;
    Tiro(float x, float y, float z) : pos(x, y, z) {}
};

void desenharTexto(float x, float y, const std::string& texto) {
    glRasterPos2f(x, y);
    for (char c : texto) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

class Nave {
private:
    Vec3 pos;
    Vec3 dim;
    std::vector<Tiro> tiros;
    float tempoDisparo;
    float recarga;

public:
    Nave(float x, float y, float z)
        : pos(x, y, z), dim(0.3f, 0.2f, 0.5f), tempoDisparo(0), recarga(0.7f) {
    }

    void setPos(float x, float y, float z) {
        pos.x = x; pos.y = y; pos.z = z;
    }

    void atualizar(float dt) {
        tempoDisparo += dt;
        for (size_t i = 0; i < tiros.size();) {
            tiros[i].pos.z -= 0.15f;
            if (tiros[i].pos.z < DIST_INICIAL) tiros.erase(tiros.begin() + i);
            else ++i;
        }
    }
    void disparar(int currentFace) {
        if (tempoDisparo < recarga) return; 

        float angle = currentFace * (2 * M_PI / NUM_LADOS);
        float x = RAIO_TUNEL * cos(angle);
        float y = RAIO_TUNEL * sin(angle);
        float z = pos.z;

        tiros.emplace_back(x, y, z); 
        tempoDisparo = 0; 
    }


    void renderizar() const {
        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        glColor3f(0.8f, 0.1f, 0.1f); // Cor da nave (vermelha)

        // Pirâmide voltada para -Z
        glBegin(GL_TRIANGLES);
        // Frente
        glVertex3f(0.0f, 0.0f, -0.3f);
        glVertex3f(-0.1f, -0.1f, 0.0f);
        glVertex3f(0.1f, -0.1f, 0.0f);

        // Lado direito
        glVertex3f(0.0f, 0.0f, -0.3f);
        glVertex3f(0.1f, -0.1f, 0.0f);
        glVertex3f(0.0f, 0.1f, 0.0f);

        // Lado esquerdo
        glVertex3f(0.0f, 0.0f, -0.3f);
        glVertex3f(0.0f, 0.1f, 0.0f);
        glVertex3f(-0.1f, -0.1f, 0.0f);
        glEnd();

        // Tiros amarelos
        for (const auto& t : tiros) {
            glPushMatrix();
            glTranslatef(t.pos.x, t.pos.y, t.pos.z);
            glColor3f(1.0f, 1.0f, 0.0f); // Cor amarela
            glutSolidCube(0.05f);
            glPopMatrix();
        }

        glPopMatrix();
    }


    Vec3 getPos() const { return pos; }
    Vec3 getDim() const { return dim; }
    std::vector<Tiro>& getTiros() { return tiros; }
    void removerTiro(size_t i) { if (i < tiros.size()) tiros.erase(tiros.begin() + i); }
};

class Inimigo {
private:
    Vec3 pos;
    Vec3 vel;
    float tam;
    bool ativo;

public:
    Inimigo(float x, float y, float z)
        : pos(x, y, z), vel(0, 0, 0.1f), tam(0.3f), ativo(true) {
    }

    void atualizar(const Vec3& alvo) {
        Vec3 dir = { alvo.x - pos.x, alvo.y - pos.y, alvo.z - pos.z };
        float len = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        if (len > 0) {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }
        float velocidade = 0.1f;
        pos.x += dir.x * velocidade;
        pos.y += dir.y * velocidade;
        pos.z += dir.z * velocidade;

        if (pos.z > 0) ativo = false;
       
    }

    void renderizar() const {
        if (!ativo) return;
        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        glColor3f(1.0f, 0.0f, 0.0f);
        glutSolidSphere(tam, 16, 16);
        glColor3f(1.0f, 1.0f, 0.0f);
        glPushMatrix(); glTranslatef(tam, 0, 0); glutSolidSphere(tam / 3, 8, 8); glPopMatrix();
        glPushMatrix(); glTranslatef(-tam, 0, 0); glutSolidSphere(tam / 3, 8, 8); glPopMatrix();
        glPopMatrix();
    }

    Vec3 getPos() const { return pos; }
    float getTam() const { return tam; }
    bool isAtivo() const { return ativo; }

    bool atingido(const Tiro& tiro) {
        float dist = sqrt(pow(tiro.pos.x - pos.x, 2) +
            pow(tiro.pos.y - pos.y, 2) +
            pow(tiro.pos.z - pos.z, 2));
        return dist < (tam + 0.05f);
    }

    
    Vec3 getMin() const { return { pos.x - tam / 2, pos.y - tam / 2, pos.z - tam / 2 }; }
    Vec3 getMax() const { return { pos.x + tam / 2, pos.y + tam / 2, pos.z + tam / 2 }; }
};


class Obstaculo {
private:
    Vec3 pos;
    float tam;
    float raio;
    bool destrutivel;

public:
    
    Obstaculo(float x, float y, float z, float t, bool destrutivel)
        : pos(x, y, z), tam(t), destrutivel(destrutivel) {
    }

    float getTam() const { return tam; }
    
    bool colideCom(const Vec3& outroPos, float outroRaio) const {
        float dx = pos.x - outroPos.x;
        float dy = pos.y - outroPos.y;
        float dz = pos.z - outroPos.z;
        float distQuadrada = dx * dx + dy * dy + dz * dz;
        float somaRaios = raio + outroRaio;
        return distQuadrada <= (somaRaios * somaRaios);
    }
 

    void atualizar(float v) { pos.z += v; }

    void renderizar() const {
        glPushMatrix();

        glTranslatef(pos.x, pos.y, pos.z);
        if (destrutivel) {
            glColor3f(0.2f, 0.8f, 0.2f);
            glutSolidCube(tam);
        }
        else { 
            glColor3f(1.0f, 0.5f, 0.0f);
            glutWireCube(tam);
        }
        glPopMatrix();
    }

    bool isDestrutivel() const { return destrutivel; }
    Vec3 getMin() const { return { pos.x - tam, pos.y - tam, pos.z - tam }; }
    Vec3 getMax() const { return { pos.x + tam, pos.y + tam, pos.z + tam }; }
    Vec3 getPos() const { return pos; }
};

class Jogo {
private:
    std::unique_ptr<Nave> nave;
    std::vector<Obstaculo> obst;  
    std::vector<float> tunelZ;
    std::vector<Inimigo> inimigos;
    float tempoSpawnInimigo;
    float vel;
    bool gameOver;
    int largura, altura;
    int currentFace;
    float navZ;
    int pontos = 0;

    void initTunel() {
        for (int i = 0; i < 50; ++i)
            tunelZ.push_back(DIST_INICIAL + i * 0.5f);
    }

    bool checkCollisionEsferica(const Vec3& a, float raioA, const Vec3& b, float raioB) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        float dz = a.z - b.z;
        float dist2 = dx * dx + dy * dy + dz * dz;
        float somaRaios = raioA + raioB;
        return dist2 < (somaRaios * somaRaios);
    }

public:
    Jogo() : vel(VELOCIDADE_INICIAL), gameOver(false), largura(800), altura(600),
        currentFace(0), navZ(-1.5f), tempoSpawnInimigo(0) {
        srand((unsigned)time(nullptr));
        float angle = 0;
        float x = RAIO_TUNEL * cos(angle);
        float y = RAIO_TUNEL * sin(angle);
        nave = std::make_unique<Nave>(x, y, navZ);
        initTunel();
    }

    void atualizar() {
        if (gameOver) return;

        // Atualizar túnel
        for (auto& z : tunelZ) {
            z += vel;
            if (z > DIST_REPOS) z = DIST_INICIAL;
        }

        // Atualizar nave
        nave->atualizar(0.016f);

        // Spawn de obstáculos 
        if (rand() % 100 < 2) {
            int p = rand() % NUM_LADOS;
            float angle = p * (2 * M_PI / NUM_LADOS);
            float ox = RAIO_TUNEL * cos(angle);
            float oy = RAIO_TUNEL * sin(angle);
            obst.emplace_back(ox, oy, DIST_INICIAL, 0.3f, false); 
        }

        // Spawn de inimigos
        tempoSpawnInimigo += 0.016f;
        if (tempoSpawnInimigo > 2.0f) {
            spawnInimigo();
            tempoSpawnInimigo = 0;
        }

        // Atualizar obstáculos 
        Vec3 navPos = nave->getPos();
        Vec3 navDim = nave->getDim();
        for (size_t i = 0; i < obst.size(); ) {
            obst[i].atualizar(vel);

            if (checkCollisionEsferica(navPos, 0.25f, obst[i].getPos(), obst[i].getTam() / 2.0f)) {
                gameOver = true;
            }

            if (obst[i].getPos().z > DIST_REPOS) {
                obst.erase(obst.begin() + i);
            }
            else {
                i++;
            }
        }

        // Atualizar inimigos 
        for (auto it = inimigos.begin(); it != inimigos.end(); ) {
            it->atualizar(nave->getPos());

            // Remove inativos
            if (!it->isAtivo()) {
                it = inimigos.erase(it);
            }
            else {
                // Verifica colisão com tiros
                bool removido = false;
                auto& tiros = nave->getTiros();

                for (auto t = tiros.begin(); t != tiros.end(); ) {
                    if (it->atingido(*t)) {
                        t = tiros.erase(t);
                        it = inimigos.erase(it);
                        pontos++;
                        removido = true;

                        break;
                    }
                    else {
                        ++t;
                    }
                }

                if (!removido) {
                    // Verifica colisão com nave
                    if (checkCollisionEsferica(navPos, 0.25f, it->getPos(), it->getTam())) {
                        gameOver = true;
                    }
                    ++it;
                }
            }
        }
        if (gameOver) return;
    }

    void spawnInimigo() {
        // Gera posição em um círculo à frente da nave
        float angle = (rand() % 360) * M_PI / 180.0f;
        float radius = RAIO_TUNEL * 0.7f;

        float x = cos(angle) * radius;
        float y = sin(angle) * radius;
        float z = nave->getPos().z - 20.0;  

        inimigos.emplace_back(x, y, z);
    }

    void renderizar() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        gluLookAt(0, 0, 5,
            0, 0, nave->getPos().z - 5.0f,
            0, 1, 0);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, largura, 0, altura);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor3f(1, 1, 1);
        std::string scoreText = "Pontos: " + std::to_string(pontos);
        desenharTexto(10, altura - 20, scoreText);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        // Renderizar túnel
        glColor3f(0.1f, 0.1f, 0.8f);
        for (auto z : tunelZ) {
            glPushMatrix();
            glTranslatef(0, 0, z);
            float esc = 1.0f / (1.0f + z * 0.1f);
            glScalef(esc, esc, 1.0f);
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < NUM_LADOS; ++i) {
                float a = i * (2 * M_PI / NUM_LADOS);
                glVertex3f(RAIO_TUNEL * cos(a), RAIO_TUNEL * sin(a), 0);
            }
            glEnd();
            glPopMatrix();
        }

        // Renderizar inimigos
        for (auto& inimigo : inimigos) {
            inimigo.renderizar();
        }

        // Renderizar nave
        nave->renderizar();

        // Renderizar apenas obstáculos wireframe (laranja)
        for (auto& o : obst) {
            o.renderizar(); // Já são wireframe por serem todos indestrutíveis
        }

        if (gameOver) {
            glMatrixMode(GL_PROJECTION);
            glPushMatrix(); glLoadIdentity();
            gluOrtho2D(0, largura, 0, altura);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix(); glLoadIdentity();
            std::string txt = "GAME OVER - PRESSIONE R PARA REINICIAR";
            glColor3f(1, 0, 0);
            glRasterPos2i(largura / 2 - 180, altura / 2 - 10);
            for (char c : txt) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
            glPopMatrix();
            glMatrixMode(GL_PROJECTION); glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
        }

        glutSwapBuffers();
    }

    void Teclado(unsigned char k, int, int) {
        if (!gameOver && (k == 'a' || k == 'd')) {
            if (k == 'a') currentFace = (currentFace - 1 + NUM_LADOS) % NUM_LADOS;
            else currentFace = (currentFace + 1) % NUM_LADOS;
            float angle = currentFace * (2 * M_PI / NUM_LADOS);
            float x = RAIO_TUNEL * cos(angle);
            float y = RAIO_TUNEL * sin(angle);
            nave->setPos(x, y, navZ);
        }
        if (k == 'r' && gameOver) {
            obst.clear();
            inimigos.clear();
            gameOver = false;
            currentFace = 0;
            tempoSpawnInimigo = 0;
            pontos = 0;
            float angle = currentFace * (2 * M_PI / NUM_LADOS);
            nave->setPos(RAIO_TUNEL * cos(angle), RAIO_TUNEL * sin(angle), navZ);
        }
        if (k == 32) {
            nave->disparar(currentFace);
        }
    }

    void redimensionar(int w, int h) {
        largura = w; altura = h;
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION); glLoadIdentity();
        gluPerspective(45.0, (float)w / h, 0.1, 100.0);
        glMatrixMode(GL_MODELVIEW);
    }
};

std::unique_ptr<Jogo> jogo;
void atualizar(int) { jogo->atualizar(); glutPostRedisplay(); glutTimerFunc(16, atualizar, 0); }
void renderizar() { jogo->renderizar(); }
void tecladoCB(unsigned char k, int x, int y) { jogo->Teclado(k, x, y); }
void redimCB(int w, int h) { jogo->redimensionar(w, h); }

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Tunelius");
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);

    jogo = std::make_unique<Jogo>();
    glutDisplayFunc(renderizar);
    glutReshapeFunc(redimCB);
    glutKeyboardFunc(tecladoCB);
    glutTimerFunc(0, atualizar, 0);
    glutMainLoop();
    return 0;
}