#ifndef MAIN_H
#define MAIN_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <raylib.h>

#define TIMESTEP 0.05f
#define G 1
#define E 3.f

#define BH_THRESHOLD 0.5

typedef struct Boundary {
    Vector2 position;
    double width, height;
} Boundary;

typedef struct Particle {
    double mass;

    Vector2 position;
    Vector2 velocity;
    Vector2 force;
} Particle;

typedef struct QuadTree {
    Boundary boundary;
    Particle particle;
    struct QuadTree* nw;
    struct QuadTree* ne;
    struct QuadTree* sw;
    struct QuadTree* se;
} QuadTree;

typedef struct ParticleData {
    Vector2 position;
    double size;
} ParticleData;

QuadTree* initialize(Boundary boundary);
void insert(QuadTree* qt, Particle particle);
void freetree(QuadTree* qt);
void drawtree(QuadTree* qt, RenderTexture2D* render_texture, Vector3 camdata);

int inBoundary(Boundary boundary, Vector2 position);
void drawBoundaries(QuadTree qt, float zoom);
void search(QuadTree* qt, Vector2 pos, Particle* particle);

void initParticles(Particle* particle_list, int num_particles);
void generateParticles(Particle* particle_list, int num_particles, Boundary boundary);
void generateGalaxy(Particle* particle_list, int num_particles, Vector2 pos,  Vector2 pec);
void generateCloud(Particle* particle_list, int num_particles, Vector2 pos, Vector2 max_mag);

void gravIntegrateNewton(Particle* particle_list, int num_particles);
void newtonGravCalc(Particle* particle_list, int num_particles, Particle* particle);
void gravIntegrateBH(QuadTree* qt, Particle* particle_list, int num_particles);
void bhGravCalc(QuadTree* qt, Particle* particle);
void bhTreeDraw(QuadTree* qt, Particle particle, RenderTexture2D* render_texture, Vector3 camdata);

void update(QuadTree* qt, Particle* particle_list, int num_Particles, int bh);

double dist(Vector2 pos1, Vector2 pos2);
double distsqr(Vector2 pos1, Vector2 pos2);
double minl(double a, double b);
double maxl(double a, double b);
double absl(double a);
int sgn(double a);
double randrange(double min, double max);
double randrangeBiased(double min, double max);
Vector2 cartToScreen(Vector2 pos, Vector3 camdata, RenderTexture2D render_texture);
Vector2 screenToCart(Vector2 pos, Vector3 camdata, RenderTexture2D render_texture);
Vector2 normalize(Vector2 vec1, Vector2 vec2);
Vector2 lerp2d(Vector2 p1, Vector2 p2, float t);

int writedata(char *filename, ParticleData *particle_data, int particles, int frames);
ParticleData* readdata(char *filename, int *particles, int *frames);

Color velocityColourLerp(Particle ob);

#endif