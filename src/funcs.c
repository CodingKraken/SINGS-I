#include "main.h"

// Return the distance between two position vectors
double dist(Vector2 pos1, Vector2 pos2) {
    return sqrtl((pos1.x-pos2.x)*(pos1.x-pos2.x) + (pos1.y-pos2.y)*(pos1.y-pos2.y));
}

// Return the distance squared between two position vectors
double distsqr(Vector2 pos1, Vector2 pos2) {
    return (pos1.x-pos2.x)*(pos1.x-pos2.x) + (pos1.y-pos2.y)*(pos1.y-pos2.y);
}

// Return the max of two doubles
double maxl(double a, double b) {
    if(a > b) return a;
    return b;
}

// Return the min of two doubles
double minl(double a, double b) {
    if(a < b) return a;
    return b;
}

double absl(double a) {
    return sgn(a)*a;
}

int sgn(double a) {
    return ((a > 0) - (a < 0));
}

// Return a random double between an upper and lower bounds
double randrange(double min, double max) {
    return (double)rand()/RAND_MAX*(max-min) + min;
}

double randrangeBiased(double min, double max) {
    return (1 - cbrtl(1 - (double)rand()/RAND_MAX))*(1+max-min) + min;
}


Vector2 cartToScreen(Vector2 pos, Vector3 camdata, RenderTexture2D render_texture) {
    return (Vector2){pos.x*camdata.z + camdata.x + render_texture.texture.width/2, 
    (-pos.y*camdata.z + render_texture.texture.height/2 - camdata.y)};
}

Vector2 screenToCart(Vector2 pos, Vector3 camdata, RenderTexture2D render_texture) {
    return (Vector2){.x=(pos.x-camdata.x-render_texture.texture.width/2)/camdata.z, 
    .y=-(pos.y-camdata.y-render_texture.texture.height/2)/camdata.z};
}

// Create a unit vector that points from the first position vector to the second
Vector2 normalize(Vector2 vec1, Vector2 vec2) {
    return (Vector2){(vec2.x-vec1.x)/dist(vec1, vec2), (vec2.y-vec1.y)/dist(vec1,vec2)};
}

// Linear interpolate colours between purple to orange based on a particle's velocity.
Color velocityColourLerp(Particle ob) {
    Color slowcolour = (Color){59, 0, 107, 255};
    Color fastcolour = (Color){255, 147, 0, 255};
    double vmag = sqrtl(powl(ob.velocity.x,2)+powl(ob.velocity.y,2));
    double tval = minl(vmag/100, 1);

    Color newcolour = (Color){
        .r = slowcolour.r + (fastcolour.r-slowcolour.r)*tval,
        .g = slowcolour.g + (fastcolour.g-slowcolour.g)*tval,
        .b = slowcolour.b + (fastcolour.b-slowcolour.b)*tval,
        .a = slowcolour.a + (fastcolour.a-slowcolour.a)*tval
    };
    
    return newcolour;
}

Vector2 lerp2d(Vector2 p1, Vector2 p2, float t) {
    return (Vector2) {
        .x= p1.x + (p2.x-p1.x)*t,
        .y= p1.y + (p2.y-p1.x)*t
    };
}

int writedata(char *filename, ParticleData *particle_data, int particles, int frames) {
    int total = particles*frames;
    FILE *file;
    
    file = fopen(filename, "w+b");

    if(file == NULL) return 0;

    if(fwrite(&particles, sizeof(int), 1, file) != 1)
        return 0;
    if(fwrite(&frames, sizeof(int), 1, file) != 1)
        return 0;

    if(fwrite(particle_data, sizeof(ParticleData), total, file) != total)
        return 0;

    if(fclose(file) == EOF) return 0;

    return 1;
}

ParticleData* readdata(char *filename, int *particles, int *frames) {
    FILE *file;

    file = fopen(filename, "rb");

    if(file == NULL) return NULL;

    if(fread(particles, sizeof(int), 1, file) != 1)
        return NULL;
    if(fread(frames, sizeof(int), 1, file) != 1)
        return NULL;

    int total = *particles * *frames;
    ParticleData *particle_data = malloc(sizeof(ParticleData)*total);

    if(fread(particle_data, sizeof(ParticleData), total, file) != total) {
        free(particle_data);
        return NULL;
    }

    if(fclose(file) == EOF) {
        free(particle_data);
        return NULL;
    }

    return particle_data;
}